
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys_header/VersionHelpers.h"

#include "cb_sysinfo.h"

#define CB_SYSINFO_KERNEL32_DLL "kernel32.dll"

typedef struct
{
    /** 版本定义, 由win系统提供*/
    int product_info;
    /** product_info表示的版本的名称*/
    char* product_name;
}cb_sysinfo_win8_ver_t;

typedef void (WINAPI *procGetSysinfo)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *procGetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);

const static cb_sysinfo_win8_ver_t g_win8_ver_map[] = 
{
    { PRODUCT_ULTIMATE,                         " Ultimate Edition" },
    { PRODUCT_PROFESSIONAL,                     " Professional" },
    { PRODUCT_HOME_PREMIUM ,                    " Home Premium Edition" },
    { PRODUCT_HOME_BASIC,                       " Home Basic Edition" },
    { PRODUCT_ENTERPRISE,                       " Enterprise Edition" },
    { PRODUCT_BUSINESS,                         " Business Edition" },
    { PRODUCT_STARTER,                          " Starter Edition"},
    { PRODUCT_CLUSTER_SERVER,                   " Cluster Server Edition"},
    { PRODUCT_DATACENTER_SERVER,                " Datacenter Edition" },
    { PRODUCT_DATACENTER_SERVER_CORE,           " Datacenter Edition (core installation)" },
    { PRODUCT_ENTERPRISE_SERVER,                " Enterprise Edition" },
    { PRODUCT_ENTERPRISE_SERVER_CORE,           " Enterprise Edition (core installation)" },
    { PRODUCT_ENTERPRISE_SERVER_IA64,           " Enterprise Edition for Itanium-based Systems" },
    { PRODUCT_SMALLBUSINESS_SERVER,             " Small Business Server" },
    { PRODUCT_SMALLBUSINESS_SERVER_PREMIUM,     " Small Business Server Premium Edition" },
    { PRODUCT_STANDARD_SERVER,                  " Standard Edition" },
    { PRODUCT_STANDARD_SERVER_CORE,             " Standard Edition (core installation)" },
    { PRODUCT_WEB_SERVER,                       " Web Server Edition" }
};


