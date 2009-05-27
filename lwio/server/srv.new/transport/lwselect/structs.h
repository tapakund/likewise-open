#ifndef __STRUCTS_H__
#define __STRUCTS_H__

typedef struct _LWIO_SRV_SOCKET_READER_CONTEXT
{
    pthread_mutex_t  mutex;
    pthread_mutex_t* pMutex;

    ULONG          readerId;

    BOOLEAN        bStop;
    BOOLEAN        bActive;

    PLWRTL_RB_TREE pConnections;
    ULONG          ulNumSockets;

    PSMB_PROD_CONS_QUEUE pWorkQueue;

    // pipe used to interrupt the reader
    int fd[2];

} LWIO_SRV_SOCKET_READER_CONTEXT, *PLWIO_SRV_SOCKET_READER_CONTEXT;

typedef struct _LWIO_SRV_SOCKET_READER
{
    pthread_t  reader;
    pthread_t* pReader;

    ULONG      readerId;

    LWIO_SRV_SOCKET_READER_CONTEXT context;

} LWIO_SRV_SOCKET_READER, *PLWIO_SRV_SOCKET_READER;

typedef struct _LWIO_SRV_LISTENER_CONTEXT
{
    pthread_mutex_t  mutex;
    pthread_mutex_t* pMutex;

    BOOLEAN bStop;

    SRV_PROPERTIES serverProperties;

    // Invariant
    // Not owned
    HANDLE                    hPacketAllocator;
    HANDLE                    hGssContext;
    PLWIO_SRV_SHARE_LIST      pShareList;
    PLWIO_SRV_SOCKET_READER   pReaderArray;
    ULONG                     ulNumReaders;

} LWIO_SRV_LISTENER_CONTEXT, *PLWIO_SRV_LISTENER_CONTEXT;

typedef struct _LWIO_SRV_LISTENER
{
    pthread_t  listener;
    pthread_t* pListener;

    LWIO_SRV_LISTENER_CONTEXT context;

} LWIO_SRV_LISTENER, *PLWIO_SRV_LISTENER;

#endif /* __STRUCTS_H__ */
