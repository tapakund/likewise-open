#ifndef _SVCCTL_REGISTER_H_
#define _SVCCTL_REGISTER_H_

#include <dce/rpc.h>


typedef struct dcerpc_endpoint {
    PCSTR pszProtocol;
    PCSTR pszEndpoint;
} ENDPOINT, *PENDPOINT;


DWORD
RpcSvcRegisterRpcInterface(
    rpc_if_handle_t SrvInterface
    );


DWORD
RpcSvcBindRpcInterface(
    rpc_binding_vector_p_t pSrvBinding,
    rpc_if_handle_t SrvInterface,
    PENDPOINT pEndPoints,
    PSTR pszSrvDescription
    );


DWORD
RpcSvcUnregisterRpcInterface(
    rpc_if_handle_t SrvInterface
    );


DWORD
RpcSvcUnbindRpcInterface(
    rpc_binding_vector_p_t pSrvBinding,
    rpc_if_handle_t SrvInterface
    );


#endif /* _SVCCTL_REGISTER_H_ */
