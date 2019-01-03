#include "stdafx.h"
#include "Adapter.h"
#include <WinSock2.h>
#include <Iphlpapi.h>

#pragma comment(lib,"iphlpapi.lib")

int MyGetAdptersInfo(std::vector<MyAdpterInfo>& adpterInfo)
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
	{
		printf("Error allocating memory needed to call GetAdaptersinfo\n");
		return -1;
	}
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
			return -1;	//	error data return
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			MyAdpterInfo info;
			info.Name = std::string(pAdapter->AdapterName);
			info.Description = std::string(pAdapter->Description);
			info.Type = pAdapter->Type;
			char buffer[20];
			sprintf_s(buffer, "%.2x-%.2x-%.2x-%.2x-%.2x-%.2x", pAdapter->Address[0],
				pAdapter->Address[1], pAdapter->Address[2], pAdapter->Address[3],
				pAdapter->Address[4], pAdapter->Address[5]);
			info.MacAddress = std::string(buffer);
			IP_ADDR_STRING *pIpAddrString = &(pAdapter->IpAddressList);
			do
			{
				info.Ip.push_back(std::string(pIpAddrString->IpAddress.String));
				pIpAddrString = pIpAddrString->Next;
			} while (pIpAddrString);
			pAdapter = pAdapter->Next;
			adpterInfo.push_back(info);
		}
		if (pAdapterInfo)
			FREE(pAdapterInfo);
		return 0;	// normal return
	}
	else
	{
		if (pAdapterInfo)
			FREE(pAdapterInfo);
		printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
		return 1;	//	null data return
	}
}

std::string GetMacAddress()
{
	std::vector<MyAdpterInfo> AdptersInfo;
	int ret = MyGetAdptersInfo(AdptersInfo);
	if (ret != 0)
	{
		return std::string("");
	}
	for (size_t i = 0; i < AdptersInfo.size(); ++i)
	{
		std::string strDesc = AdptersInfo[i].Description;
		if (strDesc.find("Virtual") == std::string::npos && strDesc.find("VPN") == std::string::npos)
		{
			return AdptersInfo[i].MacAddress;
		}
	} 
	return std::string("");
}

