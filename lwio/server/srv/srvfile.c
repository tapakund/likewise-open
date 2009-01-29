#include "includes.h"

static
VOID
SrvFileFree(
    PSMB_SRV_FILE pFile
    );

NTSTATUS
SrvFileCreate(
    USHORT                  fid,
    PIO_FILE_HANDLE*        pphFile,
    PIO_STATUS_BLOCK*       ppIoStatusBlock,
    PIO_FILE_NAME*          ppFilename,
    ACCESS_MASK             desiredAccess,
    LONG64                  allocationSize,
    FILE_ATTRIBUTES         fileAttributes,
    FILE_SHARE_FLAGS        shareAccess,
    FILE_CREATE_DISPOSITION createDisposition,
    FILE_CREATE_OPTIONS     createOptions,
    PSMB_SRV_FILE*          ppFile
    )
{
    NTSTATUS ntStatus = 0;
    PSMB_SRV_FILE pFile = NULL;

    SMB_LOG_DEBUG("Creating file [fid:%u]", fid);

    ntStatus = SMBAllocateMemory(
                    sizeof(SMB_SRV_FILE),
                    (PVOID*)&pFile);
    BAIL_ON_NT_STATUS(ntStatus);

    pFile->refcount = 1;

    pthread_rwlock_init(&pFile->mutex, NULL);
    pFile->pMutex = &pFile->mutex;

    pFile->fid = fid;
    pFile->phFile = *pphFile;
    *pphFile = NULL;
    pFile->pIoStatusBlock = *ppIoStatusBlock;
    *ppIoStatusBlock = NULL;
    pFile->pFilename = *ppFilename;
    *ppFilename = NULL;
    pFile->desiredAccess = desiredAccess;
    pFile->allocationSize = allocationSize;
    pFile->fileAttributes = fileAttributes;
    pFile->shareAccess = shareAccess;
    pFile->createDisposition = createDisposition;
    pFile->createOptions = createOptions;

    SMB_LOG_DEBUG("Associating file [object:0x%x][fid:%u]",
                    pFile,
                    fid);

    *ppFile = pFile;

cleanup:

    return ntStatus;

error:

    *ppFile = NULL;

    if (pFile)
    {
        SrvFileRelease(pFile);
    }

    goto cleanup;
}

VOID
SrvFileRelease(
    PSMB_SRV_FILE pFile
    )
{
    SMB_LOG_DEBUG("Releasing file [fid:%u]", pFile->fid);

    if (InterlockedDecrement(&pFile->refcount) == 0)
    {
        SrvFileFree(pFile);
    }
}

static
VOID
SrvFileFree(
    PSMB_SRV_FILE pFile
    )
{
    SMB_LOG_DEBUG("Freeing file [object:0x%x][fid:%u]",
                    pFile,
                    pFile->fid);

    if (pFile->pMutex)
    {
        pthread_rwlock_destroy(&pFile->mutex);
        pFile->pMutex = NULL;
    }

    SMB_SAFE_FREE_MEMORY(pFile->pIoStatusBlock);
    if (pFile->pFilename)
    {
        SMB_SAFE_FREE_MEMORY(pFile->pFilename->FileName);
        SMBFreeMemory(pFile->pFilename);
    }

    if (pFile->phFile)
    {
        IoCloseFile(*pFile->phFile);
        SMBFreeMemory(pFile->phFile);
    }

    SMBFreeMemory(pFile);
}
