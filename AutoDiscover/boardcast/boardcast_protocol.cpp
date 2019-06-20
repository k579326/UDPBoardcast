
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boardcast_protocol.h"
#include "boardcast_common.h"


void make_shutdown_pkg(boardcast_package_t* pkg)
{
	pkg->version = BOARDCAST_PROTOCAL_VERSION;
	pkg->magic = BOARDCAST_MAGIC_NUM;

	memcpy(&pkg->sys_info, systemInfo(), sizeof(system_info_t));

	pkg->msg_type = BOARDCAST_MSG_SHUTDOWN;
	return;
}
void make_active_pkg(boardcast_package_t* pkg)
{
	pkg->version = BOARDCAST_PROTOCAL_VERSION;
	pkg->magic = BOARDCAST_MAGIC_NUM;
	memcpy(&pkg->sys_info, systemInfo(), sizeof(system_info_t));

	pkg->msg_type = BOARDCAST_MSG_STARTUP;
	return;
}
