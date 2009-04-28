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
 *        unprov.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        Active Directory Authentication Provider
 *
 * Authors:
 *          Wei Fu (wfu@likewisesoftware.com)
 */

#include "adprovider.h"

//By default, ENABLE_ALIAS_TO_BE_SAMACCOUNT_NAME should not be enabled
#if 0
#define ENABLE_ALIAS_TO_BE_SAMACCOUNT_NAME
#endif

static
DWORD
ADUnprovPlugin_QueryByIdWithDomainName(
    IN BOOLEAN bIsUser,
    IN DWORD dwId,
    IN PCSTR pszDnsDomainName,
    OUT PSTR* ppszSid,
    OUT PSTR* ppszAlias
    )
{
    DWORD dwError = 0;
    PSID pDomainSid = NULL;
    PSTR pszDomainSid = NULL;
    PSTR pszSid = NULL;
    PSTR pszAlias = NULL;

    // lsass unprovisioned mode converts a group/user uid/gid to objectSid/name with the same algorithm

    dwError = LsaDmQueryDomainInfo(pszDnsDomainName,
                                   NULL,
                                   NULL,
                                   &pDomainSid,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);
    if (LSA_ERROR_NO_SUCH_DOMAIN == dwError)
    {
        LSA_LOG_WARNING("Domain '%s' is unknown.", pszDnsDomainName);
    }
    BAIL_ON_LSA_ERROR(dwError);

    dwError = AD_SidToString(
                 pDomainSid,
                 &pszDomainSid);
    BAIL_ON_LSA_ERROR(dwError);

    BAIL_ON_INVALID_STRING(pszDomainSid);

    dwError = UnprovisionedModeMakeLocalSID(
                     pszDomainSid,
                     dwId,
                     &pszSid);
    BAIL_ON_LSA_ERROR(dwError);

    *ppszSid = pszSid;
    *ppszAlias = pszAlias;

cleanup:
    LSA_SAFE_FREE_MEMORY(pDomainSid);
    LSA_SAFE_FREE_STRING(pszDomainSid);

    return dwError;

error:
    LSA_SAFE_FREE_STRING(pszSid);
    LSA_SAFE_FREE_STRING(pszAlias);
    *ppszSid = NULL;
    *ppszAlias = NULL;

    goto cleanup;
}

static
DWORD
ADUnprovPlugin_QueryByAliasWithDomainName(
    IN BOOLEAN bIsUser,
    IN PCSTR pszAlias,
    IN PCSTR pszDnsDomainName,
    OUT PSTR* ppszSid,
    OUT PDWORD pdwId
    )
{
    DWORD dwError = 0;
    PSTR pszSid = NULL;
    DWORD dwId = 0;
    BOOLEAN bIsFoundObjectUser = !bIsUser;
    PSTR pszNT4Name = NULL;
    PSTR pszNetBiosName = NULL;

    dwError = LsaDmWrapGetDomainName(
                 pszDnsDomainName,
                 NULL,
                 &pszNetBiosName);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = LsaAllocateStringPrintf(
                 &pszNT4Name,
                 "%s\\%s",
                 pszNetBiosName,
                 pszAlias);
    BAIL_ON_LSA_ERROR(dwError);

    dwError = LsaDmWrapNetLookupObjectSidByName(
                    gpADProviderData->szDomain,
                    pszNT4Name,
                    &pszSid,
                    &bIsFoundObjectUser);
    BAIL_ON_LSA_ERROR(dwError);

    if (bIsUser != bIsFoundObjectUser)
    {
        dwError = LSA_ERROR_INTERNAL;
        BAIL_ON_LSA_ERROR(dwError);
    }

    *ppszSid = pszSid;
    *pdwId = dwId;

cleanup:
    LSA_SAFE_FREE_STRING(pszNT4Name);
    LSA_SAFE_FREE_STRING(pszNetBiosName);
    return dwError;

error:
    LSA_SAFE_FREE_STRING(pszSid);
    *ppszSid = NULL;
    *pdwId = 0;

    goto cleanup;
}

// Dummy AD unprov plugin initializer
// called during ad-provider init
DWORD
ADUnprovPlugin_Initialize(
    VOID
    )
{
    return LSA_ERROR_SUCCESS;
}

VOID
ADUnprovPlugin_Cleanup(
    VOID
    )
{
    // Dummy AD unprov plugin cleanup
    // Called during ad-provider shutdown
}

BOOLEAN
ADUnprovPlugin_SupportsAliases(
    VOID
    )
{
#ifdef ENABLE_ALIAS_TO_BE_SAMACCOUNT_NAME
    return TRUE;
#else
    return FALSE;
#endif
}

