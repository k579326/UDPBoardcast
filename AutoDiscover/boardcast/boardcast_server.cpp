
#include <stdbool.h>
#include "boardcast_server.h"

#include "boardcast_protocol.h"
#include "boardcast_common.h"

#include "select_network.h"
#include "boardcast_define.h"
// #include "bridge/boardcast_cache.h"
#include "threadhelper.h"

static socket_env_t g_svr_bc;			// 用于服务器广播
static socket_env_t g_svrbc_listen;		// 用于接收客户端广播

static sleeper g_slp;

static SOCKET g_svr_feedback_sockfd = -1;	// 定向反馈socket
static SOCKET g_svr_shutdown_sockfd = -1;   // 关闭服务端的广播


DEPRECATED static void _boardcast_shutdown_msg()
{
    boardcast_package_t pkg;

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = PhyBoardcastAddr();
    //server_addr.sin_addr.s_addr = INADDR_BROADCAST;
    server_addr.sin_port = htons(CLIENT_BOARDCAST_PORT);

    make_shutdown_pkg(&pkg);
    sendto(g_svr_shutdown_sockfd, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));

    return;
}



static void _oriented_feedback(const char* clientip)
{
	boardcast_package_t pkg;
	sockaddr_in clientAddr;
	int ret = 0;
	// TODO: log client ip

	if (!checksocket(g_svr_feedback_sockfd))
	{
		return; // TODO:
	}

    uv_ip4_addr(clientip, CLIENT_BOARDCAST_PORT, &clientAddr);
	//clientAddr.sin_family = AF_INET;
	//clientAddr.sin_port = htons(CLIENT_BOARDCAST_PORT);
	//inet_pton(AF_INET, clientip, &clientAddr.sin_addr);

	make_startup_pkg(&pkg);
	ret = sendto(g_svr_feedback_sockfd, (const char*)&pkg, sizeof(boardcast_package_t),
				 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
	if (ret != sizeof(boardcast_package_t))
	{
        // TODO:
		// printf("[Server Feedback] error, send %d bytes\n", ret);
	}

	return;
}

static void _handle_boardcast_msg(int msg_type, const peer_info_t& peer)
{
    if (msg_type == BOARDCAST_MSG_STARTUP)
    {
        _oriented_feedback(peer.ip.c_str());
        // SafeCltList::getInstance()->add(peer.ip);
    }
    else if (msg_type == BOARDCAST_MSG_SHUTDOWN)
    {
        // SafeCltList::getInstance()->del(peer.ip);
    }
    else if (msg_type == BOARDCAST_MSG_KEEPALIVE)
    {
        // SafeCltList::getInstance()->add(peer.ip);
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
		ret = recvfrom(g_svrbc_listen.sockfd, (char*)&pkg, buflen, 0, (sockaddr*)&peerAddr, &addrlen);

		if (ret != buflen)
		{
			// printf("[Server BC Listen] error, send %d bytes!\n", ret);
		}
		else
		{
            peer_info_t peer;
            peer.ip = NetIpToString(peerAddr.sin_addr);
            peer.port = pkg.port;

            // 过滤无关广播
            if (pkg.magic == BOARDCAST_MAGIC_NUM)
            {
                _handle_boardcast_msg(pkg.msg_type, peer);
            }
		}

        // 刷新keepalive时间
        // SafeCltList::getInstance()->RefreshAliveTime();
		ssn_sleep(SVR_BC_LISTEN_TIMESPACE);
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
    //server_addr.sin_addr.s_addr = INADDR_BROADCAST;
    server_addr.sin_port = htons(CLIENT_BOARDCAST_PORT);

	while (1)
	{
		if (uv_sem_trywait(&g_svr_bc.sem_exit) == 0)
		{
			break;
		}
        make_startup_pkg(&pkg);

		ret = sendto(g_svr_bc.sockfd, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
		if (ret != sizeof(boardcast_package_t))
		{
			// TODO:
		}
        g_slp.sleep(SVR_BOARDCAST_TIMESPACE);
	}

	return;
}


static int _svr_boardcast_start()
{
    uv_sem_init(&g_svr_bc.sem_exit, 1);

    g_svr_bc.sockfd = create_boardcast_socket();
    if (g_svr_bc.sockfd == -1)
    {
        uv_sem_destroy(&g_svr_bc.sem_exit);
        return -1;
    }

    // 开启广播线程
    uv_sem_wait(&g_svr_bc.sem_exit);
    uv_thread_create(&g_svr_bc.thread, _boardcast_svr_msg, NULL);

    return 0;
}

static int _svr_boardcast_stop()
{
    uv_sem_post(&g_svr_bc.sem_exit);

    // // 唤醒广播线程
    // uv_mutex_lock(&g_svr_bc.mutex);
    // g_svr_bc.pause = false;
    // uv_mutex_unlock(&g_svr_bc.mutex);
    // uv_cond_signal(&g_svr_bc.cond);
    g_slp.wakeup();
    uv_thread_join(&g_svr_bc.thread); // 不等待子线程，可能发生崩溃

    uv_sem_destroy(&g_svr_bc.sem_exit);
    cleansocket(&g_svr_bc.sockfd);
    return 0;
}


static int _svr_listen_stop()
{
    uv_sem_post(&g_svrbc_listen.sem_exit);
    uv_thread_join(&g_svrbc_listen.thread);

    cleansocket(&g_svrbc_listen.sockfd);
    cleansocket(&g_svr_feedback_sockfd);
    uv_sem_destroy(&g_svrbc_listen.sem_exit);
    // cleansocket(&g_svr_shutdown_sockfd);

    // uv_mutex_lock(&g_svrbc_listen.mutex);
    // g_svrbc_listen.pause = true;
    // uv_mutex_unlock(&g_svrbc_listen.mutex);

    return 0;
}


static int _svr_listen_start()
{
    int ret = 0;

    g_svrbc_listen.sockfd = create_listen_udp_socket(SERVER_BOARDCAST_PORT);
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

    // uv_mutex_lock(&g_svrbc_listen.mutex);
    // g_svrbc_listen.pause = false;
    // uv_mutex_unlock(&g_svrbc_listen.mutex);

    uv_sem_init(&g_svrbc_listen.sem_exit, 1);
    uv_sem_wait(&g_svrbc_listen.sem_exit);
    uv_thread_create(&g_svrbc_listen.thread, _svrbc_listen_thread, NULL);
    //uv_cond_signal(&g_svrbc_listen.cond);

    return 0;
}

int svr_model_start()
{
    int err = 0;
    int lErr, bErr;

    lErr = _svr_listen_start();
	bErr = _svr_boardcast_start();

    if (lErr != 0 || bErr != 0){ 
        lErr == 0 ? _svr_listen_stop() : NULL;
        bErr == 0 ? _svr_boardcast_stop() : NULL;
        err = -1;
        // TODO: LOG ERROR
    }
    else
    {
        err = 0;
    }
	return err;
}

int svr_model_stop()
{
	_svr_boardcast_stop();
	_svr_listen_stop();

	return 0;
}


