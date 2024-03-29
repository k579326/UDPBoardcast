

#include "select_network.h"
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <algorithm>

#include "sysheader.h"

#ifdef _WIN32
#include <iphlpapi.h>


// 下面的值是Windows的定义，但是找不到这些定义所在的头文件。不可随意更改这些值

#define NCF_VIRTUAL				0x01			// 是否虚拟网卡
#define NCF_SOFTWARE_ENUMERATED 0x02
#define NCF_PHYSICAL			0x04			// 是否物理网卡
#define NCF_HIDDEN				0x08
#define NCF_NO_SERVICE			0x10			// 该设备无驱动
#define NCF_NOT_USER_REMOVABLE	0x20
#define NCF_HAS_UI				0x80
#define NCF_FILTER				0x400	
#define NCF_NDIS_PROTOCOL		0x4000
#define NCF_LW_FILTER			0x40000


typedef struct
{
	char ip[64];
	char mask[64];
	char adpname[48];
}network_t;

typedef struct
{
	DWORD Characteristics;
	char adpname[48];
}netcard_t;


static std::vector<netcard_t> _readRegInfoForNetCard()
{
	std::vector<netcard_t> nwList;
	TCHAR szKeyName[260] = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}";
	HKEY hSubKey = NULL;
	int index = 0;

	LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, szKeyName, 0, KEY_READ, &hSubKey);
	if (lRet != ERROR_SUCCESS)
	{
		return nwList;
	}

	while (1)
	{
		netcard_t element;
		HKEY hCardkey;
		char keyname[128];
		DWORD namelen = 128;

		namelen = 128;
		LSTATUS ret = RegEnumKeyExA(hSubKey, index, keyname, &namelen, NULL, NULL, NULL, NULL);
		if (ret != ERROR_SUCCESS)
		{
			break;
		}

		index++;

		ret = RegOpenKeyExA(hSubKey, keyname, 0, KEY_READ, &hCardkey);
		if (ret != ERROR_SUCCESS)
		{
			continue;
		}

		char adpname[64];
		DWORD ncf_value;
		DWORD len = 64;

		ret = RegQueryValueExA(hCardkey, "NetCfgInstanceId", NULL, NULL, (LPBYTE)adpname, &len);
		if (ret == ERROR_SUCCESS)
		{
			strcpy(element.adpname, adpname);
		}

		len = sizeof(DWORD);
		ret = RegQueryValueExA(hCardkey, "Characteristics", NULL, NULL, (LPBYTE)&ncf_value, &len);
		if (ret == ERROR_SUCCESS)
		{
			element.Characteristics = ncf_value;
		}

		RegCloseKey(hCardkey);
		nwList.push_back(element);
	}

	RegCloseKey(hSubKey);

	return nwList;
}



static bool _select(const std::vector<network_t>& nwList, network_t* targetNw)
{
	std::vector<netcard_t> ncList;
	unsigned long boardcastaddr;

	if (nwList.empty())
	{
		return false;
	}

	ncList = _readRegInfoForNetCard();

	DWORD max;		// NCF_PHYSICAL > NCF_VIRTUAL, 所以找最大的

	for (std::vector<netcard_t>::iterator it = ncList.begin(); it != ncList.end(); )
	{
		bool hit = false;
		for (std::vector<network_t>::const_iterator in = nwList.begin(); in != nwList.end(); in++)
		{
			it->Characteristics = it->Characteristics & (NCF_VIRTUAL | NCF_PHYSICAL);
			if (strcmp(in->adpname, it->adpname) == 0 && it->Characteristics == NCF_PHYSICAL)
			{
                memcpy(targetNw, &*in, sizeof(network_t));
				return true;
			}
		}

		if (!hit)
		{
			it = ncList.erase(it);
		}
		else
		{
			it++;
		}
	}

	return false;
}


static std::vector<network_t> _AdapterList()
{
    ULONG boardcastaddr = INADDR_BROADCAST;
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    std::vector<network_t> nwList;

    pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
    {
        pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            network_t nw;

            strcpy(nw.ip, pAdapter->IpAddressList.IpAddress.String);
            strcpy(nw.mask, pAdapter->IpAddressList.IpMask.String);
            strcpy(nw.adpname, pAdapter->AdapterName);

            nwList.push_back(nw);
            pAdapter = pAdapter->Next;
        }
    }

    if (pAdapterInfo)
        free(pAdapterInfo);

    for (std::vector<network_t>::iterator it = nwList.begin(); it != nwList.end(); )
    {
        if (it->ip == std::string("0.0.0.0"))
        {
            it = nwList.erase(it);
        }
        else
        {
            it++;
        }
    }

    return nwList;
}

static bool _PhyNetConfigInfo(network_t* nw)
{
    std::vector<network_t> nwlist = _AdapterList();

    return _select(nwlist, nw);
}


unsigned long PhyBoardcastAddr()
{
    unsigned long boardcastaddr = INADDR_BROADCAST;
    network_t nw;

    if (_PhyNetConfigInfo(&nw))
    {
        ULONG ip, mask;
        ip = inet_addr(nw.ip);
        mask = inet_addr(nw.mask);
        boardcastaddr = ip | ~mask;
    }

    return boardcastaddr;
}

unsigned long PhyIpAddress()
{
    unsigned long ip = INADDR_ANY;
    network_t nw;

    if (_PhyNetConfigInfo(&nw))
    {
        ip = inet_addr(nw.ip);
    }

    return ip;
}


#else

#include <fcntl.h>
#include <netinet/in.h>
unsigned long PhyBoardcastAddr()
{
	return INADDR_BROADCAST;
}
unsigned long PhyIpAddress()
{
    return INADDR_ANY;
}
#endif


