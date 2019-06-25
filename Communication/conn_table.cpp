

#include <map>



#include "comm_define.h"
#include "conn_table.h"


using namespace std;


typedef struct
{
    uv_rwlock_t lock;
    map<uint16_t, tcp_conn_t> table;
}safe_table_t;

static safe_table_t g_client_table;
static safe_table_t g_server_table;















