
#include <string.h>

#include "comm_protocol.h"
#include "ssnet_err.h"


comm_pkg_t* proto_build_package(const void* buf, int buflen, uint8_t pkgType, uint64_t taskId)
{
    comm_pkg_t* pkg = (comm_pkg_t*)new char[buflen + sizeof(comm_pkg_t)];

    pkg->length = buflen;
    pkg->version = COMM_CURRENT_VERSION;
    pkg->type = pkgType;
    pkg->taskId = taskId;
    pkg->magic = COMM_MAGIC_NUM;
    memcpy(pkg->data, buf, buflen);
    return pkg;
}


comm_pkg_t* proto_parse_package(void** buf, int* remainSize)
{
    // 必须有这个判断
    if (*remainSize < sizeof(comm_pkg_t))
    {
        return NULL;
    }

    char* pNextPkg = (char*)*buf;
    comm_pkg_t* pkg = (comm_pkg_t*)*buf;
    int requireLen = pkg->length + sizeof(comm_pkg_t);
    if (requireLen > *remainSize)
    {
        return NULL;
    }

    pNextPkg = pNextPkg + requireLen;
    *buf = pNextPkg;
    *remainSize = *remainSize - requireLen;
    return pkg;
}

void proto_release_package(comm_pkg_t* pkg)
{
    delete[] pkg;
    return;
}

int proto_check_pkg(comm_pkg_t* pkg)
{
    if (pkg->magic != COMM_MAGIC_NUM){ 
        return ERR_BAD_COMM_PACKAGE;
    }
    if (pkg->version != COMM_CURRENT_VERSION){ 
        return ERR_COMM_PROTO_VERSION;
    }
    return 0;
}



