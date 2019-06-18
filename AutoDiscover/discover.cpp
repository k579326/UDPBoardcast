

#include "discover.h"
#include "boardcast/boardcast-mgr.h"

static int g_runType = 0;

int nd_set_running_type(int runType)
{
	g_runType = runType;
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
		ret = auto_sch_runas_client();
		if (ret == 0)
		{
			ret = auto_sch_runas_server();
		}
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
	return 0;
}

int nd_boardcast_uninit()
{
	auto_sch_uninit();
	return 0;
}