static int inner_get_sys_version(char* sys_ver)
{
    char buf[128] = { 0 };
    char version[CB_SYSINFO_SV_LEN] = { 0 };
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    procGetSysinfo func1;
    procGetProductInfo func2;
    BOOL bret;
    DWORD dwType;
    int i = 0;

    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionExA((OSVERSIONINFO*)&osvi))
    {
        return CB_SYSINFO_ERROR_SYSAPI_FAILED;
    }
    
    /** GetNativeSystemInfo return void always*/
    func1 = (procGetSysinfo)GetProcAddress(GetModuleHandleA(CB_SYSINFO_KERNEL32_DLL), "GetNativeSystemInfo");
    if (NULL != func1)
    {
        func1(&si);
    }
    else
    {
        /** GetSystemInfo return void always*/
        GetSystemInfo(&si);
    }

    if (IsWindows10OrGreater())
    {
        strcpy(version, "Microsoft");

        if (osvi.wProductType == VER_NT_WORKSTATION)
        {
            strcat(version, " Windows 10");
        }
        else
        {
            strcat(version, " Windows Server 2016 Technical Preview");
        }
    }
    else if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion > 4)
    {
        strcpy(version, "Microsoft");

        if (osvi.dwMajorVersion == 6)
        {
            if (osvi.dwMinorVersion == 0)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                {
                    strcat(version, " Windows Vista");
                }
                else
                {
                    strcat(version, " Windows Server 2008");
                }
            }

            if (osvi.dwMinorVersion == 1)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                {
                    strcat(version, " Windows 7");
                }
                else
                {
                    strcat(version, " Windows Server 2008 R2");
                }
            }

            if (osvi.dwMinorVersion == 2)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                {
                    strcat(version, " Windows 8");
                }
                else
                {
                    strcat(version, " Windows Server2012 R2");
                }
            }

            if (osvi.dwMinorVersion == 3)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                {
                    strcat(version, " Windows 8.1");
                }
                else
                {
                    strcat(version, " Windows Server2012 R2");
                }
            }

            func2 = (procGetProductInfo)GetProcAddress(GetModuleHandleA(CB_SYSINFO_KERNEL32_DLL), "GetProductInfo");
            if (!func2)
            {
                return CB_SYSINFO_ERROR_SYSAPI_FAILED;
            }

            bret = func2(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
            if (!bret)
            {
                return CB_SYSINFO_ERROR_SYSAPI_FAILED;
            }

            for (i = 0; i < sizeof(g_win8_ver_map) / sizeof(cb_sysinfo_win8_ver_t); i++)
            {
                if (g_win8_ver_map[i].product_info == dwType)
                {
                    strcat(version, g_win8_ver_map[i].product_name);
                }
            }
        }

        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
        {
            if (GetSystemMetrics(SM_SERVERR2))
            {
                strcat(version, " Windows Server 2003 R2");
            }
            else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
            {
                strcat(version, " Windows Storage Server 2003");
            }
            else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
            {
                strcat(version, " Windows Home Server");
            }
            else if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            {
                strcat(version, " Windows XP Professional x64 Edition");
            }
            else
            {
                strcat(version, (" Windows Server 2003"));
            }

            if (osvi.wProductType != VER_NT_WORKSTATION)
            {
                if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
                {
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    {
                        strcat(version, " Datacenter Edition for Itanium-based Systems");
                    }
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    {
                        strcat(version, " Enterprise Edition for Itanium-based Systems");
                    }
                }

                else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                {
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    {
                        strcat(version, " Datacenter x64 Edition");
                    }
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    {
                        strcat(version, " Enterprise x64 Edition");
                    }
                    else
                    {
                        strcat(version, " Standard x64 Edition");
                    }
                }
                else
                {
                    if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
                    {
                        strcat(version, " Compute Cluster Edition");
                    }
                    else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    {
                        strcat(version, " Datacenter Edition");
                    }
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    {
                        strcat(version, " Enterprise Edition");
                    }
                    else if (osvi.wSuiteMask & VER_SUITE_BLADE)
                    {
                        strcat(version, " Web Edition");
                    }
                    else
                    {
                        strcat(version, " Standard Edition");
                    }
                }
            }
        }

        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
        {
            strcat(version, "Windows XP");
            if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
            {
                strcat(version, " Home Edition");
            }
            else
            {
                strcat(version, " Professional");
            }
        }

        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
        {
            strcat(version, "Windows 2000");

            if (osvi.wProductType == VER_NT_WORKSTATION)
            {
                strcat(version, " Professional");
            }
            else
            {
                if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                {
                    strcat(version, " Datacenter Server");
                }
                else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                {
                    strcat(version, " Advanced Server");
                }
                else
                {
                    strcat(version, " Server");
                }
            }
        }
    }
    else
    {
        return CB_SYSINFO_ERROR_NOT_SUPPORT_SYSTEM;
    }

    if (strlen(osvi.szCSDVersion) > 0)
    {
        strcat(version, (" "));
        strcat(version, osvi.szCSDVersion);
    }


    sprintf(buf, "(build %d)", osvi.dwBuildNumber);
    strcat(version, buf);

    if (osvi.dwMajorVersion >= 6)
    {
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        {
            strcat(version, (" x64"));
        }
        else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        {
            strcat(version, (" x86"));
        }
    }

    strcpy(sys_ver, version);
    
    return CB_SYSINFO_SUCCESS;
}

static int get_file_verison(const char* file_path, DWORD *HMs, DWORD *LMs, DWORD *HLs)
{
    DWORD handle = 0;
    DWORD ver_size = 0;
    BOOL ret = FALSE;
    char* verinfo_buf = NULL;

    VS_FIXEDFILEINFO* info = NULL;
    unsigned int info_len = 0;

    ver_size = GetFileVersionInfoSizeA(file_path, &handle);
    if (0 == ver_size)
    {
        return CB_SYSINFO_ERROR_SYSAPI_FAILED;
    }

    verinfo_buf = malloc(ver_size);
    memset(verinfo_buf, 0, CB_SYSINFO_KV_LEN);

    ret = GetFileVersionInfoA(file_path, handle, ver_size, verinfo_buf);
    if (TRUE != ret)
    {
        ret = CB_SYSINFO_ERROR_SYSAPI_FAILED;
        goto exit;
    }

    ret = VerQueryValueA(verinfo_buf, "\\", (void**)&info, &info_len);
    if (TRUE != ret)
    {
        ret = CB_SYSINFO_ERROR_SYSAPI_FAILED;
        goto exit;
    }

    *HMs = HIWORD(info->dwFileVersionMS);
    *LMs = LOWORD(info->dwFileVersionMS);
    *HLs = HIWORD(info->dwFileVersionLS);

exit:

    free(verinfo_buf);
    return CB_SYSINFO_SUCCESS;
}


