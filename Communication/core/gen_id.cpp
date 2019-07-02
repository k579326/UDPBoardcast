

#include <stdint.h>
#include "gen_id.h"
#include "sysheader.h"

class GenTaskID
{
public:
    GenTaskID()
    {
        uv_mutex_init(&lock_);
        taskId_ = 0;
    }
    ~GenTaskID(){
        uv_mutex_destroy(&lock_);
    }

    uint64_t applyId()
    {
        uint64_t x;
        uv_mutex_lock(&lock_);
        x = taskId_++;
        uv_mutex_unlock(&lock_);
        return x;
    }

    uint64_t taskId_;
    uv_mutex_t lock_;
};


class GenConnID
{
public:
    GenConnID()
    {
        uv_mutex_init(&lock_);
        taskId_ = 0;
    }
    ~GenConnID()
    {
        uv_mutex_destroy(&lock_);
    }

    uint16_t applyId()
    {
        uint16_t x;
        uv_mutex_lock(&lock_);
        x = taskId_++;
        uv_mutex_unlock(&lock_);
        return x;
    }

    uint16_t taskId_;
    uv_mutex_t lock_;
};


static GenTaskID g_taskId;
static GenConnID g_ConnIdForSvr;
static GenConnID g_ConnIdForClt;

uint64_t ApplyTaskId()
{
	return g_taskId.applyId();
}


uint16_t ApplyConnIdForSvr()
{
    return g_ConnIdForSvr.applyId();
}

uint16_t ApplyConnIdForClt()
{
    return g_ConnIdForClt.applyId();
}













