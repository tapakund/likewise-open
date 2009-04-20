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
 *        lpmisc.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        Local Authentication Provider
 *
 *        Miscellaneous
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 */

#include "includes.h"

DWORD
LocalCrackDomainQualifiedName(
    PCSTR pszId,
    PLSA_LOGIN_NAME_INFO* ppNameInfo
    )
{
    DWORD dwError = 0;
    PLSA_LOGIN_NAME_INFO pNameInfo = NULL;

    dwError = LsaCrackDomainQualifiedName(
                    pszId,
                    gLPGlobals.pszNetBIOSName,
                    &pNameInfo);
    BAIL_ON_LSA_ERROR(dwError);

    // TODO: Handle aliases
    //       Handle AD Domains
    if (!strcasecmp(pNameInfo->pszDomainNetBiosName,
                    gLPGlobals.pszBuiltinDomain))
    {
        LSA_SAFE_FREE_STRING(pNameInfo->pszFullDomainName);

        dwError = LsaAllocateString(
                        gLPGlobals.pszBuiltinDomain,
                        &pNameInfo->pszFullDomainName);
        BAIL_ON_LSA_ERROR(dwError);
    }
    else if (!strcasecmp(pNameInfo->pszDomainNetBiosName,
                         gLPGlobals.pszNetBIOSName))
    {
        LSA_SAFE_FREE_STRING(pNameInfo->pszFullDomainName);

        dwError = LsaAllocateString(
                        gLPGlobals.pszLocalDomain,
                        &pNameInfo->pszFullDomainName);
        BAIL_ON_LSA_ERROR(dwError);
    }


    *ppNameInfo = pNameInfo;

cleanup:

    return dwError;

error:

    *ppNameInfo = NULL;

    if (pNameInfo)
    {
        LsaFreeNameInfo(pNameInfo);
    }

    goto cleanup;
}

DWORD
LocalBuildDN(
    PLSA_LOGIN_NAME_INFO pLoginInfo,
    PWSTR*               ppwszDN
    )
{
    DWORD dwError = 0;
    WCHAR wszCNPrefix[] = LOCAL_DIR_CN_PREFIX;
    PWSTR pwszName = NULL;
    PWSTR pwszDN = NULL;

    dwError = LsaMbsToWc16s(
                    pLoginInfo->pszName,
                    &pwszName);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = LsaAllocateMemory(
                    sizeof(wszCNPrefix) + strlen(pLoginInfo->pszName) * sizeof(WCHAR),
                    (PVOID*)&pwszDN);
    BAIL_ON_LSA_ERROR(dwError);

    // Build CN=<sam account name>
    memcpy((PBYTE)pwszDN, (PBYTE)&wszCNPrefix[0], sizeof(wszCNPrefix) - sizeof(WCHAR));

    memcpy((PBYTE)(pwszDN + sizeof(wszCNPrefix) - sizeof(WCHAR)),
           (PBYTE)pwszName,
           strlen(pLoginInfo->pszName) * sizeof(WCHAR));

    *ppwszDN = pwszDN;

cleanup:

    LSA_SAFE_FREE_MEMORY(pwszName);

    return dwError;

error:

    *ppwszDN = NULL;

    LSA_SAFE_FREE_MEMORY(pwszDN);

    goto cleanup;
}

LONG64
LocalGetNTTime(
    time_t timeVal
    )
{
    return (timeVal + 11644473600LL) * 10000000LL;
}

DWORD
LocalBuildHomeDirPathFromTemplate(
    PCSTR pszSamAccountName,
    PCSTR pszNetBIOSDomainName,
    PSTR* ppszHomedir
    )
{
    DWORD dwError = 0;
    PSTR pszHomedir = NULL;
    DWORD dwOffset = 0;
    PSTR  pszHomedirPrefix = NULL;
    PSTR  pszHomedirTemplate = NULL;
    PCSTR pszIterTemplate = NULL;
    DWORD dwBytesAllocated = 0;
    DWORD dwNetBIOSDomainNameLength = 0;
    DWORD dwSamAccountNameLength = 0;
    DWORD dwHomedirPrefixLength = 0;
    PSTR pszHostName = NULL;
    DWORD dwHostNameLength = 0;

    BAIL_ON_INVALID_STRING(pszNetBIOSDomainName);
    BAIL_ON_INVALID_STRING(pszSamAccountName);

    dwError = LocalCfgGetHomedirTemplate(&pszHomedirTemplate);
    BAIL_ON_LSA_ERROR(dwError);

    pszIterTemplate = pszHomedirTemplate;

    if (strstr(pszHomedirTemplate, "%H"))
    {
        dwError = LocalCfgGetHomedirPrefix(&pszHomedirPrefix);
        BAIL_ON_LSA_ERROR(dwError);

        BAIL_ON_INVALID_STRING(pszHomedirPrefix);

        dwHomedirPrefixLength = strlen(pszHomedirPrefix);
    }

    if (strstr(pszHomedirTemplate, "%L"))
    {
        dwError = LsaDnsGetHostInfo(&pszHostName);
        BAIL_ON_LSA_ERROR(dwError);

        BAIL_ON_INVALID_STRING(pszHostName);

        dwHostNameLength = strlen(pszHostName);
    }

    dwNetBIOSDomainNameLength = strlen(pszNetBIOSDomainName);
    dwSamAccountNameLength = strlen(pszSamAccountName);

    // Handle common case where we might use all replacements.
    dwBytesAllocated = (strlen(pszHomedirTemplate) +
                        dwNetBIOSDomainNameLength +
                        dwSamAccountNameLength +
                        dwHomedirPrefixLength +
                        dwHostNameLength +
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
                case 'L':
                    pszInsert = pszHostName;
                    dwInsertLength = dwHostNameLength;
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

    LSA_SAFE_FREE_STRING(pszHomedirTemplate);
    LSA_SAFE_FREE_STRING(pszHomedirPrefix);
    LSA_SAFE_FREE_STRING(pszHostName);

    return dwError;

error:

    *ppszHomedir = NULL;

    LSA_SAFE_FREE_MEMORY(pszHomedir);

    goto cleanup;
}

