/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  You should have received a copy of the GNU General
 * Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        provider-main.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        AD LDAP User Marshalling
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 *          Wei Fu (wfu@likewisesoftware.com)
 *          Brian Dunstan (bdunstan@likewisesoftware.com)
 *          Kyle Stemen (kstemen@likewisesoftware.com)
 */

#include "adprovider.h"

DWORD
ADMarshalFromUserCache(
    PAD_SECURITY_OBJECT pUser,
    DWORD       dwUserInfoLevel,
    PVOID*      ppUserInfo
    )
{
    DWORD dwError = 0;
    PVOID pUserInfo = NULL;
    /* These variables represent pUserInfo casted to different types. Do not
     * free these values directly, free pUserInfo instead.
     */
    PLSA_USER_INFO_0 pUserInfo0 = NULL;
    PLSA_USER_INFO_1 pUserInfo1 = NULL;
    PLSA_USER_INFO_2 pUserInfo2 = NULL;

    *ppUserInfo = NULL;

    BAIL_ON_INVALID_POINTER(pUser);

    if (pUser->type != AccountType_User)
    {
        dwError = LSA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LSA_ERROR(dwError);
    }

    switch(dwUserInfoLevel)
    {
        case 0:
            dwError = LsaAllocateMemory(
                            sizeof(LSA_USER_INFO_0),
                            (PVOID*)&pUserInfo);
            BAIL_ON_LSA_ERROR(dwError);
            pUserInfo0 = (PLSA_USER_INFO_0) pUserInfo;
            break;
        case 1:
            dwError = LsaAllocateMemory(
                            sizeof(LSA_USER_INFO_1),
                            (PVOID*)&pUserInfo);
            BAIL_ON_LSA_ERROR(dwError);
            pUserInfo0 = (PLSA_USER_INFO_0) pUserInfo;
            pUserInfo1 = (PLSA_USER_INFO_1) pUserInfo;
            break;
        case 2:
            dwError = LsaAllocateMemory(
                            sizeof(LSA_USER_INFO_2),
                            (PVOID*)&pUserInfo);
            BAIL_ON_LSA_ERROR(dwError);
            pUserInfo0 = (PLSA_USER_INFO_0) pUserInfo;
            pUserInfo1 = (PLSA_USER_INFO_1) pUserInfo;
            pUserInfo2 = (PLSA_USER_INFO_2) pUserInfo;
            break;
        default:
            dwError = LSA_ERROR_INVALID_PARAMETER;
            BAIL_ON_LSA_ERROR(dwError);
            break;
    }

    if (pUserInfo0 != NULL)
    {
        pUserInfo0->uid = pUser->userInfo.uid;
        pUserInfo0->gid = pUser->userInfo.gid;
        dwError = ADMarshalGetCanonicalName(
                pUser,
                &pUserInfo0->pszName);
        BAIL_ON_LSA_ERROR(dwError);

        // Optional values use LsaStrDupOrNull. Required values use
        // LsaAllocateString.
        dwError = LsaStrDupOrNull(
                    pUser->userInfo.pszPasswd,
                    &pUserInfo0->pszPasswd);
        BAIL_ON_LSA_ERROR(dwError);

        dwError = LsaStrDupOrNull(
                    pUser->userInfo.pszGecos,
                    &pUserInfo0->pszGecos);
        BAIL_ON_LSA_ERROR(dwError);

        dwError = LsaAllocateString(
                    pUser->userInfo.pszShell,
                    &pUserInfo0->pszShell);
        BAIL_ON_LSA_ERROR(dwError);

        dwError = LsaAllocateString(
                    pUser->userInfo.pszHomedir,
                    &pUserInfo0->pszHomedir);
        BAIL_ON_LSA_ERROR(dwError);

        dwError = LsaAllocateString(
                    pUser->pszObjectSid,
                    &pUserInfo0->pszSid);
        BAIL_ON_LSA_ERROR(dwError);
    }

    if (pUserInfo1 != NULL)
    {
        dwError = LsaStrDupOrNull(
                      pUser->userInfo.pszUPN,
                      &pUserInfo1->pszUPN);
        BAIL_ON_LSA_ERROR(dwError);

        pUserInfo1->bIsGeneratedUPN = pUser->userInfo.bIsGeneratedUPN;
        pUserInfo1->bIsLocalUser = FALSE;
        pUserInfo1->pLMHash = NULL;
        pUserInfo1->dwLMHashLen = 0;
        pUserInfo1->pNTHash = NULL;
        pUserInfo1->dwNTHashLen = 0;
    }

    if (pUserInfo2 != NULL)
    {
        struct timeval current_tv;
        UINT64 u64current_NTtime = 0;
        int64_t qwNanosecsToPasswordExpiry;

        if (gettimeofday(&current_tv, NULL) < 0)
        {
            dwError = errno;
            BAIL_ON_LSA_ERROR(dwError);
        }
        ADConvertTimeUnix2Nt(current_tv.tv_sec,
                             &u64current_NTtime);

        qwNanosecsToPasswordExpiry = gpADProviderData->adMaxPwdAge -
            (u64current_NTtime - pUser->userInfo.qwPwdLastSet);

        dwError = AD_UpdateUserObjectFlags(pUser);
        BAIL_ON_LSA_ERROR(dwError);

        if (pUser->userInfo.bPasswordNeverExpires ||
            gpADProviderData->adMaxPwdAge == 0)
        {
            //password never expires
            pUserInfo2->dwDaysToPasswordExpiry = 0LL;
        }
        else if (pUser->userInfo.bPasswordExpired)
        {
            //password is expired already
            pUserInfo2->dwDaysToPasswordExpiry = 0LL;
        }
        else
        {
            pUserInfo2->dwDaysToPasswordExpiry = qwNanosecsToPasswordExpiry /
                (10000000LL * 24*60*60);
        }
        pUserInfo2->bPasswordExpired = pUser->userInfo.bPasswordExpired;
        pUserInfo2->bPasswordNeverExpires = pUser->userInfo.bPasswordNeverExpires;
        pUserInfo2->bPromptPasswordChange = pUser->userInfo.bPromptPasswordChange;
        pUserInfo2->bUserCanChangePassword = pUser->userInfo.bUserCanChangePassword;
        pUserInfo2->bAccountDisabled = pUser->userInfo.bAccountDisabled;
        pUserInfo2->bAccountExpired = pUser->userInfo.bAccountExpired;
        pUserInfo2->bAccountLocked = pUser->userInfo.bAccountLocked;
    }

    *ppUserInfo = pUserInfo;

cleanup:

    return dwError;


error:
    if (pUserInfo) {
        LsaFreeUserInfo(dwUserInfoLevel, pUserInfo);
        pUserInfo = NULL;
    }

    *ppUserInfo = NULL;

    goto cleanup;
}

