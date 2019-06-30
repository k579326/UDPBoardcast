


#include "task_id.h"
#include "sysheader.h"

class TaskID
{
public:
	TaskID() {
		uv_mutex_init(&lock_);
		taskId_ = 0;
	}
	~TaskID {
		uv_mutex_destroy(&lock_);
	}
	
	uint64_t applyId(){
		
		uint64_t x;
		uv_mutex_lock(&lock_);
		x = TaskID++;
		uv_mutex_unlock(&lock_);
		return x;
	}
	
	uint64_t taskId_;
	uv_mutex_t lock_;
}


static TaskID g_var;


uint64_t ApplyTaskId()
{
	return g_var.applyId();
}