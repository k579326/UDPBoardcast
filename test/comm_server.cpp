


#include "Communication/comm.h"

#include "uv.h"


void server_thread(void* param)
{

}


int main()
{
    uv_sem_t sem;
    uv_sem_init(&sem, 1);
    uv_sem_wait(&sem);
    ssn_startup_server();


    uv_sem_wait(&sem);



    return 0;
}



