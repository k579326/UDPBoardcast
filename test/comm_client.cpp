




#include "Communication/comm.h"

#include "uv.h"

int main()
{

    ssn_startup_client();

    ssn_connect("192.168.1.3", 10038, 3000);

    // ssn_send();

    return 0;
}





