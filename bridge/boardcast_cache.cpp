

#include <stdio.h>
#include <algorithm>
#include "boardcast_cache.h"
#include "ssnet_define.h"
#include "connreq_mgr.h"

SafeSvrList* SafeSvrList::getInstance()
{
	static SafeSvrList instance_;
	return &instance_;
}


struct peer_compare
{
	bool operator() (const peer_info_t& tar)
	{
		return m_.ip == tar.ip;
	}

	peer_info_t m_;
};


bool SafeSvrList::add(const peer_info_t& peer)
{
	std::vector<peer_info_t>::const_iterator it;
	peer_compare CMP;
	lock();

	CMP.m_ = peer;
	it = std::find_if(hostList_.begin(), hostList_.end(), CMP);
	if (it == hostList_.end())
	{
		hostList_.push_back(peer);
        deliver_svrchange_msg(REQ_Add, &peer);
	}
	unlock();

	return true;
}

bool SafeSvrList::del(const peer_info_t& peer)
{
	std::vector<peer_info_t>::iterator it;
	peer_compare CMP;

	lock();

	CMP.m_ = peer;
	it = std::find_if(hostList_.begin(), hostList_.end(), CMP);
	if (it != hostList_.end())
	{
		hostList_.erase(it);
        deliver_svrchange_msg(REQ_Del, &peer);
	}
	unlock();

	return true;
}

std::vector<peer_info_t> SafeSvrList::clr()
{
	std::vector<peer_info_t> lst;

	lock();

	lst = hostList_;
    hostList_.clear();
    clean_svrchange_msg();

	unlock();

	return lst;
}



void SafeCltList::_keepalive(void* param)
{
    SafeCltList* scl = (SafeCltList*)param;
    while (1)
    {
        scl->lock();
        uv_cond_wait(&scl->cond_, &scl->mutex_);

        for (std::map<CLIENTIP, int>::iterator it = scl->hostList_.begin();
             it != scl->hostList_.end(); )
        {
            it->second += SVR_BC_LISTEN_TIMESPACE;
            if (it->second > SVR_KEEPALIVE_TIMEOUT)
            {// 超时
                peer_info_t peer;
                peer.ip = it->first;
                peer.port = 0;

                scl->hostList_.erase(it++);
                deliver_cltchange_msg(REQ_Del, &peer);
            }
            else
            {
                it++;
            }
        }
        scl->unlock();
    }

    return;
}

SafeCltList* SafeCltList::getInstance()
{
    static SafeCltList instance_;
    return &instance_;
}

bool SafeCltList::add(const CLIENTIP& ip)
{
    std::map<CLIENTIP, int>::iterator it;

    lock();

    it = hostList_.find(ip);
    if (it == hostList_.end())
    {
        hostList_[ip] = -SVR_BC_LISTEN_TIMESPACE; // 插入新的客户端，keepalive定为负间隔

        peer_info_t peer;
        peer.ip = ip;
        peer.port = 0;
        deliver_cltchange_msg(REQ_Add, &peer);
    }
    else
    {
        it->second = -SVR_BC_LISTEN_TIMESPACE; // 重置keepalive时间
    }
    unlock();

    return true;
}
bool SafeCltList::del(const CLIENTIP& ip)
{
    std::map<CLIENTIP, int>::iterator it;

    lock();

    it = hostList_.find(ip);
    if (it != hostList_.end())
    {
        hostList_.erase(it);

        peer_info_t peer;
        peer.ip = it->first;
        peer.port = 0;
        deliver_cltchange_msg(REQ_Del, &peer);
    }
    unlock();

    return true;
}
std::vector<CLIENTIP> SafeCltList::clr()
{
    std::vector<CLIENTIP> lst;

    lock();
    
    for (std::map<CLIENTIP, int>::iterator it = hostList_.begin();
         it != hostList_.end(); )
    {
        lst.push_back(it->first);
        hostList_.erase(it++);
    }

    clean_cltchange_msg();

    unlock();

    return lst;
}

void SafeCltList::RefreshAliveTime()
{
    uv_cond_signal(&cond_);
}
