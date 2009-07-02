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
 *        acceptsecctxt.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        AcceptSecurityContext client wrapper API
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Marc Guy (mguy@likewisesoftware.com)
 */

#include <ntlmsrvapi.h>

DWORD
NtlmServerAcceptSecurityContext(
    PCredHandle phCredential,
    PCtxtHandle phContext,
    PSecBufferDesc pInput,
    DWORD fContextReq,
    DWORD TargetDataRep,
    PCtxtHandle phNewContext,
    PSecBufferDesc pOutput,
    PDWORD pfContextAttr,
    PTimeStamp ptsTimeStamp
    )
{
    DWORD dwError = LSA_ERROR_SUCCESS;
    PNTLM_CONTEXT pNtlmCtxtOut = NULL;
    PNTLM_CONTEXT pNtlmCtxtIn = NULL;
    PNTLM_CONTEXT pNtlmCtxtChlng = NULL;
    BOOLEAN bInLock = FALSE;

    if(ptsTimeStamp)
    {
        memset(ptsTimeStamp, 0, sizeof(TimeStamp));
    }

    if(!phContext)
    {
        dwError = NtlmInitContext(&pNtlmCtxtIn);
        BAIL_ON_NTLM_ERROR(dwError);

        pNtlmCtxtIn->NtlmState = NtlmStateNegotiate;
        pNtlmCtxtIn->pMessage = pInput;
    }
    else
    {
        // The only time we should get a context handle passed in is when
        // we are validating a challenge and we need to look up the original
        // challenge sent
        ENTER_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);
            dwError = NtlmFindContext(phContext, &pNtlmCtxtChlng);
        LEAVE_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);

        BAIL_ON_NTLM_ERROR(dwError);

        // In this case we need to build up a temp context for the response
        // message sent in
        dwError = NtlmInitContext(&pNtlmCtxtIn);
        BAIL_ON_NTLM_ERROR(dwError);

        pNtlmCtxtIn->NtlmState = NtlmStateResponse;
        pNtlmCtxtIn->pMessage = pInput;
    }

    switch(pNtlmCtxtIn->NtlmState)
    {
    case NtlmStateNegotiate:
        {
            // If we start with a blank context, upgrade it to a challenge
            // message
            dwError = NtlmCreateChallengeContext(pNtlmCtxtIn, &pNtlmCtxtOut);

            BAIL_ON_NTLM_ERROR(dwError);
        }
        break;
    case NtlmStateResponse:
        {
            //... authenticate the response
            dwError = NtlmValidateResponse(pNtlmCtxtIn, pNtlmCtxtChlng);
            BAIL_ON_NTLM_ERROR(dwError);

            dwError = NtlmFreeContext(pNtlmCtxtChlng);
            BAIL_ON_NTLM_ERROR(dwError);

            pNtlmCtxtChlng = NULL;
        }
        break;
    default:
        dwError = LSA_ERROR_INTERNAL;
        BAIL_ON_NTLM_ERROR(dwError);
    }

    // TODO - copy message to the output parameter

    if(pNtlmCtxtOut)
    {
        ENTER_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);
        //
            dwError = NtlmInsertContext(
                pNtlmCtxtOut
                );
        //
        LEAVE_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);

        BAIL_ON_NTLM_ERROR(dwError);

        *phNewContext = pNtlmCtxtOut->ContextHandle;

        dwError = LSA_WARNING_CONTINUE_NEEDED;
    }


cleanup:
    return(dwError);
error:
    if(phNewContext)
    {
        ENTER_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);
            NtlmRemoveContext(phNewContext);
        LEAVE_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);
        memset(phNewContext, 0 , sizeof(CtxtHandle));
    }
    if(pNtlmCtxtChlng)
    {
        ENTER_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);
            NtlmRemoveContext(&pNtlmCtxtChlng->ContextHandle);
        LEAVE_NTLM_CONTEXT_LIST_WRITER_LOCK(bInLock);
    }
    goto cleanup;
}

DWORD
NtlmCreateChallengeContext(
    IN PNTLM_CONTEXT pNtlmNegCtxt,
    OUT PNTLM_CONTEXT *ppNtlmContext
    )
{
    DWORD dwError = LSA_ERROR_SUCCESS;
    PNTLM_CHALLENGE_MESSAGE pChlngMsg = NULL;

    if(!ppNtlmContext)
    {
        dwError = LSA_ERROR_INVALID_PARAMETER;
        BAIL_ON_NTLM_ERROR(dwError);
    }

    dwError = NtlmInitContext(ppNtlmContext);

    BAIL_ON_NTLM_ERROR(dwError);

    dwError = NtlmCreateChallengeMessage(
        (PNTLM_NEGOTIATE_MESSAGE)pNtlmNegCtxt->pMessage,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &pChlngMsg
        );

    BAIL_ON_NTLM_ERROR(dwError);

    (*ppNtlmContext)->pMessage = (PVOID)pChlngMsg;
    (*ppNtlmContext)->NtlmState = NtlmStateChallenge;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
NtlmValidateResponse(
    PNTLM_CONTEXT pRespCtxt,
    PNTLM_CONTEXT pChlngCtxt
    )
{
    return LSA_ERROR_SUCCESS;
}
