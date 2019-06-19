
#include <stdlib.h>
#include <stdio.h>
#include "discover.h"
#include "boardcast/boardcast-mgr.h"

static int g_runType = 0;

int nd_set_running_type(int runType)
{
	return 0;
}

int nd_test_oriented_server()
{
	return 0;
}

int nd_add_oriented_server()
{
	return 0;
}


int nd_boardcast_init()
{
	return auto_sch_init();
}


int nd_boardcast_startup(int runType)
{
	int ret = 0;
	if (runType == CLT_RUN_TYPE)
	{
		auto_sch_stop_server();
		ret = auto_sch_runas_client();
	}
	else if (runType == SVR_RUN_TYPE)
	{
		auto_sch_stop_client();
		ret = auto_sch_runas_server();
	}
	else
	{
		int retc, rets;
		retc = auto_sch_runas_client();
		rets = auto_sch_runas_server();
		
		if (retc != 0 || rets != 0)
		{// 一起失败
			retc == 0 ? auto_sch_stop_client() : NULL;
			rets == 0 ? auto_sch_stop_server() : NULL;
		}

		// TODO: 处理错误
		// ret = ?
	}

	return ret;
}

int nd_boardcast_shutdown(int runType)
{
	if (runType == CLT_RUN_TYPE)
	{
		auto_sch_stop_client();
	}
	else if (runType == SVR_RUN_TYPE)
	{
		auto_sch_stop_server();
	}
	else
	{
		auto_sch_stop_client();
		auto_sch_stop_server();
	}
	return 0;
}

int nd_boardcast_uninit()
{
	auto_sch_uninit();
	return 0;
}


