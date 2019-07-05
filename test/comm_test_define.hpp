

#pragma once

#include <vector>
#include <map>
#include <string>


class ConnMgr
{
public:
    ConnMgr()
    {
        uv_mutex_init(&connListLock);
    }

    ~ConnMgr()
    {
        uv_mutex_destroy(&connListLock);
    }

    void AddConn(uint16_t connId, const std::string& ip)
    {
        uv_mutex_lock(&connListLock);
        auto it = connList.find(connId);
        if (it == connList.end()){ 
            connList.insert(std::pair<uint16_t, std::string>(connId, ip));
        }
        uv_mutex_unlock(&connListLock);
    }

    std::string DelConn(uint16_t connId)
    {
        std::string x;
        uv_mutex_lock(&connListLock);
        auto it = connList.find(connId);
        if (it != connList.end())
        {
            x = it->second;
            connList.erase(it);
        }
        uv_mutex_unlock(&connListLock);
        return x;
    }

    std::map<uint16_t, std::string> GetAllConn(){
        std::map<uint16_t, std::string> x;
        uv_mutex_lock(&connListLock);
        x = connList;
        uv_mutex_unlock(&connListLock);
        return x;
    }

    string FindConn(uint16_t connId)
    {
        std::string x;
        uv_mutex_lock(&connListLock);
        x = connList.find(connId)->second;
        uv_mutex_unlock(&connListLock);
        return x;
    }


    uv_mutex_t connListLock;
    std::map<uint16_t, std::string> connList;
};