DWORD
ADGetCurrentNtTime(
    OUT UINT64* pqwResult
    )
{
    DWORD dwError = LSA_ERROR_SUCCESS;
    time_t now = 0;

    dwError = LsaGetCurrentTimeSeconds(&now);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = ADConvertTimeUnix2Nt(now, pqwResult);
    BAIL_ON_LSA_ERROR(dwError);

cleanup:
    return dwError;

error:
    *pqwResult = 0;
    goto cleanup;
}

DWORD
ADConvertTimeNt2Unix(
    UINT64 ntTime,
    PUINT64 pUnixTime
        )
{
    UINT64 unixTime = 0;

    unixTime = ntTime/10000000LL - 11644473600LL;

    *pUnixTime = unixTime;

    return 0;
}

DWORD
ADConvertTimeUnix2Nt(
    UINT64 unixTime,
    PUINT64 pNtTime
        )
{
    UINT64 ntTime = 0;

    ntTime = (unixTime+11644473600LL)*10000000LL;

    *pNtTime = ntTime;

    return 0;
}

DWORD
ADStr2UINT64(
    PSTR pszStr,
    PUINT64 pResult)
{
    UINT64 result = 0;

    if (pszStr){
        while (*pszStr != '\0')
        {
            result *= 10 ;
            result += *pszStr - '0';
            pszStr ++ ;
        }
    }

    *pResult = result;

    return 0;
}

