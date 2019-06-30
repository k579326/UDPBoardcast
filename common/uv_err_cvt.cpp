


#include <string>
#include <map>
#include "ssnet_err.h"
#include "sysheader.h"


static std::map<int, std::string> g_errmsg =
{
    {0, "success"},
    {ERR_PARAM, "invalid param"},
    {ERR_NOT_READY, "net module state wrong"},
    {ERR_CONN_NOT_EXIST, "connect is lost"},
    {ERR_TIMEOUT, "operation timeout"}
};



int uverr_convert(int uverr)
{
    return 0;
}


const char* ssn_errmsg(int err)
{
    return g_errmsg[err].c_str();
}
