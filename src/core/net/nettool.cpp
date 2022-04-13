#include "stdafx.h"
#include "nettool.h"
#include "base/stringtool.h"

#ifdef _MSC_VER
#pragma comment(lib,"Iphlpapi.lib")
#include <Iptypes.h>
#include <Iphlpapi.h>
#include <IPExport.h>
#include <ws2ipdef.h>
#else
#include <sys/types.h>
#include <ifaddrs.h>

#ifndef __APPLE__
#include <net/route.h>
#endif

#endif

using namespace std;

namespace Bear {
namespace Core
{
using namespace FileSystem;
namespace Net {

static const char* TAG = "NetTool";

//判断是否为有效的mac地址
//
bool NetTool::IsValidMac(const char *mac)
{
	if (!mac)
	{
		return FALSE;
	}

	int len = (int)strlen(mac);
	if (len != 17 && len != 12)
	{
		return FALSE;
	}

	if (len == 12)//没有采用:或者-分隔符
	{
		for (int i = 0; i < len; i++)
		{
			if (!StringTool::IsHexChar(mac[i]))
				return FALSE;
		}

		return TRUE;
	}

	if (!mac || strlen(mac) != 17)//strlen("xx:xx:xx:xx:xx:xx"))
		return FALSE;

	{
		string szSave = mac;
		StringTool::Replace(szSave, "-", ":");

		if (szSave[2] != ':' || szSave[5] != ':' || szSave[8] != ':' || szSave[11] != ':' || szSave[14] != ':')
		{
			return FALSE;
		}

		StringTool::Replace(szSave, ":", "0");
		for (UINT i = 0; i < szSave.length(); i++)
		{
			if (!StringTool::IsHexChar(szSave[i]))
				return FALSE;
		}
	}

	return TRUE;
}

#ifdef _MSC_VER
#pragma comment(lib,"Iphlpapi.lib")

string NetTool::GetLanIP(string eth)
{
	const int nc = 100;
	auto pAdapterInfo = new IP_ADAPTER_INFO[nc];
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO) * nc;

	vector<string> ips;

	auto ret = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	if (ret)
	{
		pAdapterInfo->Type = IF_TYPE_OTHER;
		pAdapterInfo->Next = nullptr;

		SAFE_DELETEA(pAdapterInfo);
		return "";
	}

	for (auto adapter = pAdapterInfo; adapter; adapter = adapter->Next)
	{
		auto type = adapter->Type;
		if (type != IF_TYPE_IEEE80211 && type != MIB_IF_TYPE_ETHERNET)
		{
			continue;
		}

#ifdef _DEBUG
		string mac;

		for (UINT i = 0; i < adapter->AddressLength; i++)
		{
			if (i != 0)
			{
				mac += "-";
			}

			StringTool::AppendFormat(mac, "%.2X", (int)adapter->Address[i]);
		}

		//LogV(TAG,"Adapter [%s] desc=[%s],mac=[%s]", adapter->AdapterName, adapter->Description, mac.c_str());
#endif

		for (auto item = &adapter->IpAddressList; item; item = item->Next)
		{
			string ip = item->IpAddress.String;
			if (ip == "0.0.0.0" || ip.empty())
			{
				continue;
			}

			//LogV(TAG,"IP Address: %s", ip.c_str());
			ips.push_back(ip);
		}
	}


	string ip;
	for (auto iter = ips.begin(); iter != ips.end(); ++iter)
	{
		if (iter->find("192.") == 0)
		{
			ip = *iter;
			break;
		}
	}

	if (!ips.empty())
	{
		ip = ips[0];
	}

	SAFE_DELETEA(pAdapterInfo);
	return ip;
}

#else
string NetTool::GetLanIP(string eth)
{
#ifdef _CONFIG_ANDROID
	return "";
#else
	string szIP;
	struct ifaddrs * ifAddrStruct = NULL;
	struct ifaddrs * ifa = NULL;
	void * tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr->sa_family == AF_INET)
		{
			// check it is IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			bzero(addressBuffer, sizeof(addressBuffer));
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if (eth == ifa->ifa_name)
			{
				//LogV(TAG,"%s IP Address %s", ifa->ifa_name, addressBuffer); 
				szIP = addressBuffer;
				break;
			}
		}
	}
	if (ifAddrStruct)
	{
		freeifaddrs(ifAddrStruct);
		ifAddrStruct = NULL;
	}

	return szIP;
#endif
}

#endif

