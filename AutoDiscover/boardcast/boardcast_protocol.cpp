
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cb_sysinfo.h"
#include "boardcast_protocol.h"



static system_info_t g_sysinfo = { 0 };


void sys_environment_init()
{
	char sysver[512];
	char cptname[256];
	int len = 256;
	int bits = -1;

	int ret = cb_sysinfo_version(sysver, NULL);
	if (ret == 0)
	{
		strncpy(g_sysinfo.sysver, sysver, sizeof(g_sysinfo.sysver) - 1);
	}

	ret = cb_sysinfo_computer_name(cptname, &len);
	if (ret == 0)
	{
		strncpy(g_sysinfo.cptname, cptname, sizeof(g_sysinfo.cptname) - 1);
	}

	cb_sysinfo_bits(&bits);
	g_sysinfo.bits = (uint8_t)bits;
	return;
}

void make_shutdown_pkg(boardcast_package_t* pkg)
{
	pkg->version = BOARDCAST_PROTOCAL_VERSION;
	pkg->magic = BOARDCAST_MAGIC_NUM;
	memcpy(&pkg->sys_info, &g_sysinfo, sizeof(system_info_t));

	pkg->msg_type = BOARDCAST_MSG_SHUTDOWN;
	return;
}
void make_discover_pkg(boardcast_package_t* pkg)
{
	pkg->version = BOARDCAST_PROTOCAL_VERSION;
	pkg->magic = BOARDCAST_MAGIC_NUM;
	memcpy(&pkg->sys_info, &g_sysinfo, sizeof(system_info_t));

	pkg->msg_type = BOARDCAST_MSG_STARTUP;
	return;
}
