

#include <string.h>
#include <vector>
#include <list>
#include "connect_info.h"
#include "uv.h"

using namespace std;

class SafeHostList
{
public:
	SafeHostList(){
		uv_mutex_init(&mutex_);
	}
	~SafeHostList(){
		uv_mutex_destroy(&mutex_);
	}

	list<host_info_t> hostList_;

	void lock(){ 
		uv_mutex_lock(&mutex_);
	}
	void unlock(){ 
		uv_mutex_unlock(&mutex_);
	}

private:

	uv_mutex_t mutex_;
};



static SafeHostList g_client_list;
static SafeHostList g_server_list;



int conn_add_client(char* ip)
{
	list<host_info_t>::const_iterator it;
	host_info_t host;
	strcpy(host.ip, ip);

	g_client_list.lock();
	for (; it != g_client_list.hostList_.end(); it++)
	{
		if (strcmp(it->ip, ip) == 0)
		{
			break;
		}
	}
	if (it == g_client_list.hostList_.end())
	{
		g_client_list.hostList_.push_back(host);
	}
	g_client_list.unlock();

	return 0;
}
int conn_del_client(char* ip)
{
	return 0;
}
int conn_clr_client(char* ip)
{
	return 0;
}

int conn_add_server(char* ip)
{
	list<host_info_t>::const_iterator it;
	host_info_t host;
	strcpy(host.ip, ip);

	g_server_list.lock();
	for (; it != g_server_list.hostList_.end(); it++)
	{
		if (strcmp(it->ip, ip) == 0)
		{
			break;
		}
	}
	if (it == g_server_list.hostList_.end())
	{
		g_server_list.hostList_.push_back(host);
	}
	g_server_list.unlock();

	return 0;
}
int conn_del_server(char* ip)
{
	list<host_info_t>::iterator it;
	g_server_list.lock();
	for (; it != g_server_list.hostList_.end(); )
	{
		if (strcmp(it->ip, ip) == 0)
		{
			g_server_list.hostList_.erase(it);
		}
		else
		{
			it++;
		}
	}
	g_server_list.unlock();
	return 0;
}
int conn_clr_server(char* ip)
{
	g_server_list.lock();
	g_server_list.hostList_.clear();
	g_server_list.unlock();
	return 0;
}
