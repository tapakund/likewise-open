/*
 * lwregerror-table.h
 *
 *  Created on: Nov 13, 2009
 *      Author: wfu
 */

#ifndef LWREGERRORTABLE_H_
#define LWREGERRORTABLE_H_

/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

/*
 *
 *   NT status and Win32 error code mapping table
 *
 *   Some of these definitions were written from scratch.
 *
 *   NT status codes are unsigned 32-bit hex values
 *
 *   Win32 error codes are unsigned 32-bit decimal values
 */

/* Win32 codes*/
#ifdef __ERROR_WIN32__

#include <lw/winerror.h>

#endif

/* Equivalence table*/
#ifdef __ERROR_XMACRO__
#define S STATUS_CODE
// List of lw extended errors
S ( LW_STATUS_SUCCESS, ERROR_SUCCESS, 0, "" )
S ( LW_STATUS_BUFFER_TOO_SMALL, ERROR_INSUFFICIENT_BUFFER, -1, "" )
S ( LW_STATUS_INSUFFICIENT_RESOURCES, ERROR_OUTOFMEMORY, -1, "" )
S ( LW_STATUS_NOT_IMPLEMENTED, LWREG_ERROR_NOT_IMPLEMENTED, -1, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_REGEX_COMPILE_FAILED, -1, "" )
S ( LW_STATUS_INTERNAL_ERROR, ERROR_INTERNAL_ERROR, -1, "" )
S ( LW_STATUS_INVALID_PARAMETER, ERROR_INVALID_PARAMETER, EINVAL, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, ERROR_INVALID_MESSAGE, -1, "" )
S ( LW_STATUS_OBJECT_NAME_NOT_FOUND, LWREG_ERROR_NO_SUCH_KEY_OR_VALUE, -1, "" )
S ( LW_STATUS_RESOURCE_IN_USE, LWREG_ERROR_KEY_IS_ACTIVE, -1, "" )
S ( LW_STATUS_DUPLICATE_NAME, LWREG_ERROR_DUPLICATE_KEYVALUENAME, -1, "" )
S ( LW_STATUS_KEY_HAS_CHILDREN, LWREG_ERROR_FAILED_DELETE_HAS_SUBKEY, -1, "" )
S ( LW_STATUS_NOT_SUPPORTED, LWREG_ERROR_UNKNOWN_DATA_TYPE, -1, "" )
S ( LW_STATUS_INVALID_BLOCK_LENGTH, LWREG_ERROR_BEYOUND_MAX_KEY_OR_VALUE_LENGTH, -1, "" )
S ( LW_STATUS_NO_MORE_MATCHES, LWREG_ERROR_NO_MORE_KEYS_OR_VALUES, -1, "" )
S ( LW_STATUS_OBJECT_NAME_INVALID, LWREG_ERROR_INVALID_NAME, -1, "" )
S ( LW_STATUS_ACCESS_DENIED, ERROR_ACCESS_DENIED, EACCES, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_INVALID_CACHE_PATH, -1, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_INVALID_PREFIX_PATH, -1, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_REGEX_COMPILE_FAILED, -1, "" )
S ( LW_STATUS_UNHANDLED_EXCEPTION, LWREG_ERROR_NOT_HANDLED, -1, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_UNEXPECTED_TOKEN, -1, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_INVALID_LOG_LEVEL, -1, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_UNKNOWN, -1, "" )
S ( LW_STATUS_MESSAGE_NOT_FOUND, LWREG_ERROR_INVALID_CONTEXT, -1, "" )
S ( LW_STATUS_OBJECTID_EXISTS, LWREG_ERROR_KEYNAME_EXIST, -1, "" )
S ( LW_STATUS_NO_SECURITY_ON_OBJECT, LWREG_ERROR_NO_SECURITY_ON_KEY, -1, "")
S ( LW_STATUS_INVALID_SECURITY_DESCR, LWREG_ERROR_INVALID_SECURITY_DESCR, -1, "")
S ( LW_STATUS_NO_TOKEN, LWREG_ERROR_INVALID_ACCESS_TOKEN, -1, "")
#undef S
#endif


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/



#endif /* LWREGERRORTABLE_H_ */
