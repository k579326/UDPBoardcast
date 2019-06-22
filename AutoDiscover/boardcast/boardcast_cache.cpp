

#include <stdio.h>
#include <algorithm>
#include "boardcast_cache.h"

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
	bool flag = false;
	std::vector<peer_info_t>::const_iterator it;
	peer_compare CMP;
	lock();

	CMP.m_ = peer;
	it = std::find_if(hostList_.begin(), hostList_.end(), CMP);
	if (it == hostList_.end())
	{
		hostList_.push_back(peer);
		flag = true;
	}
	else
	{
		flag = false;
	}
	unlock();

	return flag;
}

bool SafeSvrList::del(const peer_info_t& peer)
{
	bool flag = false;
	std::vector<peer_info_t>::const_iterator it;
	peer_compare CMP;

	lock();

	CMP.m_ = peer;
	it = std::find_if(hostList_.begin(), hostList_.end(), CMP);
	if (it != hostList_.end())
	{
		hostList_.erase(it);
		flag = true;
	}
	else
	{
		flag = false;
	}
	unlock();

	return flag;
}

std::vector<peer_info_t> SafeSvrList::clr()
{
	std::vector<peer_info_t> lst;

	lock();
	lst = hostList_;
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
            it->second += SVR_BOARDCAST_TIMESPACE;
            if (it->second > SVR_KEEPALIVE_TIMEOUT)
            {// 超时
                it = scl->hostList_.erase(it);
                // TODO: 发送超时消息给队列
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
    bool flag = false;
    std::map<CLIENTIP, int>::iterator it;

    lock();

    it = hostList_.find(ip);
    if (it == hostList_.end())
    {
        hostList_[ip] = -SVR_BOARDCAST_TIMESPACE; // 插入新的客户端，keepalive定为负间隔
        flag = true;
    }
    else
    {
        it->second = -SVR_BOARDCAST_TIMESPACE; // 重置keepalive时间
        flag = false;
    }
    unlock();

    return flag;
}
bool SafeCltList::del(const CLIENTIP& ip)
{
    bool flag = false;
    std::map<CLIENTIP, int>::iterator it;

    lock();

    it = hostList_.find(ip);
    if (it != hostList_.end())
    {
        hostList_.erase(it); // 插入新的客户端，keepalive定为负间隔
        flag = true;
    }
    else
    {
        flag = false;
    }
    unlock();

    return flag;
}
std::vector<CLIENTIP> SafeCltList::clr()
{
    std::vector<CLIENTIP> lst;

    lock();
    
    for (std::map<CLIENTIP, int>::const_iterator it = hostList_.begin();
         it != hostList_.end(); )
    {
        lst.push_back(it->first);
        it = hostList_.erase(it);
    }

    unlock();

    return lst;
}

void SafeCltList::RefreshAliveTime()
{
    uv_cond_signal(&cond_);
}
