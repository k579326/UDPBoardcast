


#include "discover.h"
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif


int main()
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	
	int ret = nd_boardcast_init();
	if (ret != 0)
	{
		return ret;
	}

	nd_set_running_type(MIX_RUN_TYPE);
	nd_set_running_type(CLT_RUN_TYPE);
	nd_set_running_type(SVR_RUN_TYPE);
	nd_set_running_type(MIX_RUN_TYPE);

	getchar();

	nd_boardcast_uninit();

	return 0;
}