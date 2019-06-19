#include <list>
#include "cb_thread.h"
#include "uv.h"

#include "boardcast-mgr.h"
#include "boardcast-server.h"

#include "boardcast_protocol.h"
#include "boardcast_common.h"
#include "boardcast_define.h"


typedef struct
{
	bool pause;
	SOCKET sockfd;
	uv_thread_t thread;
	uv_mutex_t mutex;
	uv_cond_t cond;
	uv_sem_t sem_exit;
}boardcast_listen_t;

static boardcast_listen_t g_cltbc_listen;		// 用于接收服务端广播
static boardcast_listen_t g_svrbc_listen;		// 用于接收客户端广播

static SOCKET g_svr_feedback_sockfd = -1;



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

static int _create_server_listen_res()
{
	int ret = 0;
	sockaddr_in server_addr;

	g_svrbc_listen.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!bc_checksocket(g_svrbc_listen.sockfd))
	{
		return -1; // TODO:
	}

	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	int optval = 1;
	ret = setsockopt(g_svrbc_listen.sockfd, SOL_SOCKET, SO_BROADCAST, (char*)& optval, sizeof(int));
	if (ret < 0)
	{
		ret = -1;
		goto exit; // TODO:
	}

	ret = bc_setnonblock(g_svrbc_listen.sockfd);
	if (ret < 0)
	{
		ret = -1;
		goto exit; // TODO:
	}

	ret = bind(g_svrbc_listen.sockfd, (sockaddr*) &server_addr, sizeof(server_addr));
	if (ret < 0)
	{
		ret = -1;
		goto exit; // TODO:
	}
exit:
	if (ret == -1)
	{
		bc_cleansocket(&g_svrbc_listen.sockfd);
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
			printf("[Client BC Listen] error, send %d bytes!\n", ret);
		}
		else
		{
			printf("[boardcast from server]: %s\n", (char*)pkg.sys_info.cptname);
		}

		CB_THREAD_SLEEP_MS(200);
	}
	
	return;
}


static void _oriented_feedback(char* clientip)
{
	boardcast_package_t pkg;
	sockaddr_in clientAddr;

	// TODO: log client ip

	if (!bc_checksocket(g_svr_feedback_sockfd))
	{
		return; // TODO:
	}

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = CLIENT_PORT;
	inet_pton(AF_INET, clientip, &clientAddr.sin_addr);
	
	make_discover_pkg(&pkg);
	sendto(g_svr_feedback_sockfd, (const char*)&pkg, sizeof(boardcast_package_t), 
		   0, (sockaddr*)&clientAddr, sizeof(clientAddr));

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
			// TODO:
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
			printf("[Server BC Listen] error, send %d bytes!\n", ret);
		}
		else
		{
			printf("[boardcast from client]: %s\n", (char*)pkg.sys_info.cptname);
			_oriented_feedback(inet_ntoa(peerAddr.sin_addr));
		}

		CB_THREAD_SLEEP_MS(200);
	}

	return;
}


static int _client_do_boardcast(bool isStop)
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

	isStop ? make_shutdown_pkg(&pkg) : make_discover_pkg(&pkg);

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

static int _create_svr_feedback_res()
{
	int ret = 0;

	g_svr_feedback_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (!bc_checksocket(g_svr_feedback_sockfd))
	{
		return -1; // TODO:
	}

	return 0;
}


