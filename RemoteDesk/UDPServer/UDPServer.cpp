// UDPServer.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "UDPServer.h"


// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 UDPServer.h
CUDPServer::CUDPServer(USHORT unPort, PTR_ON_RECV_FUN pOnRevcFun)
{
	m_bIsReady = FALSE;
	m_bIsStart = FALSE;

	int ret;
	ret = WSAStartup(MAKEWORD(2, 2), &m_wsad);
	if (ret != 0)
	{
		WSACleanup();
		return;
	}
	m_socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_socketServer == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}

	m_addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	m_addrServer.sin_family = AF_INET;
	m_addrServer.sin_port = htons(unPort);
	ret = bind(m_socketServer, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
	if (ret == SOCKET_ERROR)
	{
		closesocket(m_socketServer);
		WSACleanup();
		return;
	}

	m_pOnRevcFun = pOnRevcFun;
	m_bIsReady = TRUE;
}

CUDPServer::~CUDPServer()
{
	if (m_bIsReady)
	{
		closesocket(m_socketServer);
		WSACleanup();
		_endthread();
	}
}

void ThreadFun(void* p)
{
	CUDPServer* pUDPClient = (CUDPServer*)p;
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

BOOL CUDPServer::StartRecv()
{
	if (m_bIsReady && !m_bIsStart)
	{
		_beginthread(ThreadFun, 0, this);
		m_bIsStart = TRUE;
		return TRUE;
	}
	return FALSE;
}

int CUDPServer::OnRecv()
{
	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
	int nLen = sizeof(SOCKADDR_IN);
	int ret = recvfrom(m_socketServer, m_szRecvBuf, sizeof(m_szRecvBuf), 0, (SOCKADDR*)&m_addrClient, &nLen);
	if (ret != SOCKET_ERROR)
	{
		m_pOnRevcFun(m_szRecvBuf);
	}
	return ret;
}

int CUDPServer::SendToClient(const char* lpszSendBuf)
{
	return sendto(m_socketServer, lpszSendBuf, strlen(lpszSendBuf), 0, (SOCKADDR*)&m_addrClient, sizeof(SOCKADDR));
}
