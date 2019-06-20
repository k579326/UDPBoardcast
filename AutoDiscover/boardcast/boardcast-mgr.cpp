#include <list>
#include <stdint.h>


#include "cb_thread.h"
#include "uv.h"

#include "boardcast-mgr.h"
#include "boardcast_protocol.h"
#include "boardcast_common.h"
#include "boardcast_define.h"


static boardcast_socket_t g_cltbc_listen;		// 用于接收服务端广播

static int _create_client_listen_res()
{
	int ret = 0;
	sockaddr_in client_addr;

	g_cltbc_listen.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!bc_checksocket(g_cltbc_listen.sockfd))
	{
		return -1; // TODO:
	}

	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	client_addr.sin_port = htons(CLIENT_PORT);

	int optval = 1;
	ret = setsockopt(g_cltbc_listen.sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(int));
	if (ret < 0)
	{
		ret = -1;
		goto exit; // TODO:
	}

	ret = bc_setnonblock(g_cltbc_listen.sockfd);
	if (ret < 0)
	{
		ret = -1;
		goto exit; // TODO:
	}

	ret = bind(g_cltbc_listen.sockfd, (sockaddr*)&client_addr, sizeof(client_addr));
	if (ret != 0)
	{
		ret = -1;
		goto exit;  // TODO:
	}

exit:
	if (ret == -1)
	{
		bc_cleansocket(&g_cltbc_listen.sockfd);
	}

	return ret;
}


static void _cltbc_listen_thread(void* param)
{
	int ret = 0;
	boardcast_package_t pkg;
	uint16_t buflen;
	sockaddr_in peerAddr;
	socklen_t addrlen;

	while (1)
	{
		if (0 == uv_sem_trywait(&g_cltbc_listen.sem_exit))
		{
			// TODO:
			break;
		}

		buflen = sizeof(pkg);
		addrlen = sizeof(peerAddr);

		uv_mutex_lock(&g_cltbc_listen.mutex);
		if (g_cltbc_listen.pause)
		{
			uv_cond_wait(&g_cltbc_listen.cond, &g_cltbc_listen.mutex);
		}
		ret = recvfrom(g_cltbc_listen.sockfd, (char*)&pkg, buflen, 0, (sockaddr*)&peerAddr, &addrlen);
		uv_mutex_unlock(&g_cltbc_listen.mutex);

		if (ret != buflen)
		{
			// printf("[Client BC Listen] error, send %d bytes!\n", ret);
		}
		else
		{
			printf("[boardcast from server]: %s, msg_type: %d\n", (char*)pkg.sys_info.cptname, pkg.msg_type);
		}

		CB_THREAD_SLEEP_MS(200);
	}
	
	return;
}



static int _client_do_boardcast()
{
	int ret = 0;
	boardcast_package_t pkg;

	sockaddr_in server_addr;
	SOCKET sockfd;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!bc_checksocket(sockfd))
	{
		return -1; // TODO:
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	server_addr.sin_port = htons(SERVER_PORT);

	int optval = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(int));
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

	make_discover_pkg(&pkg);

	ret = sendto(sockfd, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
	if (ret != sizeof(boardcast_package_t))
	{
		printf("[Client Boardcast] error, send %d bytes!\n", ret);
		ret = -1;
		goto exit; // TODO:
	}

exit:
	bc_cleansocket(&sockfd);
	return ret;
}


int clt_model_init()
{
	int ret = 0;

	ret = _create_client_listen_res();
	if (ret != 0)
	{
		bc_cleansocket(&g_cltbc_listen.sockfd);
		return ret;
	}

	g_cltbc_listen.pause = true;
	uv_mutex_init(&g_cltbc_listen.mutex);
	uv_cond_init(&g_cltbc_listen.cond);
	uv_sem_init(&g_cltbc_listen.sem_exit, 1);
	uv_sem_wait(&g_cltbc_listen.sem_exit);

	uv_thread_create(&g_cltbc_listen.thread, _cltbc_listen_thread, NULL);

	return ret;
}
int clt_model_start()
{
	int ret = 0;
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_cltbc_listen.pause = false;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	uv_cond_signal(&g_cltbc_listen.cond);

	ret = _client_do_boardcast();
	// TODO: 不强制要求成功，如果失败，输出日志

	return 0;
}
int clt_model_stop()
{
	int ret;
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_cltbc_listen.pause = true;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	ret = _client_do_boardcast(true);
	// TODO: 不强制要求成功，如果失败，输出日志

	return 0;
}
int clt_model_uninit()
{
	uv_sem_post(&g_cltbc_listen.sem_exit);

	// 唤醒挂起的线程
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_cltbc_listen.pause = false;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	uv_cond_signal(&g_cltbc_listen.cond);
	uv_thread_join(&g_cltbc_listen.thread);

	uv_cond_destroy(&g_cltbc_listen.cond);
	uv_mutex_destroy(&g_cltbc_listen.mutex);
	uv_sem_destroy(&g_cltbc_listen.sem_exit);

	bc_cleansocket(&g_cltbc_listen.sockfd);

	return 0;
}