static int inner_get_kern_version(char* kern_ver)
{
    OSVERSIONINFOA osver;
    DWORD majorVersion = 0;
    DWORD minorVersion = 0;
    DWORD buildNumber = 0;
    int ret = 0;

    if (IsWindows8Point1OrGreater())
    {
        ret = get_file_verison(CB_SYSINFO_KERNEL32_DLL, &majorVersion, &minorVersion, &buildNumber);
    }
    else
    {
        osver.dwOSVersionInfoSize = sizeof(osver);
        ret = (GetVersionExA(&osver) == TRUE ? 0 : -1);
        if (ret == 0)
        {
            majorVersion = osver.dwMajorVersion;
            minorVersion = osver.dwMinorVersion;
            buildNumber = osver.dwBuildNumber; 
        }
    }

    if (ret != 0)
    {
        printf("error : %d", GetLastError());
        return CB_SYSINFO_ERROR_SYSAPI_FAILED;
    }

    sprintf(kern_ver, "%d.%d.%d", majorVersion, minorVersion, buildNumber);

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
        ret = inner_get_sys_version(sys_ver);
    }
    if (ret != CB_SYSINFO_SUCCESS)
    {
        return ret;
    }

    if (kern_ver != NULL)
    {
        ret = inner_get_kern_version(kern_ver);
    }

    return ret;
}

int cb_sysinfo_bits(int *bits)
{
    procGetSysinfo func1;
    SYSTEM_INFO si;

    if (!bits)
    {
        return CB_SYSINFO_ERROR_INVALID_PARAM;
    }

    /** GetNativeSystemInfo return void always*/
    func1 = (procGetSysinfo)GetProcAddress(GetModuleHandleA(CB_SYSINFO_KERNEL32_DLL), "GetNativeSystemInfo");
    if (NULL != func1)
    {
        func1(&si);
    }
    else
    {
        /** GetSystemInfo return void always*/
        GetSystemInfo(&si);
    }

    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64
        || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    {
        *bits = CB_SYSINFO_X64;
    }
    else
    {
        *bits = CB_SYSINFO_X86;
    }

    return CB_SYSINFO_SUCCESS;
}

int cb_sysinfo_computer_name(char *name, int* len)
{
    if (name == NULL || len == NULL)
    {
        return CB_SYSINFO_ERROR_INVALID_PARAM;
    }

    if (!GetComputerName(name, len))
    {
        if (GetLastError() == ERROR_BUFFER_OVERFLOW)
        {
            return CB_SYSINFO_ERROR_BUFFER_NOT_ENOUGH;
        }
        else
        {
            return CB_SYSINFO_ERROR_SYSAPI_FAILED;
        }
    }

    return CB_SYSINFO_SUCCESS;
}


int cb_sysinfo_meminfo(cb_sysinfo_mem_t *meminfo)
{
    MEMORYSTATUSEX ms;
    BOOL bret;

    if (meminfo == NULL)
    {
        return CB_SYSINFO_ERROR_INVALID_PARAM;
    }

    ms.dwLength = sizeof(ms);
    bret = GlobalMemoryStatusEx(&ms);
    if (!bret)
    {
        return CB_SYSINFO_ERROR_SYSAPI_FAILED;
    }
    
    meminfo->total_phys_mem = ms.ullTotalPhys;
    meminfo->avail_phys_mem = ms.ullAvailPhys;
    meminfo->total_vir_mem = ms.ullTotalVirtual;
    meminfo->avail_vir_mem = ms.ullAvailVirtual;

    return CB_SYSINFO_SUCCESS;
}



unsigned long long cb_sysinfo_time()
{
    SYSTEMTIME systime;
    struct tm tm;
    time_t time;

    GetLocalTime(&systime);

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = systime.wYear - 1900;
    tm.tm_mon = systime.wMonth - 1;
    tm.tm_mday = systime.wDay;
    tm.tm_hour = systime.wHour;
    tm.tm_min = systime.wMinute;
    tm.tm_sec = systime.wSecond;

    time = mktime(&tm) * 1000 + systime.wMilliseconds;

    return time;
}