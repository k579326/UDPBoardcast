#include <list>
#include <stdint.h>


#include "uv.h"

#include "boardcast_client.h"
#include "boardcast_protocol.h"
#include "boardcast_common.h"
#include "boardcast_define.h"
#include "select_network.h"
// #include "bridge/boardcast_cache.h"
#include "bridge/connreq_mgr.h"
#include "ssnet_define.h"

static socket_env_t g_cltbc_listen;		// 用于接收服务端广播
static SOCKET g_clt_stutdown_socket = -1;		// 用于客户端停止是发送消息
static SOCKET g_clt_startup_socket = -1;


// deprecated 
static void _oriented_feedback_alive(const std::string& ip)
{
    int ret = 0;
    boardcast_package_t pkg;
    sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = StringToNetIp(ip.c_str());
    server_addr.sin_port = htons(SERVER_BOARDCAST_PORT);

    make_keepalive_pkg(&pkg);

    ret = sendto(g_cltbc_listen.sockfd, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
    if (ret != sizeof(boardcast_package_t))
    {
        printf("[Client Alive] send keepalive msg failed!\n");
    }
    //else
    //{
    //    printf("[Client Alive] feedback to %s success!\n", ip.c_str());
    //}
    return;
}


static void _handle_boardcast_msg(int msg_type, const peer_info_t& peer)
{
	if (msg_type == BOARDCAST_MSG_STARTUP)
	{
        deliver_addsvr_msg(peer.ip.c_str(), peer.port);
		// SafeSvrList::getInstance()->add(peer);
        // _oriented_feedback_alive(peer.ip);
	}
	else if (msg_type == BOARDCAST_MSG_SHUTDOWN)
	{
        // SafeSvrList::getInstance()->del(peer);
	}
    return;
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
			break;
		}

		buflen = sizeof(pkg);
		addrlen = sizeof(peerAddr);
		ret = recvfrom(g_cltbc_listen.sockfd, (char*)&pkg, buflen, 0, (sockaddr*)&peerAddr, &addrlen);

		if (ret != buflen)
		{
			// printf("[Client BC Listen] error, send %d bytes!\n", GetLastError());
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

		ssn_sleep(CLT_BC_LISTEN_TIMESPACE);
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
	server_addr.sin_port = htons(SERVER_BOARDCAST_PORT);

    make_startup_pkg(&pkg);

	ret = sendto(g_clt_startup_socket, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
	if (ret != sizeof(boardcast_package_t))
	{
		printf("[Client Boardcast] error, send %d bytes!\n", ret);
		return -1;
	}

	return ret;
}

// static void _client_shutdown_oriented_notify()
// {
//     std::vector<peer_info_t> svrList = SafeSvrList::getInstance()->clr();
// 
//     for (std::vector<peer_info_t>::const_iterator it = svrList.begin();
//          it != svrList.end(); it++)
//     {
//         int ret = 0;
//         boardcast_package_t pkg;
//         sockaddr_in server_addr;
//         
//         server_addr.sin_family = AF_INET;
//         server_addr.sin_addr = StringToNetIp(it->ip.c_str());
//         server_addr.sin_port = htons(SERVER_PORT);
// 
//         make_shutdown_pkg(&pkg);
// 
//         ret = sendto(g_clt_stutdown_socket, (char*)&pkg, sizeof(boardcast_package_t), 0, (sockaddr*)&server_addr, sizeof(server_addr));
//         if (ret != sizeof(boardcast_package_t))
//         {
//             // TODO:
//         }
//     }
// }


int clt_model_start()
{
	int ret = 0;

    g_cltbc_listen.sockfd = create_listen_udp_socket(CLIENT_BOARDCAST_PORT);
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

    uv_sem_init(&g_cltbc_listen.sem_exit, 1);
    uv_sem_wait(&g_cltbc_listen.sem_exit);
    uv_thread_create(&g_cltbc_listen.thread, _cltbc_listen_thread, NULL);

	_client_startup_boardcast();

exit:
    if (ret != 0)
    {
        cleansocket(&g_cltbc_listen.sockfd);
        cleansocket(&g_clt_startup_socket);
        // cleansocket(&g_clt_stutdown_socket);
    }

	return ret;
}
int clt_model_stop()
{
	int ret;

    uv_sem_post(&g_cltbc_listen.sem_exit);
    uv_thread_join(&g_cltbc_listen.thread);

    cleansocket(&g_cltbc_listen.sockfd);
    cleansocket(&g_clt_startup_socket);
    uv_sem_destroy(&g_cltbc_listen.sem_exit);

	return 0;
}








