




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

	nd_boardcast_startup(1);


	//Sleep(10);
	//nd_boardcast_startup(1);
	//
	//Sleep(30);
	//nd_boardcast_startup(2);

	getchar();

	nd_boardcast_uninit();

	return 0;
}