int GetMacList(vector<string>& items)
{
	items.clear();

#ifdef _MSC_VER
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	UINT i;

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
	{
		LogV(TAG,"Error allocating memory needed to call GetAdaptersinfo\n");
		return -1;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			LogV(TAG,"Error allocating memory needed to call GetAdaptersinfo\n");
			return -1;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			string mac;
			for (i = 0; i < pAdapter->AddressLength; i++)
			{
				if (i == (pAdapter->AddressLength - 1))
					mac += Core::StringTool::Format("%.2X", (int)pAdapter->Address[i]);
				else
					mac += Core::StringTool::Format("%.2X-", (int)pAdapter->Address[i]);
			}

			items.push_back(mac);

			//LogV(TAG,"Adapter[%s],mac=[%s],desc[%s]", pAdapter->AdapterName, mac.c_str(), pAdapter->Description);
			//LogV(TAG,"Type: ");
			/*
			switch (pAdapter->Type) {
			case MIB_IF_TYPE_OTHER:
			LogV(TAG,"Other\n");
			break;
			case MIB_IF_TYPE_ETHERNET:
			LogV(TAG,"Ethernet\n");
			break;
			case MIB_IF_TYPE_TOKENRING:
			LogV(TAG,"Token Ring\n");
			break;
			case MIB_IF_TYPE_FDDI:
			LogV(TAG,"FDDI\n");
			break;
			case MIB_IF_TYPE_PPP:
			LogV(TAG,"PPP\n");
			break;
			case MIB_IF_TYPE_LOOPBACK:
			LogV(TAG,"Lookback\n");
			break;
			case MIB_IF_TYPE_SLIP:
			LogV(TAG,"Slip\n");
			break;
			default:
			LogV(TAG,"Unknown type %ld\n", pAdapter->Type);
			break;
			}

			LogV(TAG,"IP Address:%s", pAdapter->IpAddressList.IpAddress.String);
			//*/
			pAdapter = pAdapter->Next;
		}
	}
	else {
		LogV(TAG,"GetAdaptersInfo failed with error: %d\n", dwRetVal);

	}
	if (pAdapterInfo)
		FREE(pAdapterInfo);
#endif
	return 0;
}

vector<string> NetTool::GetMacs()
{
	vector<string> items;
	GetMacList(items);
	return items;
}

bool NetTool::MacExists(const string& mac)
{
	auto items = GetMacs();
	for (auto iter = items.begin(); iter != items.end(); ++iter)
	{
		auto& item = *iter;
		if (Core::StringTool::CompareNoCase(mac, item) == 0)
		{
			return true;
		}
	}

	return false;
}

string NetTool::GetNetSection(string ip, string mask)
{
	int arrIP[4];
	int arrMask[4];

	bzero(arrIP, sizeof(arrIP));
	bzero(arrMask, sizeof(arrMask));

	sscanf(ip.c_str(), "%d.%d.%d.%d", &arrIP[0], &arrIP[1], &arrIP[2], &arrIP[3]);
	sscanf(mask.c_str(), "%d.%d.%d.%d", &arrMask[0], &arrMask[1], &arrMask[2], &arrMask[3]);

	string netSection = StringTool::Format("%d.%d.%d.%d",
		arrIP[0] & arrMask[0],
		arrIP[1] & arrMask[1],
		arrIP[2] & arrMask[2],
		arrIP[3] & arrMask[3]
	);

	return netSection;
}

