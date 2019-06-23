

#pragma once

#include <vector>
#include <map>
#include "sysheader.h"
#include "ssnet_define.h"

class SafeSvrList
{
private:
	SafeSvrList()
	{
		uv_mutex_init(&mutex_);
	}
	~SafeSvrList()
	{
		uv_mutex_destroy(&mutex_);
	}

	std::vector<peer_info_t> hostList_;

	void lock()
	{
		uv_mutex_lock(&mutex_);
	}
	void unlock()
	{
		uv_mutex_unlock(&mutex_);
	}
	uv_mutex_t mutex_;

public:
	bool add(const peer_info_t& peer);
	bool del(const peer_info_t& peer);
	std::vector<peer_info_t> clr();

	static SafeSvrList* getInstance();

};



typedef std::string CLIENTIP;

class SafeCltList
{
private:
	SafeCltList()
	{
		uv_mutex_init(&mutex_);
        uv_cond_init(&cond_);

        uv_thread_create(&thread_, _keepalive, this);
	}
	~SafeCltList()
	{
        uv_cond_destroy(&cond_);
		uv_mutex_destroy(&mutex_);
	}

	std::map<CLIENTIP, int> hostList_;

	void lock()
	{
		uv_mutex_lock(&mutex_);
	}
	void unlock()
	{
		uv_mutex_unlock(&mutex_);
	}

    static void _keepalive(void* param);

    uv_thread_t thread_;
	uv_mutex_t mutex_;
    uv_cond_t cond_;
public:

    bool add(const CLIENTIP& ip);
    bool del(const CLIENTIP& ip);
    std::vector<CLIENTIP> clr();

    void RefreshAliveTime();

    static SafeCltList* getInstance();
};