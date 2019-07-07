
#include <assert.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Communication/comm.h"
#include "comm_test_define.hpp"
#include "discover.h"
#include "sysheader.h"


using namespace std;


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
        auto connMap = connmgr.GetAllConn();
        for (auto it = connMap.begin(); it != connMap.end(); it++)
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
        
        Sleep(200);
    }
}

int main()
{
    int err = 0;
    uv_thread_t thread[1];
    
    ssn_startup_client(push_msg_handler, connect_changed_handler);

    
    // ssn_connect("192.168.0.233", 10038, 3000);
    // ssn_connect("192.168.1.3", 10038, 3000);
    for (int i= 0; i < 1; i++)
        uv_thread_create(&thread[i], client_send_msg, NULL);
    
    err = nd_boardcast_init();
    if (err != 0)
    {
        assert(0);
    }
    
    nd_set_running_type(CLT_RUN_TYPE);


    getchar();

    ssn_shutdown_client();
    nd_boardcast_uninit();
    
    return 0;
}





