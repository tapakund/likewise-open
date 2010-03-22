/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        main.c
 *
 * Abstract:
 *
 *        Likewise System NET Utilities
 *
 *        Driver Dispatch
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Wei Fu (wfu@likewise.com)
 */

#include "includes.h"

#define NET_COMMAND_UNKNOWN 0
#define NET_COMMAND_HELP 1
#define NET_COMMAND_SHARE 2
#define NET_COMMAND_SESSION 3
#define NET_COMMAND_USER 4
#define NET_COMMAND_VIEW 5
#define NET_COMMAND_LOCALGROUP 6
#define NET_COMMAND_TIME 7

#define NET_COMMAND_HELP_NAME "HELP"
#define NET_COMMAND_SHARE_NAME "SHARE"
#define NET_COMMAND_SESSION_NAME "SESSION"
#define NET_COMMAND_USER_NAME "USER"
#define NET_COMMAND_VIEW_NAME "VIEW"
#define NET_COMMAND_LOCALGROUP_NAME "LOCALGROUP"
#define NET_COMMAND_TIME_NAME "TIME"

static
DWORD
NetMapSubCommand(
    char *param,
    PNET_SUB_COMMAND pdwSubCommand
    )
{
	NET_SUB_COMMAND dwSubCommand = NET_COMMAND_UNKNOWN;

	if (!param || !strcasecmp(param, NET_COMMAND_HELP_NAME))
	{
		dwSubCommand = NET_COMMAND_HELP;
	}
	else if (!strcasecmp(param, NET_COMMAND_SHARE_NAME))
	{
		dwSubCommand = NET_COMMAND_SHARE;
	}
	else if (!strcasecmp(param, NET_COMMAND_SESSION_NAME))
	{
		dwSubCommand = NET_COMMAND_SESSION;
	}
	else if (!strcasecmp(param, NET_COMMAND_USER_NAME))
	{
		dwSubCommand = NET_COMMAND_USER;
	}
	else if (!strcasecmp(param, NET_COMMAND_VIEW_NAME))
	{
		dwSubCommand = NET_COMMAND_VIEW;
	}
	else if (!strcasecmp(param, NET_COMMAND_LOCALGROUP_NAME))
	{
		dwSubCommand = NET_COMMAND_LOCALGROUP;
	}
	else if (!strcasecmp(param, NET_COMMAND_TIME_NAME))
	{
		dwSubCommand = NET_COMMAND_TIME;
	}

	*pdwSubCommand = dwSubCommand;

	return 0;
}


int 
main(
    int argc,
    char ** argv
    )
{
	DWORD dwError = 0;
	NET_SUB_COMMAND dwSubCommand = NET_COMMAND_UNKNOWN;

    dwError = NetMapSubCommand(
                    argv[1],
                    &dwSubCommand
                    );
    BAIL_ON_LWUTIL_ERROR(dwError);

    switch (dwSubCommand)
    {
        case NET_COMMAND_HELP:
        	printf("net help\n");
        	//dwError = Share(argc, argv);
        	break;

        case NET_COMMAND_SHARE:
            dwError = NetShare(argc, argv);
            BAIL_ON_LWUTIL_ERROR(dwError);

            break;

        case NET_COMMAND_SESSION:
        	printf("net session\n");
        	break;

        case NET_COMMAND_USER:
        	printf("net user\n");
        	break;

        case NET_COMMAND_VIEW:
        	printf("net view\n");
        	break;

        case NET_COMMAND_LOCALGROUP:
        	printf("net localgroup\n");
        	break;

        case NET_COMMAND_TIME:
        	printf("net time\n");
        	break;

        case NET_COMMAND_UNKNOWN:
        default:
        	printf("Wrong command\n");
        	break;
    }

 error:

     return dwError;
}