int auto_sch_init()
{
	int ret = 0;
//#ifdef _WIN32
//	WSADATA wd;
//	ret = WSAStartup(MAKEWORD(2, 2), &wd);
//	if (ret != 0)
//	{
//		return ret; // TODO:
//	}
//#endif

	sys_environment_init();

	ret = _create_server_listen_res();
	if (ret != 0)
	{
		return ret;
	}
	ret = _create_client_listen_res();
	if (ret != 0)
	{
		bc_cleansocket(&g_cltbc_listen.sockfd);
		return ret;
	}

	ret = svr_boardcast_init();
	if (ret != 0)
	{
		// TODO: 服务广播初始化失败
	}

	g_cltbc_listen.pause = true;
	g_svrbc_listen.pause = true;

	uv_mutex_init(&g_cltbc_listen.mutex);
	uv_mutex_init(&g_svrbc_listen.mutex);
	uv_cond_init(&g_cltbc_listen.cond);
	uv_cond_init(&g_svrbc_listen.cond);

	uv_sem_init(&g_cltbc_listen.sem_exit, 1);
	uv_sem_init(&g_svrbc_listen.sem_exit, 1);

	uv_sem_wait(&g_cltbc_listen.sem_exit);
	uv_sem_wait(&g_svrbc_listen.sem_exit);

	// 创建服务器定向反馈socket
	_create_svr_feedback_res();

	uv_thread_create(&g_cltbc_listen.thread, _cltbc_listen_thread, NULL);
	uv_thread_create(&g_svrbc_listen.thread, _svrbc_listen_thread, NULL);

	return ret;
}


int auto_sch_uninit()
{
//#ifdef _WIN32
//	WSACleanup();
//#endif
	
	if (svr_boardcast_isInited())
	{
		svr_boardcast_uninit();
	}

	uv_sem_post(&g_cltbc_listen.sem_exit);
	uv_sem_post(&g_svrbc_listen.sem_exit);

	// 唤醒挂起的线程
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_cltbc_listen.pause = false;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	// 唤醒挂起的线程
	uv_mutex_lock(&g_svrbc_listen.mutex);
	g_svrbc_listen.pause = false;
	uv_mutex_unlock(&g_svrbc_listen.mutex);


	uv_cond_signal(&g_cltbc_listen.cond);
	uv_cond_signal(&g_svrbc_listen.cond);

	uv_thread_join(&g_cltbc_listen.thread);
	uv_thread_join(&g_svrbc_listen.thread);

	uv_cond_destroy(&g_cltbc_listen.cond);
	uv_cond_destroy(&g_svrbc_listen.cond);
	uv_mutex_destroy(&g_cltbc_listen.mutex);
	uv_mutex_destroy(&g_svrbc_listen.mutex);

	uv_sem_destroy(&g_cltbc_listen.sem_exit);
	uv_sem_destroy(&g_svrbc_listen.sem_exit);

	bc_cleansocket(&g_cltbc_listen.sockfd);
	bc_cleansocket(&g_svrbc_listen.sockfd);
	bc_cleansocket(&g_svr_feedback_sockfd);

	return 0;
}

int auto_sch_runas_client()
{
	int ret = 0;
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_cltbc_listen.pause = false;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	uv_cond_signal(&g_cltbc_listen.cond);

	ret = _client_do_boardcast(false);
	// TODO: 不强制要求成功，如果失败，输出日志


	return 0;
}
int auto_sch_runas_server()
{
	int ret = 0;
	uv_mutex_lock(&g_svrbc_listen.mutex);
	g_svrbc_listen.pause = false;
	uv_mutex_unlock(&g_svrbc_listen.mutex);
	
	uv_cond_signal(&g_svrbc_listen.cond);

	if (svr_boardcast_isInited())
	{
		svr_boardcast_start();
	}

	return 0;
}


int auto_sch_stop_client()
{
	int ret;
	uv_mutex_lock(&g_cltbc_listen.mutex);
	g_svrbc_listen.pause = true;
	uv_mutex_unlock(&g_cltbc_listen.mutex);

	ret = _client_do_boardcast(true);
	// TODO: 不强制要求成功，如果失败，输出日志

	return 0;
}

int auto_sch_stop_server()
{
	uv_mutex_lock(&g_svrbc_listen.mutex);
	g_svrbc_listen.pause = true;
	uv_mutex_unlock(&g_svrbc_listen.mutex);

	if (svr_boardcast_isInited())
	{
		svr_boardcast_stop();
	}

	return 0;
}







