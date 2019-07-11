
#include <assert.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "comm.h"
#include "comm_test_define.hpp"
#include "discover.h"
#include "sysheader.h"
#include "bridge/connreq_mgr.h"
#include "tinyxml.h"

using namespace std;

bool g_exit = false;
static ConnMgr connmgr;


void connect_changed_handler(uint16_t connId, const char* ip, bool status)
{
    string ip_changed;

    if (status){
        connmgr.AddConn(connId, ip);
        ip_changed = ip;
    }
    else
    {
        ip_changed = connmgr.DelConn(connId);
    }

    string change;
    change = status ? "Connect" : "Disconnect";

    printf("[Conn Msg] IP: %s, ConnId: %d, status: %s\n", ip_changed.c_str(), connId, change.c_str());
}
void push_msg_handler(uint16_t connId, const void* data, int datalen)
{
    char pushmsg[256] = {0};

    memcpy(pushmsg, data, datalen);

    string ip = connmgr.FindConn(connId);

    printf("[Push Msg] %s From Server: %s\n", pushmsg, ip.c_str());
}


void client_send_msg(void* param)
{
    void* outbuf;
    

    while (1)
    {
        if (g_exit)
        {
            break;
        }
        std::map < uint16_t, std::string> connMap = connmgr.GetAllConn();
        for (std::map < uint16_t, std::string>::iterator it = connMap.begin(); it != connMap.end(); it++)
        {
            int outlen = 256;
            char outstr[256] = { 0 };

            uint64_t tickcount = uv_hrtime();
            string msg = "client msg...";

            int err = ssn_send(it->first, msg.c_str(), msg.size(), &outbuf, &outlen, 1000);
            if (err == 0)
            {
                memcpy(outstr, outbuf, outlen);
                printf("[Resp Msg] msg: %s from Server %s. ###### Lost Time: %llu ms\n", 
                       outstr, it->second.c_str(), (uv_hrtime() - tickcount) / 1000 / 1000);
                free(outbuf);
            }
            else
            {
                printf("[Err Msg] Failed! To Server %s\n", it->second.c_str());
            }
        }

        if (g_exit)
        {
            break;
        }
        ssn_sleep(200);
    }
}

typedef struct
{
    string ip;
    short port;
}svr_config_t;

static int load_config(vector<svr_config_t>& configs)
{
    string host, user, pwd, dbName;
    short port;
    TiXmlDocument doc;

    if (!doc.LoadFile("./remote_svr.xml"))
    {
        std::string errStr = doc.ErrorDesc();
        int rowID = doc.ErrorRow();
        cout << "load remote_svr.xml failed! " << errStr << "err line: " << rowID << endl;
        return -1;
    }

    TiXmlNode* node = doc.FirstChild("ServerList");
    if (!node)
    {
        cout << "remote_svr.xml is bad!" << endl;
        return -1;
    }


    TiXmlElement* element = node->FirstChildElement("SVR");
    while (1)
    {
        if (element == NULL)
        {
            break;
        }

        svr_config_t config;

        const char* ip = element->Attribute("IP");
        if (ip == NULL)
        {
            continue;
        }
        const char* port = element->Attribute("PORT");
        if (port == NULL)
        {
            continue;
        }

        config.ip = ip;
        config.port = strtol(port, NULL, 10);
        configs.push_back(config);
        element = static_cast<TiXmlElement*>(element->NextSibling());
    }

    return 0;
}


int main()
{
    int err = 0;
    uv_thread_t thread[1];
    vector<svr_config_t> configs;
    // º”‘ÿ≈‰÷√
    load_config(configs);

    ssn_startup_client(push_msg_handler, connect_changed_handler);

    for (int i= 0; i < 1; i++)
        uv_thread_create(&thread[i], client_send_msg, NULL);
    
    ssn_sleep(100);
    for (int i = 0; i < configs.size(); i++)
    {
        deliver_addsvr_msg(configs[i].ip.c_str(), configs[i].port);
    }

    nd_set_running_type(CLT_RUN_TYPE);
    getchar();

    g_exit = true;
    for (int i = 0; i < 1; i++){
        uv_thread_join(&thread[i]);
    }

    nd_set_running_type(NONE_RUN_TYPE);
    err = ssn_shutdown_client();
    
    return err;
}





