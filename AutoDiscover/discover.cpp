
#include <stdlib.h>
#include <stdio.h>
#include "discover.h"
#include "boardcast/boardcast-mgr.h"

static int g_runType = NONE_RUN_TYPE;

int nd_set_running_type(int runType)
{
	int retc = 0, rets = 0;
	if (g_runType & CLT_RUN_TYPE)
	{
		if (~runType & CLT_RUN_TYPE)
		{
			auto_sch_stop_client();
		}
	}
	else
	{
		if (runType & CLT_RUN_TYPE)
		{
			retc = auto_sch_runas_client();
		}
	}

	if (g_runType & SVR_RUN_TYPE)
	{
		if (~runType & SVR_RUN_TYPE)
		{
			auto_sch_stop_server();
		}
	}
	else
	{
		if (runType & SVR_RUN_TYPE)
		{
			rets = auto_sch_runas_server();
		}
	}

	if (retc == 0 && rets == 0)
	{
		g_runType = runType;
	}
	else
	{
		//TODO:

		g_runType = (retc != 0 ? (runType & ~CLT_RUN_TYPE) : runType);
		g_runType = (rets != 0 ? (runType & ~SVR_RUN_TYPE) : runType);
	}
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


int nd_boardcast_uninit()
{
	g_runType = NONE_RUN_TYPE;
	auto_sch_uninit();
	return 0;
}


