




#include "Communication/comm.h"
#include "uv.h"

int main()
{

    ssn_startup_client();

    ssn_connect("192.168.0.229", 10038, 3000);

    // ssn_send();

    return 0;
}