DWORD
ADNonSchemaKeywordGetString(
    PSTR *ppszValues,
    DWORD dwNumValues,
    PCSTR pszAttributeName,
    PSTR *ppszResult
    )
{
    DWORD dwError = 0;
    size_t i;
    size_t sNameLen = strlen(pszAttributeName);
    PSTR pszResult = NULL;

    for (i = 0; i < dwNumValues; i++)
    {
        PCSTR pszValue = ppszValues[i];

        // Look for ldap values which are in the form <attributename>=<value>
        if (!strncasecmp(pszValue, pszAttributeName, sNameLen) &&
                pszValue[sNameLen] == '=')
        {
            dwError = LsaAllocateString(
                        pszValue + sNameLen + 1,
                        &pszResult);
            BAIL_ON_LSA_ERROR(dwError);
            break;
        }
    }

    *ppszResult = pszResult;

cleanup:
    return dwError;

error:
    *ppszResult = NULL;
    LSA_SAFE_FREE_STRING(pszResult);

    goto cleanup;
}

DWORD
ADNonSchemaKeywordGetUInt32(
    PSTR *ppszValues,
    DWORD dwNumValues,
    PCSTR pszAttributeName,
    DWORD *pdwResult
    )
{
    size_t i;
    size_t sNameLen = strlen(pszAttributeName);

    for (i = 0; i < dwNumValues; i++)
    {
        PCSTR pszValue = ppszValues[i];
        // Don't free this
        PSTR pszEndPtr = NULL;

        // Look for ldap values which are in the form <attributename>=<value>
        if (!strncasecmp(pszValue, pszAttributeName, sNameLen) &&
                pszValue[sNameLen] == '=')
        {
            pszValue += sNameLen + 1;
            *pdwResult = strtoul(pszValue, &pszEndPtr, 10);
            if (pszEndPtr == NULL || *pszEndPtr != '\0' || pszEndPtr == pszValue)
            {
                // Couldn't parse the whole number
                return LSA_ERROR_INVALID_LDAP_ATTR_VALUE;
            }
            return LSA_ERROR_SUCCESS;
        }
    }

    // Couldn't find the attribute
    return LSA_ERROR_INVALID_LDAP_ATTR_VALUE;
}

