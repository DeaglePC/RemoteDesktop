#pragma once

#include <string>
#include <vector>

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

typedef struct _MyAdpterInfo
{
	std::vector<std::string> Ip;
	std::string MacAddress;
	std::string Description;
	std::string Name;
	unsigned int Type;
}MyAdpterInfo;

int MyGetAdptersInfo(std::vector<MyAdpterInfo>& adpterInfo);

std::string GetMacAddress();
