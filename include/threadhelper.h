


#pragma once

#include <stdint.h>
#include "sysheader.h"

class sleeper
{
public:    
    sleeper(){
        uv_mutex_init(&mutex_);
        uv_cond_init(&cond_);
    }
    ~sleeper(){
        uv_mutex_destroy(&mutex_);
        uv_cond_destroy(&cond_);
    }
    
    bool sleep(uint64_t millisecond){
        
        int t = 0;
        uv_mutex_lock(&mutex_);
        t = uv_cond_timedwait(&cond_, &mutex_, millisecond);
        uv_mutex_unlock(&mutex_);
        
        return t != 0;
    }
    
    void wakeup(){
        uv_cond_signal(&cond_);
    }
    
    
private:
    uv_mutex_t mutex_;
    uv_cond_t cond_;
        
};