DWORD
AD_BuildHomeDirFromTemplate(
    PCSTR pszHomedirTemplate,
    PCSTR pszNetBIOSDomainName,
    PCSTR pszSamAccountName,
    PSTR* ppszHomedir
    )
{
    DWORD dwError = 0;
    PSTR pszHomedirPrefix = NULL;
    PSTR pszHomedir = NULL;
    DWORD dwOffset = 0;
    PCSTR pszIterTemplate = pszHomedirTemplate;
    DWORD dwBytesAllocated = 0;
    DWORD dwNetBIOSDomainNameLength = 0;
    DWORD dwSamAccountNameLength = 0;
    DWORD dwHomedirPrefixLength = 0;

    BAIL_ON_INVALID_STRING(pszHomedirTemplate);
    BAIL_ON_INVALID_STRING(pszNetBIOSDomainName);
    BAIL_ON_INVALID_STRING(pszSamAccountName);

    if (strstr(pszHomedirTemplate, "%H"))
    {
        dwError = AD_GetHomedirPrefixPath(&pszHomedirPrefix);
        BAIL_ON_LSA_ERROR(dwError);

        BAIL_ON_INVALID_STRING(pszHomedirPrefix);

        dwHomedirPrefixLength = strlen(pszHomedirPrefix);
    }

    dwNetBIOSDomainNameLength = strlen(pszNetBIOSDomainName);
    dwSamAccountNameLength = strlen(pszSamAccountName);

    // Handle common case where we might use all replacements.
    dwBytesAllocated = (strlen(pszHomedirTemplate) +
                        dwNetBIOSDomainNameLength +
                        dwSamAccountNameLength +
                        dwHomedirPrefixLength +
                        1);

    dwError = LsaAllocateMemory(
                    sizeof(CHAR) * dwBytesAllocated,
                    (PVOID*)&pszHomedir);
    BAIL_ON_LSA_ERROR(dwError);

    while (pszIterTemplate[0])
    {
        // Do not count the terminating NULL as "available".
        DWORD dwBytesRemaining = dwBytesAllocated - dwOffset - 1;
        PCSTR pszInsert = NULL;
        DWORD dwInsertLength = 0;
        BOOLEAN bNeedUpper = FALSE;
        BOOLEAN bNeedLower = FALSE;

        LSA_ASSERT(dwOffset < dwBytesAllocated);

        if (pszIterTemplate[0] == '%')
        {
            switch (pszIterTemplate[1])
            {
                case 'D':
                    pszInsert = pszNetBIOSDomainName;
                    dwInsertLength = dwNetBIOSDomainNameLength;
                    bNeedUpper = TRUE;
                    break;
                case 'U':
                    pszInsert = pszSamAccountName;
                    dwInsertLength = dwSamAccountNameLength;
                    bNeedLower = TRUE;
                    break;
                case 'H':
                    pszInsert = pszHomedirPrefix;
                    dwInsertLength = dwHomedirPrefixLength;
                    break;
                default:
                    dwError = LSA_ERROR_INVALID_HOMEDIR_TEMPLATE;
                    BAIL_ON_LSA_ERROR(dwError);
            }
            LSA_ASSERT(!(bNeedUpper && bNeedLower));
            pszIterTemplate += 2;
        }
        else
        {
            PCSTR pszEnd = strchr(pszIterTemplate, '%');
            if (!pszEnd)
            {
                dwInsertLength = strlen(pszIterTemplate);
            }
            else
            {
                dwInsertLength = pszEnd - pszIterTemplate;
            }
            pszInsert = pszIterTemplate;
            pszIterTemplate += dwInsertLength;
        }

        if (dwBytesRemaining < dwInsertLength)
        {
            // We will increment by at least a minimum amount.
            DWORD dwAllocate = LSA_MAX(dwInsertLength - dwBytesRemaining, 64);
            PSTR pszNewHomedir = NULL;
            dwError = LsaReallocMemory(
                            pszHomedir,
                            (PVOID*)&pszNewHomedir,
                            dwBytesAllocated + dwAllocate);
            BAIL_ON_LSA_ERROR(dwError);
            pszHomedir = pszNewHomedir;
            dwBytesAllocated += dwAllocate;
        }
        memcpy(pszHomedir + dwOffset,
               pszInsert,
               dwInsertLength);
        if (bNeedUpper)
        {
            LsaStrnToUpper(pszHomedir + dwOffset, dwInsertLength);
        }
        else if (bNeedLower)
        {
            LsaStrnToLower(pszHomedir + dwOffset, dwInsertLength);
        }
        dwOffset += dwInsertLength;
    }

    // We should still have enough room for NULL.
    LSA_ASSERT(dwOffset < dwBytesAllocated);

    pszHomedir[dwOffset] = 0;
    dwOffset++;

    *ppszHomedir = pszHomedir;

cleanup:
    LSA_SAFE_FREE_STRING(pszHomedirPrefix);

    return dwError;

error:
    *ppszHomedir = NULL;
    LSA_SAFE_FREE_MEMORY(pszHomedir);

    goto cleanup;
}

/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
