


#include <string>
#include <map>
#include "ssnet_err.h"
#include "sysheader.h"


class ERRMAP
{
public:
    ERRMAP(){
        errmsg[0] = "success";

        errmsg[ERR_PARAM] = "invalid param";
        errmsg[ERR_NOT_READY] = "net module state wrong";
        errmsg[ERR_CONN_NOT_EXIST] = "connect is lost";
        errmsg[ERR_CONN_ALREADY_EXIST] = "connnect already exist, can't connect again";
        errmsg[ERR_TIMEOUT] = "operation timeout";
        errmsg[ERR_SHUTDOWN] = "loop is closed";
        errmsg[ERR_COMM_PROTO_VERSION] = "communication version error";
        errmsg[ERR_BAD_COMM_PACKAGE] = "bad communication package";
    }


    std::map<int, std::string> errmsg;
};

static ERRMAP g_errmap;


const char* ssn_errmsg(int err)
{
    return g_errmap.errmsg[err].c_str();
}
