
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <CoreServices/CoreServices.h>
#include <TargetConditionals.h>
#include "cb_sysinfo.h"


typedef struct 
{
    SInt32  major;
    SInt32  minor;
    char*   sysname;
}cb_sysver_t;


const static char* g_x64_name[] = { "x86_64", "aarch64", "mips64el" };
const static char* g_x86_name[] = { "x86", "arm", "mipsel" };

/** mac系统各版本的版本号与系统名称对应关系*/ 
/** 如需更新，可参照该网站 https://www.applex.net/pages/macos*/ 
const static cb_sysver_t g_macos_ver[] = {
    {10, 13,    "macOS High Sierra"},
    {10, 12,    "macOS Sierra"},
    {10, 11,    "OS X EI Captian"},
    {10, 10,    "OS X Yosemite"},
    {10, 9,     "OS X Mavericks"},
    {10, 8,     "OS X Mountain Lion"},
    {10, 7,     "OS X Lion"},
    {10, 6,     "MAC OS X Snow Leopard"},
    {10, 5,     "MAC OS X Leopard"},
    {0,  0,     "Unknown OS"}
};


static char* get_macos_name(SInt32 major, SInt32 minor)
{
    int i = 0;
    for (; i < sizeof(g_macos_ver) / sizeof(g_macos_ver[0]); i++)
    {
        if (major == g_macos_ver[i].major && minor == g_macos_ver[i].minor)
        {
            return g_macos_ver[i].sysname;
        }
    }
    
    return g_macos_ver[i - 1].sysname;
}


static int get_macos_version(char* sys_ver)
{
    SInt32 major_ver = 0, minor_ver = 0, bugfix_ver = 0;
    char* sysname = NULL;
    
    Gestalt(gestaltSystemVersionMajor, &major_ver);
    Gestalt(gestaltSystemVersionMinor, &minor_ver);
    Gestalt(gestaltSystemVersionBugFix, &bugfix_ver);
    
    sysname = get_macos_name(major_ver, minor_ver);
    sprintf(sys_ver, "%s(%d.%d.%d)", sysname, major_ver, minor_ver, bugfix_ver);
    return CB_SYSINFO_SUCCESS;
}


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
        ret = get_macos_version(sys_ver);
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

#if TARGET_CPU_X86 || TARGET_CPU_ARM
static int get_physical_meminfo(cb_sysinfo_mem_t* meminfo)
{
    vm_size_t page_sz;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    
    vm_statistics_data_t    vm_stat;
    count = HOST_VM_INFO_COUNT;
    
    mach_port = mach_host_self();
    host_page_size(mach_port, &pagesz);
    
    host_statistics(mach_port, HOST_VM_INFO, (host_info_t)&vm_stat, &count);
    
    meminfo->total_phys_mem = (vm_stat.active_count + vm_stat.inactive_count + vm_stat.wire_count + vm_stat.free_count) * page_sz;
    meminfo->avail_phys_mem = vm_stat.purgeable_count * page_sz;
    
    return CB_SYSINFO_SUCCESS;
}
#elif TARGET_CPU_X86_64 || TARGET_CPU_ARM64
static int get_physical_meminfo64(cb_sysinfo_mem_t* meminfo)
{
    vm_size_t page_sz;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    
    vm_statistics64_data_t  vm_stat;
    count = HOST_VM_INFO64_COUNT;
    
    mach_port = mach_host_self();
    host_page_size(mach_port, &page_sz);
    
    host_statistics64(mach_port, HOST_VM_INFO64, (host_info64_t)&vm_stat, &count);
    
    meminfo->total_phys_mem = (vm_stat.active_count + vm_stat.inactive_count + vm_stat.wire_count + vm_stat.free_count + vm_stat.compressor_page_count) * page_sz;
    meminfo->avail_phys_mem = (vm_stat.purgeable_count + vm_stat.external_page_count) * page_sz;
    
    return CB_SYSINFO_SUCCESS;
}
#elif
#error "not support architecture!"
#endif

int cb_sysinfo_meminfo(cb_sysinfo_mem_t *meminfo)
{
    int api_ret = 0;
    int mib[2];
    size_t msize;
    struct xsw_usage xu;
    
    if (meminfo == NULL)
    {
        return CB_SYSINFO_ERROR_INVALID_PARAM;
    }
    
#if TARGET_CPU_X86 || TARGET_CPU_ARM
    /** 获取物理内存信息*/
    get_physical_meminfo(meminfo);
#elif TARGET_CPU_X86_64 || TARGET_CPU_ARM64
    /** 获取物理内存信息 64位*/
    get_physical_meminfo64(meminfo);
#elif
#error "unknown architecture"
#endif
   
   mib[0] = CTL_VM;
   mib[1] = VM_SWAPUSAGE;
   msize = sizeof(xu);
   
   api_ret = sysctl(mib, sizeof(mib)/sizeof(mib[0]), &xu, &msize, NULL, 0);
   if (api_ret != 0)
   {
       return CB_SYSINFO_ERROR_SYSAPI_FAILED;
   }
   
   meminfo->total_vir_mem = xu.xsu_total;
   meminfo->avail_vir_mem = xu.xsu_avail;
   
   return CB_SYSINFO_SUCCESS;
}


unsigned long long cb_sysinfo_time()
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

