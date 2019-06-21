
#include <stdbool.h>
#include "boardcast_server.h"

#include "cb_thread.h"
#include "boardcast_protocol.h"
#include "boardcast_common.h"

#include "uv.h"

#include "boardcast_address/select_network.h"
#include "boardcast_define.h"



static socket_env_t g_svr_bc;			// 用于服务器广播
static socket_env_t g_svrbc_listen;		// 用于接收客户端广播

static SOCKET g_svr_feedback_sockfd = -1;	// 定向反馈socket


static void _oriented_feedback(char* clientip)
{
	boardcast_package_t pkg;
	sockaddr_in clientAddr;
	int ret = 0;
	// TODO: log client ip

	if (!checksocket(g_svr_feedback_sockfd))
	{
		return; // TODO:
	}

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(CLIENT_PORT);
	inet_pton(AF_INET, clientip, &clientAddr.sin_addr);

	make_active_pkg(&pkg);
	ret = sendto(g_svr_feedback_sockfd, (const char*)&pkg, sizeof(boardcast_package_t),
				 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
	if (ret != sizeof(boardcast_package_t))
	{
		printf("[Server Feedback] error, send %d bytes\n", ret);
	}

	return;
}

static void _svrbc_listen_thread(void* param)
{
	int ret = 0;
	boardcast_package_t pkg;
	uint16_t buflen;
	sockaddr_in peerAddr;
	socklen_t addrlen;

	while (1)
	{
		if (0 == uv_sem_trywait(&g_svrbc_listen.sem_exit))
		{
			break;
		}

		buflen = sizeof(boardcast_package_t);
		addrlen = sizeof(peerAddr);

		uv_mutex_lock(&g_svrbc_listen.mutex);
		if (g_svrbc_listen.pause)
		{
			uv_cond_wait(&g_svrbc_listen.cond, &g_svrbc_listen.mutex);
		}
		ret = recvfrom(g_svrbc_listen.sockfd, (char*)&pkg, buflen, 0, (sockaddr*)&peerAddr, &addrlen);
		uv_mutex_unlock(&g_svrbc_listen.mutex);

		if (ret != buflen)
		{
			// printf("[Server BC Listen] error, send %d bytes!\n", ret);
		}
		else
		{
			printf("[boardcast from client]: %s, msg_type: %d\n", (char*)pkg.sys_info.cptname, pkg.msg_type);
			_oriented_feedback(inet_ntoa(peerAddr.sin_addr));
		}

		if (uv_sem_trywait(&g_svrbc_listen.sem_exit) == 0)
		{// 增加一处退出信号捕获，降低程序退出时等待几率
			break;
		}
		CB_THREAD_SLEEP_MS(200);
	}

	// TODO: exit log

	return;
}

static void _boardcast_svr_msg(void* msg)
{
	int ret = 0;

	boardcast_package_t pkg;

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = PhyBoardcastAddr();
	server_addr.sin_port = htons(CLIENT_PORT);

	make_active_pkg(&pkg);

	while (1)
	{
		if (uv_sem_trywait(&g_svr_bc.sem_exit) == 0)
		{
			break;
		}

		uv_mutex_lock(&g_svr_bc.mutex);
		if (g_svr_bc.pause)
		{
			// 停止前广播一次服务模式关闭
			make_shutdown_pkg(&pkg);
			ret = sendto(g_svr_bc.sockfd, (char*)& pkg, sizeof(boardcast_package_t), 0, (sockaddr*)& server_addr, sizeof(server_addr));
			if (ret != sizeof(boardcast_package_t))
			{
				printf("[Server Boardcast] error, send %d bytes!\n", ret);
			}
			else
			{
				printf("[Server Boardcast] server %s close!\n", pkg.sys_info.cptname);
			}

			uv_cond_wait(&g_svr_bc.cond, &g_svr_bc.mutex);
		}
		uv_mutex_unlock(&g_svr_bc.mutex);
		ret = sendto(g_svr_bc.sockfd, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
		if (ret != sizeof(boardcast_package_t))
		{
			// TODO:
			printf("[Server Boardcast] error, send %d bytes!\n", ret);
		}

		if (uv_sem_trywait(&g_svr_bc.sem_exit) == 0)
		{// 增加一处退出信号捕获，降低程序退出时等待几率
			break;
		}
		CB_THREAD_SLEEP_MS(SVR_BOARDCAST_TIMESPACE);
	}

	// TODO: exit log

	return;
}

static int _svr_listen_init()
{
	g_svrbc_listen.sockfd = create_listen_udp_socket(SERVER_PORT);
	if (g_svrbc_listen.sockfd == -1)
	{
		return -1;
	}

	// 创建服务器定向反馈socket
	g_svr_feedback_sockfd = create_udp_socket();
	if (g_svr_feedback_sockfd == -1)
	{
		// TODO:
		cleansocket(&g_svrbc_listen.sockfd);
		return -1;
	}

	g_svrbc_listen.pause = true;
	uv_mutex_init(&g_svrbc_listen.mutex);
	uv_cond_init(&g_svrbc_listen.cond);
	uv_sem_init(&g_svrbc_listen.sem_exit, 1);
	uv_sem_wait(&g_svrbc_listen.sem_exit);

	uv_thread_create(&g_svrbc_listen.thread, _svrbc_listen_thread, NULL);

	return 0;
}

static int _svr_listen_uninit()
{
	uv_sem_post(&g_svrbc_listen.sem_exit);

	// 唤醒挂起的线程
	uv_mutex_lock(&g_svrbc_listen.mutex);
	g_svrbc_listen.pause = false;
	uv_mutex_unlock(&g_svrbc_listen.mutex);

	uv_cond_signal(&g_svrbc_listen.cond);
	uv_thread_join(&g_svrbc_listen.thread);
	uv_cond_destroy(&g_svrbc_listen.cond);
	uv_mutex_destroy(&g_svrbc_listen.mutex);
	uv_sem_destroy(&g_svrbc_listen.sem_exit);
	cleansocket(&g_svrbc_listen.sockfd);
	cleansocket(&g_svr_feedback_sockfd);
	
	return 0;
}



static int _svr_boardcast_init()
{
	g_svr_bc.sockfd = create_boardcast_socket();
	if (g_svr_bc.sockfd == -1)
	{
		return -1;
	}

	uv_mutex_init(&g_svr_bc.mutex);
	uv_cond_init(&g_svr_bc.cond);
	uv_sem_init(&g_svr_bc.sem_exit, 1);
	g_svr_bc.pause = true;

	uv_sem_wait(&g_svr_bc.sem_exit);
	uv_thread_create(&g_svr_bc.thread, _boardcast_svr_msg, NULL);

	return 0;
}

static int _svr_boardcast_uninit()
{
	uv_mutex_lock(&g_svr_bc.mutex);
	g_svr_bc.pause = false;
	uv_mutex_unlock(&g_svr_bc.mutex);

	uv_sem_post(&g_svr_bc.sem_exit);
	uv_cond_signal(&g_svr_bc.cond);

	uv_thread_join(&g_svr_bc.thread);

	uv_mutex_destroy(&g_svr_bc.mutex);
	uv_sem_destroy(&g_svr_bc.sem_exit);
	uv_cond_destroy(&g_svr_bc.cond);

	cleansocket(&g_svr_bc.sockfd);

	return 0;
}

int svr_model_init()
{
	int ret = 0;
	bool bErr = false, lErr = false;
	
	ret = _svr_listen_init();
	if (ret != 0)
	{
		lErr = true;
		goto exit;
	}

	ret = _svr_boardcast_init();
	if (ret != 0)
	{
		bErr = true;
		goto exit;
	}

exit:

	if (ret != 0)
	{
		bErr ? _svr_boardcast_uninit() : NULL;
		lErr ? _svr_listen_uninit() : NULL;
	}

	return ret;
}

int svr_model_uninit()
{
	_svr_boardcast_uninit();
	_svr_listen_uninit();
	return 0;
}

static int _svr_boardcast_start()
{
	uv_mutex_lock(&g_svr_bc.mutex);
	g_svr_bc.pause = false;
	uv_mutex_unlock(&g_svr_bc.mutex);

	uv_cond_signal(&g_svr_bc.cond);
	return 0;
}

static int _svr_boardcast_stop()
{
	uv_mutex_lock(&g_svr_bc.mutex);
	g_svr_bc.pause = true;
	uv_mutex_unlock(&g_svr_bc.mutex);

	return 0;
}


static int _svr_listen_stop()
{
	uv_mutex_lock(&g_svrbc_listen.mutex);
	g_svrbc_listen.pause = true;
	uv_mutex_unlock(&g_svrbc_listen.mutex);

	return 0;
}


static int _svr_listen_start()
{
	int ret = 0;
	uv_mutex_lock(&g_svrbc_listen.mutex);
	g_svrbc_listen.pause = false;
	uv_mutex_unlock(&g_svrbc_listen.mutex);

	uv_cond_signal(&g_svrbc_listen.cond);

	return 0;
}

int svr_model_start()
{
	_svr_listen_start();
	_svr_boardcast_start();

	return 0;
}

int svr_model_stop()
{
	_svr_boardcast_stop();
	_svr_listen_stop();

	return 0;
}


