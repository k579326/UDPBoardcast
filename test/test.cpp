


#include "discover.h"
#include "bridge/connreq_mgr.h"

#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#endif


void svr_changed_callback(const conn_req_t* req)
{
    std::string typeString;
    if (req->req_type == REQ_Add)
    {
        typeString = "Add";
    }
    else
    {
        typeString = "Del";
    }

    printf("[Client Changed MSG] target %s, type: %s\n", req->info.ip.c_str(), typeString.c_str());
}

void clt_changed_callback(const conn_req_t* req)
{
    std::string typeString;
    if (req->req_type == REQ_Add)
    {
        typeString = "Add";
    }
    else
    {
        typeString = "Del";
    }

    printf("[Server Changed MSG] target %s, type: %s\n", req->info.ip.c_str(), typeString.c_str());
}



int main()
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    RegisterClientDisconnectCallback(clt_changed_callback);
    RegisterServerChangeCallback(svr_changed_callback);

	int ret = nd_boardcast_init();
	if (ret != 0)
	{
		return ret;
	}

	nd_set_running_type(MIX_RUN_TYPE);
	//nd_set_running_type(CLT_RUN_TYPE);
	//nd_set_running_type(SVR_RUN_TYPE);
	//nd_set_running_type(MIX_RUN_TYPE);

	getchar();

	nd_boardcast_uninit();

	return 0;
}