

#include <string.h>
#include <vector>
#include <list>
#include "connreq_mgr.h"
#include "ssnet_define.h"
#include "sysheader.h"
#include "threadpool.h"
#include "Communication/comm_internal.h"
#include "ssnet_err.h"

class HostChangeReq
{
public:
    HostChangeReq()
    {
        uv_mutex_init(&mutex_);
        threadpool_init(&threadpool_, 4);       // 4条线程负责对广播发现的服务端进行连接

        cb_ = NULL;

    }
    ~HostChangeReq(){
        uv_mutex_destroy(&mutex_);
        threadpool_uninit(threadpool_);
        threadpool_ = NULL;
        cb_ = NULL;
    }

    void SetCallBack(DISCOVER_SVR_CB cb){
        cb_ = cb;
    }
    static void threadpool_work(void* thread_param);

    void addsvrconn(const char* ip, short port);
    void delsvrconn(const char* ip);
    void clrmsg(){
        lock();
        reqs.clear();
        unlock();
    }


private:

    void lock(){ 
        uv_mutex_lock(&mutex_);
    }
    void unlock(){ 
        uv_mutex_unlock(&mutex_);
    }

    std::list<peer_info_t> reqs;
    uv_mutex_t mutex_;
    DISCOVER_SVR_CB cb_;
    threadpool_t* threadpool_;
};

typedef struct
{
    HostChangeReq* ptr;
    peer_info_t peer;
}thread_param_t;


void HostChangeReq::threadpool_work(void* thread_param)
{
    int err = 0;
    thread_param_t* param = (thread_param_t*)thread_param;

    err = ssn_connect(param->peer.ip.c_str(), param->peer.port, CONN_TIMEOUT);
    if (0 != err)
    {
        // TODO: LOG
        printf("[Conn Msg] connect %s failed! errcode:%d, %s\n", param->peer.ip.c_str(), err, ssn_errmsg(err));
    }

    // 连接完成，删除连接请求
    param->ptr->delsvrconn(param->peer.ip.c_str());

    delete param;
}


void HostChangeReq::addsvrconn(const char* ip, short port)
{
    
    lock();

    std::list<peer_info_t>::iterator it;
    for (it = reqs.begin(); it != reqs.end(); it++)
    {
        if (it->ip == it->ip)
        {
            break;
        }
    }
    if (it == reqs.end())
    {
        peer_info_t peer;
        peer.ip = ip;
        peer.port = port;
        reqs.push_back(peer);

        thread_param_t* param = new thread_param_t;
        param->ptr = this;
        param->peer = peer;
        threadpool_push_work(threadpool_, param, threadpool_work, NULL);
    }
    unlock();

    return;
}

void HostChangeReq::delsvrconn(const char* ip)
{
    lock();

    std::list<peer_info_t>::iterator it;
    for (it = reqs.begin(); it != reqs.end(); )
    {
        if (it->ip == it->ip)
        {
           it = reqs.erase(it);
        }
        else
        {
            it++;
        }
    }
    unlock();
    return;
}




// static HostChangeReq g_clientReqHandler;            // 处理发现远程客户端的消息
static HostChangeReq g_serverReqHandler;            // 处理发现远程服务端的消息


// // 远端客户端断开的回调
// void RegisterClientDisconnectCallback(HOST_CHANGE_CB cb)
// {
//     g_clientReqHandler.SetCallBack(cb);
// }

// 发现和丢失远端服务器的回调
void RegisterConnectServerCallback(DISCOVER_SVR_CB cb)
{
    g_serverReqHandler.SetCallBack(cb);
}


void deliver_addsvr_msg(const char* ip, short port)
{
    if (!ip)
    {
        return;
    }
    g_serverReqHandler.addsvrconn(ip, port);
}
// void deliver_cltchange_msg(req_type_em type, const peer_info_t* peer)
// {
//     conn_req_t req;
//     req.req_type = type;
//     req.info.ip = peer->ip;
//     req.info.port = peer->port;
// 
//     g_clientReqHandler.addmsg(req);
//     g_clientReqHandler.signal();
// }

// void clean_svrchange_msg()
// {
//     g_serverReqHandler.clrmsg();
// }
// void clean_cltchange_msg()
// {
//     g_clientReqHandler.clrmsg();
// }

