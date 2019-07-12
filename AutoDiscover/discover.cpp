
#include <stdlib.h>
#include <stdio.h>
#include "discover.h"
#include "boardcast/boardcast_protocol.h"
#include "boardcast/boardcast_client.h"
#include "boardcast/boardcast_server.h"

static int g_runType = NONE_RUN_TYPE;

int ssn_set_boardcast_model(int runType)
{
	int retc = -1, rets = -1;

    if ((g_runType & CLT_RUN_TYPE) != (runType & CLT_RUN_TYPE))
    {
        if (runType & CLT_RUN_TYPE){ 
            clt_model_start();
        }
        else { 
            clt_model_stop();
        }
    }

    if ((g_runType & SVR_RUN_TYPE) != (runType & SVR_RUN_TYPE))
    {
        if (runType & SVR_RUN_TYPE){
            svr_model_start();
        }
        else {
            svr_model_stop();
        }
    }

    g_runType = runType;
	return 0;
}


