
#include <stdbool.h>
#include "uv.h"
#include "boardcast-server.h"
#include "cb_thread.h"
#include "boardcast_protocol.h"
#include "boardcast_common.h"
#include "boardcast_define.h"


typedef struct
{
	bool inited;
	bool pause;
	SOCKET sockfd;

	uv_mutex_t mutex;
	uv_cond_t cond;
	uv_sem_t sem_exit;
	uv_thread_t thread;
}svr_boardcast_t;

static svr_boardcast_t g_svr_bc;

static int _create_svr_boardcast_socket()
{
	int ret = 0;

	g_svr_bc.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!bc_checksocket(g_svr_bc.sockfd))
	{
		return -1; // TODO:
	}

	int optval = 1;
	ret = setsockopt(g_svr_bc.sockfd, SOL_SOCKET, SO_BROADCAST, (char*)& optval, sizeof(int));
	if (ret < 0)
	{
		ret = -1;
		goto exit; // TODO:
	}

	//ret = _setnonblock(sockfd);
	//if (ret < 0)
	//{
	//	ret = -1;
	//	goto exit; // TODO:
	//}

exit:
	if (ret != 0)
		bc_cleansocket(&g_svr_bc.sockfd);
	return ret;
}


static void _boardcast_svr_msg(void* msg)
{
	int ret = 0;

	boardcast_package_t pkg;

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	server_addr.sin_port = htons(CLIENT_PORT);

	make_discover_pkg(&pkg);

	while (1)
	{
		if (uv_sem_trywait(&g_svr_bc.sem_exit) == 0)
		{
			// TODO:
			break;
		}

		uv_mutex_lock(&g_svr_bc.mutex);
		if (g_svr_bc.pause)
		{
			uv_cond_wait(&g_svr_bc.cond, &g_svr_bc.mutex);
		}
		uv_mutex_unlock(&g_svr_bc.mutex);
		ret = sendto(g_svr_bc.sockfd, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
		if (ret != sizeof(boardcast_package_t))
		{
			// TODO:
			printf("[Server Boardcast] error, send %d bytes!\n", ret);
		}

		CB_THREAD_SLEEP_MS(SVR_BOARDCAST_TIMESPACE);
	}

	// 退出前广播一次
	make_shutdown_pkg(&pkg);
	sendto(g_svr_bc.sockfd, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));

	return;
}



int svr_boardcast_init()
{
	int ret = 0;
	g_svr_bc.inited = false;

	ret = _create_svr_boardcast_socket();
	if (ret != 0)
	{
		return ret;
	}

	uv_mutex_init(&g_svr_bc.mutex);
	uv_cond_init(&g_svr_bc.cond);
	uv_sem_init(&g_svr_bc.sem_exit, 1);
	g_svr_bc.pause = true;

	uv_sem_wait(&g_svr_bc.sem_exit);
	uv_thread_create(&g_svr_bc.thread, _boardcast_svr_msg, NULL);

	g_svr_bc.inited = true;
	return 0;
}

int svr_boardcast_uninit()
{
	uv_mutex_lock(&g_svr_bc.mutex);
	g_svr_bc.pause = false;
	g_svr_bc.inited = false;
	uv_mutex_unlock(&g_svr_bc.mutex);

	uv_sem_post(&g_svr_bc.sem_exit);
	uv_cond_signal(&g_svr_bc.cond);

	uv_thread_join(&g_svr_bc.thread);

	uv_mutex_destroy(&g_svr_bc.mutex);
	uv_sem_destroy(&g_svr_bc.sem_exit);
	uv_cond_destroy(&g_svr_bc.cond);

	bc_cleansocket(&g_svr_bc.sockfd);

	return 0;
}

int svr_boardcast_isInited()
{
	return g_svr_bc.pause;
}

int svr_boardcast_start()
{
	uv_mutex_lock(&g_svr_bc.mutex);
	g_svr_bc.pause = false;
	uv_mutex_unlock(&g_svr_bc.mutex);

	uv_cond_signal(&g_svr_bc.cond);
	return 0;
}

int svr_boardcast_stop()
{
	uv_mutex_lock(&g_svr_bc.mutex);
	g_svr_bc.pause = true;
	uv_mutex_unlock(&g_svr_bc.mutex);

	return 0;
}





