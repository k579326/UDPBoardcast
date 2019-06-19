
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include "cb_sysinfo.h"


const static char* g_x64_name[] = { "x86_64", "aarch64", "mips64el" };
const static char* g_x86_name[] = { "x86", "arm", "mipsel" };

static int get_kernel_version(char* kern_ver)
{
    struct utsname un;
    int api_ret = 0;

    api_ret = uname(&un);
    if (api_ret != 0)
    {
        return CB_SYSINFO_ERROR_SYSAPI_FAILED;
    }

    sprintf(kern_ver, "%s %s", un.release, un.version);

    return CB_SYSINFO_SUCCESS;
}


int cb_sysinfo_version(char *sys_ver, char *kern_ver)
{
    int ret = CB_SYSINFO_SUCCESS;

    if (!sys_ver && !kern_ver)
    {
        return CB_SYSINFO_ERROR_INVALID_PARAM;
    }

    if (sys_ver != NULL)
    {
        sys_ver[0] = 0;
        ret = CB_SYSINFO_SUCCESS;
    }

    if (kern_ver != NULL)
    {
        ret = get_kernel_version(kern_ver);
    }

    return ret;
}


int cb_sysinfo_bits(int *bits)
{
    struct utsname un;
    int api_ret = 0;
    int i = 0;

    if (!bits)
    {
        return CB_SYSINFO_ERROR_INVALID_PARAM;
    }

    api_ret = uname(&un);
    if (api_ret != 0)
    {
        return CB_SYSINFO_ERROR_SYSAPI_FAILED;
    }

    // 查找是否属于x64
    for (i = 0; i < sizeof(g_x64_name) / sizeof(g_x64_name[0]); i++)
    {
        if (strcmp(g_x64_name[i], un.machine) == 0)
        {
            *bits = CB_SYSINFO_X64;
            return CB_SYSINFO_SUCCESS;
        }
    }

    // 查找是否属于x86
    for (i = 0; i < sizeof(g_x86_name) / sizeof(g_x86_name[0]); i++)
    {
        if (strcmp(g_x86_name[i], un.machine) == 0)
        {
            *bits = CB_SYSINFO_X86;
            return CB_SYSINFO_SUCCESS;
        }
    }

    // 既不是64位系统也不是32位系统(可能cpu架构名称中没有包含该系统对应的信息)
    return CB_SYSINFO_ERROR_UNKNOWN_INFORMATION;
}

int cb_sysinfo_computer_name(char *name, int *len)
{
    struct utsname un;
    int api_ret = 0;
    int i = 0;

    if (name == NULL || len == NULL)
    {
        return CB_SYSINFO_ERROR_INVALID_PARAM;
    }

    api_ret = uname(&un);
    if (api_ret != 0)
    {
        return CB_SYSINFO_ERROR_SYSAPI_FAILED;
    }

    if (*len <= strlen(un.nodename))
    {
        *len = strlen(un.nodename) + 1;
        return CB_SYSINFO_ERROR_BUFFER_NOT_ENOUGH;
    }

    strcpy(name, un.nodename);
    *len = strlen(un.nodename);

    return CB_SYSINFO_SUCCESS;

}


unsigned long long cb_sysinfo_time()
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