string NetTool::GetMac(string eth)
{
#ifdef _MSC_VER
	{
		BYTE macAddr[6] = { 0 };
		IP_ADAPTER_INFO   AdapterInfo[16];
		bzero(AdapterInfo, sizeof(AdapterInfo));
		ULONG len = sizeof(AdapterInfo);
		DWORD status = GetAdaptersInfo(AdapterInfo, &len);
		if (status == ERROR_SUCCESS)
		{
			for (int i = 0; i < COUNT_OF(AdapterInfo); i++)
			{
				IP_ADAPTER_INFO *pi = &AdapterInfo[i];
				memcpy(macAddr, pi->Address, 6);//先复制了再说,后面找到最合适的时，会再次复制

				//LogV(TAG,"AdapterName=[%s]----------------------------", pi->AdapterName);
				//LogV(TAG,"Description=[%s]", pi->Description);
				//LogV(TAG,"AddressLen =[%d]", pi->AddressLength);
				//LogV(TAG,"Type =[%d]", pi->Type);

				if (pi->Type == MIB_IF_TYPE_ETHERNET)
				{
					string desc = pi->Description;
					StringTool::MakeUpper(desc);

					//LogV(TAG,"eth desc=[%s]", desc.c_str());
					//int pos = desc.Find("pci");
					if (
						//pos != -1 && 
						pi->AddressLength == 6)
					{
						string addr;
						addr = StringTool::Format("%02X:%02X:%02X:%02X:%02X:%02X",
							pi->Address[0],
							pi->Address[1],
							pi->Address[2],
							pi->Address[3],
							pi->Address[4],
							pi->Address[5]
						);
						//return addr;
						memcpy(macAddr, pi->Address, 6);
						LPBYTE p = (LPBYTE)macAddr;
						string mac;
						mac = StringTool::Format("%02X:%02X:%02X:%02X:%02X:%02X", p[0], p[1], p[2], p[3], p[4], p[5]);
						return mac;
					}
				}

				if (!pi->Next)
				{
					break;
				}
			}

			return 0;
		}
		else
		{
			LogW(TAG,"fail GetAdaptersInfo");
		}
	}
#elif defined __APPLE__
	
#else
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd >= 0)
	{
		SockTool::CAutoClose ac(&fd);

		ifr.ifr_addr.sa_family = AF_INET;
		strncpy(ifr.ifr_name, eth.c_str(), sizeof(ifr.ifr_name) - 1);

		int ret = ioctl(fd, SIOCGIFHWADDR, &ifr);
		if (ret == 0)
		{
			auto p = (LPBYTE)ifr.ifr_hwaddr.sa_data;
			//display mac address
			string mac=StringTool::Format("%02X:%02X:%02X:%02X:%02X:%02X", p[0], p[1], p[2], p[3], p[4], p[5]);
			return mac;
		}
		else
		{
			LogW(TAG,"fail SIOCGIFHWADDR,error=%d(%s)", errno, strerror(errno));
		}
	}
	else
	{
		LogW(TAG,"fd=%d", fd);
	}
#endif

	return "";
}

#define BUFSIZE 2048
#ifndef IF_NAMESIZE
#define IF_NAMESIZE 16
#endif

string NetTool::GetGateWay(string ifName)
{
	string ack;
	string value;

	char szIfName[16] = { 0 };
	strncpy(szIfName, ifName.c_str(), sizeof(szIfName) - 1);

#if !defined _MSC_VER && !defined __APPLE__
	char buffer[200] = { 0 };
	auto bufLen = sizeof(buffer);

	unsigned long defaultRoutePara[4] = { 0 };
	FILE * pfd = fopen("/proc/net/route", "r");
	if (NULL == pfd)
	{
		return "";
	}

	File::CAutoClose ac(pfd);

	while (fgets(buffer, (int)bufLen, pfd))
	{
		sscanf(buffer, "%*s %x %x %x %*x %*x %*x %x %*x %*x %*x\n", (unsigned int *)&defaultRoutePara[1], (unsigned int *)&defaultRoutePara[0], (unsigned int *)&defaultRoutePara[3], (unsigned int *)&defaultRoutePara[2]);

		if (NULL != strstr(buffer, ifName.c_str()))
		{
			//如果FLAG标志中有 RTF_GATEWAY  
			if (defaultRoutePara[3] & RTF_GATEWAY)
			{
				unsigned long ip = defaultRoutePara[0];
				ack=StringTool::Format("%d.%d.%d.%d", (ip & 0xff), (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
				//LogV(TAG,"gateway=%s", ack.c_str());
				break;
			}
		}
	}

#else
	ack = "192.168.1.1";
#endif

	return ack;
}

#ifndef _MSC_VER
//include net/if.h时报错，有linux/if.h有冲突,所以这里直接引入
extern "C"
{
	struct if_nameindex {
		unsigned if_index;
		char* if_name;
	};

	struct if_nameindex* if_nameindex(void);
	void if_freenameindex(struct if_nameindex* __ptr);
};
#endif

string NetTool::GetInterfaceName()
{
	string name;
#if !defined _MSC_VER && !defined _CONFIG_ANDROID
	auto info = if_nameindex();
	if (info)
	{
		int i = 0;
		while (info[i].if_index != 0) {
			string v = info[i].if_name;
			//LogV(TAG,"[%s]\r\n",v.c_str());
			if (!v.empty() && v != "lo")
			{
				name = v;
			}

			i++;
		}

		if_freenameindex(info);
	}
#endif
	return name;
}
}
}
}
