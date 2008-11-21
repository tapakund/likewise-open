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
 *        batch_common.h
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        Active Directory Authentication Provider
 *
 * Authors: Danilo Almeida (dalmeida@likewisesoftware.com)
 */

#ifndef _BATCH_COMMON_H_
#define _BATCH_COMMON_H_

#define XXX

#define SetFlag(Variable, Flags)   ((Variable) |= (Flags))
#define ClearFlag(Variable, Flags) ((Variable) &= ~(Flags))
#define IsSetFlag(Variable, Flags) (((Variable) & (Flags)) != 0)

#define LSA_XFER_STRING(Source, Target) \
    ((Target) = (Source), (Source) = NULL)


typedef UINT8 LSA_AD_BATCH_DOMAIN_ENTRY_FLAGS;
// If this is set, we are not supposed to process
// this domain.
#define LSA_AD_BATCH_DOMAIN_ENTRY_FLAG_SKIP             0x01
// If this is set, we are dealing with one-way trust scenario.
#define LSA_AD_BATCH_DOMAIN_ENTRY_FLAG_IS_ONE_WAY_TRUST 0x02


typedef UINT8 LSA_AD_BATCH_QUERY_TYPE, *PLSA_AD_BATCH_QUERY_TYPE;
#define LSA_AD_BATCH_QUERY_TYPE_UNDEFINED      0
#define LSA_AD_BATCH_QUERY_TYPE_BY_DN          1
#define LSA_AD_BATCH_QUERY_TYPE_BY_SID         2
#define LSA_AD_BATCH_QUERY_TYPE_BY_NT4         3
#define LSA_AD_BATCH_QUERY_TYPE_BY_USER_ALIAS  4
#define LSA_AD_BATCH_QUERY_TYPE_BY_GROUP_ALIAS 5
#define LSA_AD_BATCH_QUERY_TYPE_BY_UID         6
#define LSA_AD_BATCH_QUERY_TYPE_BY_GID         7


typedef UINT8 LSA_AD_BATCH_OBJECT_TYPE, *PLSA_AD_BATCH_OBJECT_TYPE;
#define LSA_AD_BATCH_OBJECT_TYPE_UNDEFINED 0
#define LSA_AD_BATCH_OBJECT_TYPE_USER      1
#define LSA_AD_BATCH_OBJECT_TYPE_GROUP     2


typedef UINT8 LSA_AD_BATCH_ITEM_FLAGS, *PLSA_AD_BATCH_ITEM_FLAGS;
#define LSA_AD_BATCH_ITEM_FLAG_HAVE_PSEUDO  0x01
#define LSA_AD_BATCH_ITEM_FLAG_HAVE_REAL    0x02
#define LSA_AD_BATCH_ITEM_FLAG_DISABLED     0x04
#define LSA_AD_BATCH_ITEM_FLAG_ERROR        0x08



typedef struct _LSA_AD_BATCH_DOMAIN_ENTRY {
    PSTR pszDnsDomainName;
    PSTR pszNetbiosDomainName;
    LSA_AD_BATCH_DOMAIN_ENTRY_FLAGS Flags;
    LSA_AD_BATCH_QUERY_TYPE QueryType;

    // The presence of these depend on the query type.
    union {
        struct {
            // pszDcPart is not allocated, but points to
            // strings that last longer than this structure.
            PCSTR pszDcPart;
        } ByDn;
        struct {
            // Allocated
            PSTR pszDomainSid;
            size_t sDomainSidLength;
        } BySid;
    } QueryMatch;

    // Number of items in BatchItemList.
    DWORD dwBatchItemCount;
    // List of LSA_AD_BATCH_ITEM.
    LSA_LIST_LINKS BatchItemList;

    // Links for list of domain entry.
    LSA_LIST_LINKS DomainEntryListLinks;
} LSA_AD_BATCH_DOMAIN_ENTRY, *PLSA_AD_BATCH_DOMAIN_ENTRY;

typedef struct _LSA_AD_BATCH_QUERY_TERM {
    LSA_AD_BATCH_QUERY_TYPE Type;
    union {
        // This can be a DN, SID, NT4 name, or alias.
        // It is not allocated.  Rather, it points to data
        // that lasts longer than this structure.
        PCSTR pszString;
        // This can be a uid or gid.
        DWORD dwId;
    };
} LSA_AD_BATCH_QUERY_TERM, *PLSA_AD_BATCH_QUERY_TERM;

typedef struct _LSA_AD_BATCH_ITEM_USER_INFO {
    // Unix fields in struct passwd order:
    PSTR pszAlias;
    PSTR pszPasswd;
    uid_t uid;
    gid_t gid;
    PSTR pszGecos;
    PSTR pszHomeDirectory;
    PSTR pszShell;
    // AD-specific fields:
    PSTR pszUserPrincipalName;
    DWORD dwPrimaryGroupRid;
    UINT32 UserAccountControl;
    UINT64 AccountExpires;
    UINT64 PasswordLastSet;
} LSA_AD_BATCH_ITEM_USER_INFO, *PLSA_AD_BATCH_ITEM_USER_INFO;

typedef struct _LSA_AD_BATCH_ITEM_GROUP_INFO {
    // Unix fields in struct group order:
    PSTR pszAlias;
    PSTR pszPasswd;
    gid_t gid;
} LSA_AD_BATCH_ITEM_GROUP_INFO, *PLSA_AD_BATCH_ITEM_GROUP_INFO;

XXX; // remove pDomainEntry as we should not need it.
XXX; // eventually remove DN field...
typedef struct _LSA_AD_BATCH_ITEM {
    LSA_AD_BATCH_QUERY_TERM QueryTerm;
    PCSTR pszQueryMatchTerm;
    PLSA_AD_BATCH_DOMAIN_ENTRY pDomainEntry;
    LSA_LIST_LINKS BatchItemListLinks;
    LSA_AD_BATCH_ITEM_FLAGS Flags;

    // Non-specific fields:
    PSTR pszSid;
    PSTR pszSamAccountName;
    PSTR pszDn;
    LSA_AD_BATCH_OBJECT_TYPE ObjectType;
    // User/Group-specific fields:
    union {
        LSA_AD_BATCH_ITEM_USER_INFO UserInfo;
        LSA_AD_BATCH_ITEM_GROUP_INFO GroupInfo;
    };
} LSA_AD_BATCH_ITEM, *PLSA_AD_BATCH_ITEM;


BOOLEAN
LsaAdBatchIsDefaultSchemaMode(
    VOID
    );

BOOLEAN
LsaAdBatchIsUnprovisionedMode(
    VOID
    );

VOID
LsaAdBatchQueryTermDebugInfo(
    IN PLSA_AD_BATCH_QUERY_TERM pQueryTerm,
    OUT OPTIONAL PCSTR* ppszType,
    OUT OPTIONAL PBOOLEAN pbIsString,
    OUT OPTIONAL PCSTR* ppszString,
    OUT OPTIONAL PDWORD pdwId
    );



#endif /* BATCH_COMMON_H_ */
