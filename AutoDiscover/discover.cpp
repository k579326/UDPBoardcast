
#include <stdlib.h>
#include <stdio.h>
#include "discover.h"
#include "boardcast/boardcast_protocol.h"
#include "boardcast/boardcast_client.h"
#include "boardcast/boardcast_server.h"

static int g_runType = NONE_RUN_TYPE;

int nd_set_running_type(int runType)
{
	int retc = 0, rets = 0;
	if (g_runType & CLT_RUN_TYPE)
	{
		if (~runType & CLT_RUN_TYPE)
		{
			clt_model_stop();
		}
	}
	else
	{
		if (runType & CLT_RUN_TYPE)
		{
			retc = clt_model_start();
		}
	}

	if (g_runType & SVR_RUN_TYPE)
	{
		if (~runType & SVR_RUN_TYPE)
		{
			svr_model_stop();
		}
	}
	else
	{
		if (runType & SVR_RUN_TYPE)
		{
			rets = svr_model_start();
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
	int rets = 0, retc = 0;
	bool cErr = false, sErr = false;

	retc = clt_model_init();
	if (retc != 0)
	{
		goto exit;
	}
	rets = svr_model_init();
	if (rets != 0)
	{
		goto exit;
	}

exit:
	if (retc == 0 && rets == 0)
	{
		return 0;
	}
	else
	{
		retc == 0 ? clt_model_uninit() : NULL;
		rets == 0 ? svr_model_uninit() : NULL;
		return -1;
	}

	return 0;
}


int nd_boardcast_uninit()
{
	g_runType = NONE_RUN_TYPE;
	clt_model_uninit();
	svr_model_uninit();
	return 0;
}


