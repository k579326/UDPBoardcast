#include <list>
#include <stdint.h>


#include "cb_thread.h"
#include "uv.h"

#include "boardcast_client.h"
#include "boardcast_protocol.h"
#include "boardcast_common.h"
#include "boardcast_define.h"
#include "boardcast_address/select_network.h"

static socket_env_t g_cltbc_listen;		// 用于接收服务端广播
static SOCKET g_clt_stutdown_socket = -1;		// 用于客户端停止是发送消息
static SOCKET g_clt_startup_socket = -1;

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

		if (uv_sem_trywait(&g_cltbc_listen.sem_exit) == 0)
		{// 增加一处退出信号捕获，降低程序退出时等待几率
			break;
		}
		CB_THREAD_SLEEP_MS(200);
	}
	
	// TODO: exit log
	return;
}



static int _client_startup_boardcast()
{
	int ret = 0;
	boardcast_package_t pkg;
	sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = PhyBoardcastAddr();
	server_addr.sin_port = htons(SERVER_PORT);

	make_active_pkg(&pkg);

	ret = sendto(g_clt_startup_socket, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
	if (ret != sizeof(boardcast_package_t))
	{
		printf("[Client Boardcast] error, send %d bytes!\n", ret);
		return -1;
	}

	return ret;
}


int clt_model_init()
{
	int ret = 0;
	g_cltbc_listen.sockfd = create_listen_udp_socket(CLIENT_PORT);
	if (g_cltbc_listen.sockfd == -1)
	{
		ret = -1;  // TODO:
		goto exit;
	}

	g_clt_startup_socket = create_boardcast_socket();
	if (g_clt_startup_socket == -1)
	{
		ret = -1;  // TODO:
		goto exit;
	}

	g_clt_stutdown_socket = create_udp_socket();
	if (g_clt_stutdown_socket == -1)
	{
		ret = -1;  // TODO:
		goto exit;
	}

	g_cltbc_listen.pause = true;
	uv_mutex_init(&g_cltbc_listen.mutex);
	uv_cond_init(&g_cltbc_listen.cond);
	uv_sem_init(&g_cltbc_listen.sem_exit, 1);
	uv_sem_wait(&g_cltbc_listen.sem_exit);

	uv_thread_create(&g_cltbc_listen.thread, _cltbc_listen_thread, NULL);

exit:

	if (ret != 0)
	{
		cleansocket(&g_cltbc_listen.sockfd);
		cleansocket(&g_clt_startup_socket);
		cleansocket(&g_clt_stutdown_socket);
	}

	return ret;
}


int clt_model_start()
{
	int ret = 0;
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_cltbc_listen.pause = false;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	uv_cond_signal(&g_cltbc_listen.cond);

	ret = _client_startup_boardcast();
	// TODO: 不强制要求成功，如果失败，输出日志

	return 0;
}
int clt_model_stop()
{
	int ret;
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_cltbc_listen.pause = true;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	//ret = _client_do_boardcast(true);
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

	cleansocket(&g_cltbc_listen.sockfd);

	return 0;
}








