#include "stdafx.h"
#include "PublicString.h"

std::vector<std::string> SplitString(const std::string& str, const char& c)
{
	std::vector<std::string> res;
	std::string tmp("");
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == c)
		{
			if (tmp.size() > 0)
			{
				res.push_back(tmp);
			}
			tmp = "";
		}
		else
		{
			tmp += str[i];
		}
	}
	if (tmp.size() > 0)
	{
		res.push_back(tmp);
	}
	return res;
}

bool IsValidIp(const char *str)
{
	int cntPart = 0;

	for (int i = 0, len = strlen(str); i <= len && cntPart <= 4; i++, cntPart++)
	{
		int digCnt = 0;
		int part = 0;
		while (digCnt < 3 && str[i] >= '0' && str[i] <= '9')
		{
			part = part * 10 + str[i] - '0';
			digCnt++;
			i++;
		}
		if (digCnt == 0 || (str[i] != '.' && str[i] != '\0') || part > 255)
			return false;
	}

	return cntPart == 4 ? true : false;
}

bool IsValidId(const char* lpszData)
{
	if (strlen(lpszData) != 9)
	{
		return false;
	}
	for (int i = 0; lpszData[i]; ++i)
	{
		if (lpszData[i] < '0' || lpszData[i] > '9')
		{
			return false;
		}
	}
	return true;
}