DWORD
ADUnprovPlugin_QueryByReal(
    IN BOOLEAN bIsUser,
    IN PCSTR pszNT4Name,
    IN PCSTR pszSid,
    OUT OPTIONAL PSTR* ppszAlias,
    OUT PDWORD pdwId
    )
{
    DWORD dwError = 0;
    DWORD dwId = 0;
    PSTR pszAlias = NULL;
#ifdef ENABLE_ALIAS_TO_BE_SAMACCOUNT_NAME
    PLSA_LOGIN_NAME_INFO pNameInfo = NULL;
    PSTR pszName = NULL;
    BOOLEAN bFoundIsUser = !bIsUser;
#endif

    // lsass unprovisioned mode converts a group/user objectSid to uid/gid with the same algorithm

    dwError = LsaHashSidStringToId(pszSid, &dwId);
    BAIL_ON_LSA_ERROR(dwError);

    // can add alias here
#ifdef ENABLE_ALIAS_TO_BE_SAMACCOUNT_NAME
    if (IsNullOrEmptyString(pszNT4Name))
    {
        dwError = LsaDmWrapNetLookupNameByObjectSid(
                    gpADProviderData->szDomain,
                    pszSid,
                    &pszName,
                    &bFoundIsUser);
        BAIL_ON_LSA_ERROR(dwError);

        if (bFoundIsUser != bIsUser)
        {
            dwError = LSA_ERROR_INTERNAL;
            BAIL_ON_LSA_ERROR(dwError);
        }
    }

    dwError = LsaCrackDomainQualifiedName(
                 !IsNullOrEmptyString(pszNT4Name) ? pszNT4Name : pszName,
                 gpADProviderData->szDomain,
                 &pNameInfo);
    BAIL_ON_LSA_ERROR(dwError);

    if (pNameInfo && !IsNullOrEmptyString(pNameInfo->pszName))
    {
        dwError = LsaAllocateString(pNameInfo->pszName,
                                    &pszAlias);
        BAIL_ON_LSA_ERROR(dwError);
    }
#endif

    if (ppszAlias)
    {
        *ppszAlias = pszAlias;
    }
    *pdwId = dwId;

cleanup:
#ifdef ENABLE_ALIAS_TO_BE_SAMACCOUNT_NAME
    LSA_SAFE_FREE_LOGIN_NAME_INFO(pNameInfo);
    LSA_SAFE_FREE_STRING(pszName);
#endif

    return dwError;

error:
    LSA_SAFE_FREE_STRING(pszAlias);
    if (ppszAlias)
    {
        *ppszAlias = NULL;
    }
    *pdwId = 0;

    goto cleanup;
}

DWORD
ADUnprovPlugin_QueryByAlias(
    IN BOOLEAN bIsUser,
    IN PCSTR pszAlias,
    OUT PSTR* ppszSid,
    OUT PDWORD pdwId
    )
{
    DWORD dwError = LSA_ERROR_SUCCESS;
    PSTR* ppszDomainNames = NULL;
    DWORD dwDomainCount = 0;
    DWORD i = 0;

    // lsass unprovisioned mode converts a group/user uid/gid to objectSid/name with the same algorithm

    dwError = ADUnprovPlugin_QueryByAliasWithDomainName(
                    bIsUser,
                    pszAlias,
                    gpADProviderData->szDomain,
                    ppszSid,
                    pdwId);
    if (LSA_ERROR_NO_SUCH_OBJECT == dwError ||
        LSA_ERROR_NO_SUCH_DOMAIN == dwError)
    {
        dwError = 0;
    }
    BAIL_ON_LSA_ERROR(dwError);


    if (!IsNullOrEmptyString(*ppszSid))
        goto cleanup;

    dwError = LsaDmEnumDomainNames(NULL, NULL, &ppszDomainNames, &dwDomainCount);
    BAIL_ON_LSA_ERROR(dwError);

    for (i = 0; i < dwDomainCount; i++)
    {
        dwError = ADUnprovPlugin_QueryByAliasWithDomainName(
                        bIsUser,
                        pszAlias,
                        ppszDomainNames[i],
                        ppszSid,
                        pdwId);
        if (LSA_ERROR_NO_SUCH_OBJECT == dwError ||
            LSA_ERROR_NO_SUCH_DOMAIN == dwError)
        {
            dwError = 0;
            continue;
        }
        BAIL_ON_LSA_ERROR(dwError);

        if (!IsNullOrEmptyString(*ppszSid))
            goto cleanup;
    }

cleanup:
    if (dwError)
    {
        *ppszSid = NULL;
    }

    LsaFreeStringArray(ppszDomainNames, dwDomainCount);

    return dwError;

error:
    goto cleanup;
}

// Must return at least one of NT4 or SID.
// Can optionally return alias as well.
DWORD
ADUnprovPlugin_QueryById(
    IN BOOLEAN bIsUser,
    IN DWORD dwId,
    OUT PSTR* ppszSid,
    OUT PSTR* ppszAlias
    )
{
    DWORD dwError = LSA_ERROR_SUCCESS;
    PSTR* ppszDomainNames = NULL;
    DWORD dwDomainCount = 0;
    DWORD i = 0;

    // lsass unprovisioned mode converts a group/user uid/gid to objectSid/name with the same algorithm

    dwError = ADUnprovPlugin_QueryByIdWithDomainName(
                    bIsUser,
                    dwId,
                    gpADProviderData->szDomain,
                    ppszSid,
                    ppszAlias);
    if (LSA_ERROR_NO_SUCH_OBJECT == dwError ||
        LSA_ERROR_NO_SUCH_DOMAIN == dwError)
    {
        dwError = 0;
    }
    BAIL_ON_LSA_ERROR(dwError);


    if (!IsNullOrEmptyString(*ppszSid))
        goto cleanup;

    dwError = LsaDmEnumDomainNames(NULL, NULL, &ppszDomainNames, &dwDomainCount);
    BAIL_ON_LSA_ERROR(dwError);

    for (i = 0; i < dwDomainCount; i++)
    {
        dwError = ADUnprovPlugin_QueryByIdWithDomainName(
                        bIsUser,
                        dwId,
                        ppszDomainNames[i],
                        ppszSid,
                        ppszAlias);
        if (LSA_ERROR_NO_SUCH_OBJECT == dwError ||
            LSA_ERROR_NO_SUCH_DOMAIN == dwError)
        {
            dwError = 0;
            continue;
        }
        BAIL_ON_LSA_ERROR(dwError);

        if (!IsNullOrEmptyString(*ppszSid))
            goto cleanup;
    }

cleanup:
    if (dwError)
    {
        *ppszSid = NULL;
        *ppszAlias = NULL;
    }

    LsaFreeStringArray(ppszDomainNames, dwDomainCount);

    return dwError;

error:
    goto cleanup;
}
