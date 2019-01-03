// UDPClient.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "UDPClient.h"


// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 UDPClient.h
CUDPClient::CUDPClient(const char* lpszServerIP, const u_short& unServerPort, PTR_ON_RECV_FUN pOnRevcFun)
{
	m_bIsReady = FALSE;
	m_bIsStart = FALSE;
	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));

	int ret;
	ret = WSAStartup(MAKEWORD(2, 2), &m_wsad);
	if (ret != 0)
	{
		WSACleanup();
		return;
	}
	m_socketClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socketClient == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}
	m_addrServer.sin_addr.S_un.S_addr = inet_addr(lpszServerIP);
	m_addrServer.sin_family = AF_INET;
	m_addrServer.sin_port = htons(unServerPort);

	m_pOnRevcFun = pOnRevcFun;
	m_bIsReady = TRUE;
}

CUDPClient::CUDPClient(const char* lpszServerIP, const u_short& unServerPort, void* pOnRevcFun)
{
	m_bIsReady = FALSE;
	m_bIsStart = FALSE;
	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));

	int ret;
	ret = WSAStartup(MAKEWORD(2, 2), &m_wsad);
	if (ret != 0)
	{
		WSACleanup();
		return;
	}
	m_socketClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socketClient == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}
	m_addrServer.sin_addr.S_un.S_addr = inet_addr(lpszServerIP);
	m_addrServer.sin_family = AF_INET;
	m_addrServer.sin_port = htons(unServerPort);

	m_pOnRevcFun = (PTR_ON_RECV_FUN)pOnRevcFun;
	m_bIsReady = TRUE;
}

CUDPClient::~CUDPClient()
{
	if (m_bIsReady)
	{
		WSACleanup();
		closesocket(m_socketClient);
		_endthread();
	}
}

void CUDPClient::SetServerIP(const char* lpszServerIP)
{
	m_addrServer.sin_addr.S_un.S_addr = inet_addr(lpszServerIP);
}

void CUDPClient::SetServerPort(const u_short& unServerPort)
{
	m_addrServer.sin_port = htons(unServerPort);
}

void ThreadFun(void* p)
{
	CUDPClient* pUDPClient = (CUDPClient*)p;
	if (pUDPClient == NULL)
	{
		return;
	}
	while (true)
	{
		pUDPClient->OnRecv();
		Sleep(10);
	}
}

BOOL CUDPClient::StartRecv()
{
	if (m_bIsReady && !m_bIsStart)
	{
		_beginthread(ThreadFun, 0, this);
		m_bIsStart = TRUE;
		return TRUE;
	}
	return FALSE;
}

int CUDPClient::OnRecv()
{
	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
	int nLen = sizeof(SOCKADDR_IN);
	int ret = recvfrom(m_socketClient, m_szRecvBuf, sizeof(m_szRecvBuf), 0, (SOCKADDR*)&m_addrServer, &nLen);
	if (ret != SOCKET_ERROR)
	{
		m_pOnRevcFun(m_szRecvBuf);
	}
	return ret;
}

int CUDPClient::SendToServer(const char* lpszSendBuf)
{
	int len = strlen(lpszSendBuf);
	return sendto(m_socketClient, lpszSendBuf,len, 0, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
}

