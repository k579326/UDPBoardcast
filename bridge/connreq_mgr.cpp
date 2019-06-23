

#include <string.h>
#include <vector>
#include <list>
#include "connreq_mgr.h"
#include "ssnet_define.h"
#include "sysheader.h"


class HostChangeReq
{
public:
    HostChangeReq()
    {
        uv_mutex_init(&mutex_);
        uv_cond_init(&cond_);
        cb_ = NULL;

        uv_thread_create(&thread_, req_handle, this);
    }
    ~HostChangeReq(){ 
        uv_mutex_destroy(&mutex_);
        uv_cond_destroy(&cond_);
        cb_ = NULL;
    }

    void SetCallBack(HOST_CHANGE_CB cb){
        lock();
        cb_ = cb;
        unlock();
    }
    static void req_handle(void* thread_param);

    void addmsg(conn_req_t req);

    void clrmsg(){
        lock();
        reqs.clear();
        unlock();
    }

    void wait()
    {
        uv_cond_wait(&cond_, &mutex_);
    }
    void signal()
    {
        uv_cond_signal(&cond_);
    }

private:

    void lock(){ 
        uv_mutex_lock(&mutex_);
    }
    void unlock(){ 
        uv_mutex_unlock(&mutex_);
    }

    std::list<conn_req_t> reqs;
    uv_thread_t thread_;
    uv_mutex_t mutex_;
    uv_cond_t cond_;
    HOST_CHANGE_CB cb_;
};


void HostChangeReq::addmsg(conn_req_t req)
{
    lock();

    std::list<conn_req_t>::iterator it;
    for (it = reqs.begin(); it != reqs.end(); it++)
    {
        // 这里不检查端口，客户端变动的消息来的时候没有带端口。这样的话就不支持服务端开两个不同端口来通信
        if (it->req_type == req.req_type && it->info.ip == it->info.ip)
        {
            break;
        }
    }
    if (it == reqs.end())
        reqs.push_back(req);

    unlock();
}

void HostChangeReq::req_handle(void* thread_param)
{
    HostChangeReq* reqObj = (HostChangeReq*)thread_param;

    while (1)
    {
        reqObj->lock();

        conn_req_t cr;
        std::list<conn_req_t>::iterator it;

        if (reqObj->reqs.empty())
        {
            reqObj->wait();
        }

        it = reqObj->reqs.begin();
        if (it == reqObj->reqs.end())
        {
            continue;
        }
        cr = *it;

        reqObj->reqs.pop_front();
        if (reqObj->cb_)
        {
            reqObj->cb_(&cr);
        }

        reqObj->unlock();
    }
}


static HostChangeReq g_clientReqHandler;            // 处理发现远程客户端的消息
static HostChangeReq g_serverReqHandler;            // 处理发现远程服务端的消息


// 远端客户端断开的回调
void RegisterClientDisconnectCallback(HOST_CHANGE_CB cb)
{
    g_clientReqHandler.SetCallBack(cb);
}

// 发现和丢失远端服务器的回调
void RegisterServerChangeCallback(HOST_CHANGE_CB cb)
{
    g_serverReqHandler.SetCallBack(cb);
}


void deliver_svrchange_msg(req_type_em type, const peer_info_t* peer)
{
    conn_req_t req;
    req.req_type = type;
    req.info.ip = peer->ip;
    req.info.port = peer->port;

    g_serverReqHandler.addmsg(req);
    g_serverReqHandler.signal();
}
void deliver_cltchange_msg(req_type_em type, const peer_info_t* peer)
{
    conn_req_t req;
    req.req_type = type;
    req.info.ip = peer->ip;
    req.info.port = peer->port;

    g_clientReqHandler.addmsg(req);
    g_clientReqHandler.signal();
}

void clean_svrchange_msg()
{
    g_serverReqHandler.clrmsg();
}
void clean_cltchange_msg()
{
    g_clientReqHandler.clrmsg();
}

