#include "stdafx.h"
#include "RemoteDeskControl.h"
#include "Adapter.h"
#include "PublicString.h"
#include "MyBitmap.h"
#include "CxImage/include/ximage.h"

#pragma comment(lib,"CxImage/lib/cximage.lib")

//#ifdef _DEBUG
//#ifdef _UNICODE
//#pragma comment(lib,"CxImage/lib/cximagedu.lib")
//#else
//#pragma comment(lib,"CxImage/lib/cximaged.lib")
//#endif // _UNICODE
//#else
//#ifdef _UNICODE
//#pragma comment(lib,"CxImage/lib/cximageu.lib")
//#else
//#pragma comment(lib,"CxImage/lib/cximage.lib")
//#endif // _UNICODE
//#endif	// _DEBUG


CRemoteDeskControl::CRemoteDeskControl(const char* lpszServerIP, const u_short& uServerPort)
{
	this->m_nId = 0;
	this->m_bIsControler = false;	//	默认是被人控制的
	memset(this->m_szPassword, 0, sizeof(this->m_szPassword));
	this->m_bIsReady = false;
	this->m_byJpegQuality = 30;
	this->m_nCurrentImageSize = 0;
	this->m_nCurrentIndex = 0;
	this->m_bIsEnd = false;
	this->m_bIsEndRecvServer = true;

	InitFlag();

	memset(m_szTargetIp, 0, sizeof(m_szTargetIp));
	m_nTargetPort = 0;

	strcpy_s(this->m_szServerIp, lpszServerIP);
	this->m_nServerPort = uServerPort;
	InitSocket(lpszServerIP, uServerPort);

	std::string strMac = GetMacAddress();
	strcpy_s(m_szMac, strMac.c_str());

	StartRecvMsg();
	SendOnlineData();
	StartHeartbeat();
}



CRemoteDeskControl::~CRemoteDeskControl()
{
	if (m_bIsSocketOk)
	{
		WSACleanup();
		closesocket(m_socketClient);
	}
}

void CRemoteDeskControl::SendHeartbeatData()
{
	char bufMsg[20];
	sprintf_s(bufMsg, "#%s#%d", CONTROL_DATA_HEART, this->m_nId);
	this->m_pUdpHeartbeat->SendToServer(bufMsg);
}

int CRemoteDeskControl::GetId()
{
	return this->m_nId;
}

char* CRemoteDeskControl::GetPassword()
{
	return this->m_szPassword;
}

// 发送约定好消息格式的表示上线的信息
int CRemoteDeskControl::SendOnlineData()
{
	char buf[30];
	sprintf_s(buf, "#%s#%s", CONTROL_DATA_ONLINE, m_szMac);
	return sendto(m_socketClient, buf, strlen(buf), 0, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
}

void CRemoteDeskControl::ControlRemoteClient(const int& m_nId, const char* lpszPassword)
{
	char bufMsg[30];
	sprintf_s(bufMsg, "#%s#%d#%d#%s", CONTROL_DATA_CONTROL, this->m_nId, m_nId, lpszPassword);
	sendto(m_socketClient, bufMsg, strlen(bufMsg), 0, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
	this->m_bIsControler = true;
}

void CRemoteDeskControl::SetDrawTargetWindow(HWND hWnd)
{
	this->m_hWnd = hWnd;
}

void CRemoteDeskControl::EndControl()
{
	char szBufMsg[5];
	sprintf_s(szBufMsg, "#%s", CONTROL_DATA_BYE);
	sendto(m_socketClient, szBufMsg, strlen(szBufMsg), 0, (SOCKADDR*)&m_addrServer, sizeof(SOCKADDR));
	this->m_bIsEnd = true;
	this->m_bIsGo = false;
	this->m_bIsControler = false;
}

bool CRemoteDeskControl::GetIsEnd()const
{
	return this->m_bIsEnd;
}

bool CRemoteDeskControl::GetIsOk()const
{
	return m_bIsOk;
}

bool CRemoteDeskControl::GetIsEndRecvServer() const
{
	return this->m_bIsEndRecvServer;
}

void CRemoteDeskControl::SetJpegQuality(BYTE q)
{
	if (q >=0 && q <= 100)
	{
		this->m_byJpegQuality = q;
	}
}

BYTE CRemoteDeskControl::GetJpegQuality() const
{
	return this->m_byJpegQuality;
}

void CRemoteDeskControl::InitFlag()
{
	this->m_bIsGo = false;
	this->m_bIsEnd = false;
	this->m_bIsOk = false;
}

void CRemoteDeskControl::InitSocket(const char* lpszServerIP, const u_short& uServerPort)
{
	m_bIsSocketOk = false;
	m_bIsSocketOk = false;
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
	m_addrServer.sin_port = htons(uServerPort);

	// 改变为非阻塞模式
	unsigned long nMode = 1;
	ret = ioctlsocket(m_socketClient, FIONBIO, &nMode);
	if (ret == SOCKET_ERROR)
	{
		WSACleanup();
		closesocket(m_socketClient);
	}

	m_bIsSocketOk = true;
}

// 接收到消息送给另一个成员函数处理
int CRemoteDeskControl::OnRecv()
{
	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
	int nLen = sizeof(SOCKADDR_IN);
	int ret = recvfrom(m_socketClient, m_szRecvBuf, sizeof(m_szRecvBuf), 0, (SOCKADDR*)&m_addrServer, &nLen);
	if (ret != SOCKET_ERROR)
	{
		OnRecvMsg(m_szRecvBuf);
	}
	Sleep(20);
	return ret;
}

// 线程函数，用于不断调用接收消息的函数
void ThreadFuncRecvMsg(void *p)
{
	CRemoteDeskControl* pRdc = (CRemoteDeskControl*)p;
	if (pRdc == NULL)
	{
		return;
	}
	while (true)
	{
		pRdc->OnRecv();
	}
}

void CRemoteDeskControl::StartRecvMsg()
{
	if (m_bIsSocketOk && !m_bIsStartRecv)
	{
		_beginthread(ThreadFuncRecvMsg, 0, this);
		m_bIsStartRecv = true;
	}
}

// 最终数据在这里处理
void CRemoteDeskControl::OnRecvMsg(const char* lpszData)
{
	std::string strData(lpszData);
	std::vector<std::string> segs = SplitString(strData, '#');
	if (segs[0] == CONTROL_DATA_ID && segs.size() == 3)
	{
		// 接收到了ID信息
		this->m_nId = atoi(segs[1].c_str());
		strcpy_s(this->m_szPassword, segs[2].c_str());
	}
	else if (segs[0] == CONTROL_DATA_GO && segs.size() == 3)
	{
		// 准备控制或者被控制
		this->m_bIsGo = true;
		this->m_bIsEnd = false;
		this->m_bIsGetAck = false;
		strcpy_s(this->m_szTargetIp, segs[1].c_str());
		this->m_nTargetPort = atoi(segs[2].c_str());

		bool res = SendP2PMsg();
		if (res)
		{
			// 进入控制模式下的接收数据操作
			if (m_bIsControler)
			{
				// 一直接收图像数据了
				ProcessImageData();
			}
			else
			{
				// 开发送图像数据的线程，接收命令数据
				this->m_nImageCount = 0;
				StartSendImage();
				ProcessCommandData();
			}
		}
	}
	else if(segs[0] == CONTROL_DATA_BYE && segs.size() == 1)
	{
		// 控制结束
		this->EndControl();
	}
	else if (segs[0] == CONTROL_DATA_WRONG_PW && segs.size() == 1)
	{
		// 密码错误
		this->EndControl();
		MessageBoxA(NULL, "密码错误", "提示", MB_OK);
	}
}

bool CRemoteDeskControl::SendP2PMsg()
{
	// 发送 OK
	SOCKADDR_IN addrTarget;
	addrTarget.sin_addr.S_un.S_addr = inet_addr(m_szTargetIp);
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(m_nTargetPort);
	char szBuf[5] = "#";
	strcat_s(szBuf, CONTROL_DATA_OK);

	char szRecvBuf[sizeof(CONTROL_DATA_OK) + 2];
	int nLen = sizeof(SOCKADDR_IN);
	int nCnt = 0;

	while (nCnt++ < 100)
	{
		int nSend = sendto(m_socketClient, szBuf, strlen(szBuf) + 1, 0, (SOCKADDR*)&addrTarget, sizeof(SOCKADDR));
		int ret = recvfrom(m_socketClient, szRecvBuf, sizeof(szRecvBuf), 0, (SOCKADDR*)&addrTarget, &nLen);
		if (ret != SOCKET_ERROR)
		{
			std::string strData(szRecvBuf);
			std::vector<std::string> segs = SplitString(strData, '#');
			if (segs[0] == CONTROL_DATA_OK && segs.size() == 1)
			{
				MessageBoxA(NULL, "打洞成功", "t", MB_OK);
				return true;
			}
		}
		Sleep(100);
	}
	MessageBoxA(NULL, "打洞失败", "t", MB_OK);
	return false;
}


void ThreadFuncSendImage(void *p)
{
	CRemoteDeskControl* pRdc = (CRemoteDeskControl*)p;
	if (pRdc == NULL)
	{
		return;
	}
	while (pRdc->GetIsEnd() == false)
	{
		pRdc->SendImage();
	}
}

void CRemoteDeskControl::StartSendImage()
{
	_beginthread(ThreadFuncSendImage, 0, this);
}

void CRemoteDeskControl::SplitSendImage(BYTE* pImgData, int nSize)
{
	m_nImageCount = m_nImageCount == MAXINT32 ? 0 : m_nImageCount + 1;
	size_t nPackageNum = nSize / MAX_IAMGE_DATA_SIZE;
	size_t nLastDataSize = nSize % MAX_IAMGE_DATA_SIZE;
	if (nLastDataSize != 0)
	{
		nPackageNum++;
	}

	SOCKADDR_IN addrTarget;
	addrTarget.sin_addr.S_un.S_addr = inet_addr(m_szTargetIp);
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(m_nTargetPort);
	int nLen = sizeof(SOCKADDR_IN);

	const int strLen = strlen(CONTROL_DATA_IMG);
	char szBuf[MAX_IAMGE_DATA_SIZE + sizeof(ImagePackageHeader) + 5];
	ImagePackageHeader imgHeader = { 0 };
	imgHeader.nImageSize = nSize;
	imgHeader.nDataSize = MAX_IAMGE_DATA_SIZE;
	imgHeader.nPackageNum = nPackageNum;
	imgHeader.nHeaderSize = sizeof(ImagePackageHeader);
	imgHeader.nImageId = m_nImageCount;
	memcpy_s(szBuf, sizeof(szBuf), CONTROL_DATA_IMG, strLen);
	for (size_t i = 0; i < nPackageNum; ++i)
	{
		if (nLastDataSize != 0 && i == nPackageNum - 1)
		{
			imgHeader.nDataSize = nLastDataSize;
		}
		imgHeader.nDataOffset = i * MAX_IAMGE_DATA_SIZE;
		imgHeader.nPackageIndex = i;

		memcpy_s(szBuf + strLen, sizeof(szBuf), &imgHeader, sizeof(imgHeader));
		memcpy_s(szBuf + strLen + sizeof(imgHeader), sizeof(szBuf), 
			pImgData + imgHeader.nDataOffset, imgHeader.nDataSize);

		sendto(m_socketClient, szBuf, strLen + sizeof(imgHeader) + imgHeader.nDataSize, 
			0, (SOCKADDR*)&addrTarget, sizeof(SOCKADDR));
		sendto(m_socketClient, szBuf, strLen + sizeof(imgHeader) + imgHeader.nDataSize,
			0, (SOCKADDR*)&addrTarget, sizeof(SOCKADDR));
	}
}

void CRemoteDeskControl::ProcessImageData()
{
	SOCKADDR_IN addrTarget;
	addrTarget.sin_addr.S_un.S_addr = inet_addr(m_szTargetIp);
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(m_nTargetPort);
	int nLen = sizeof(SOCKADDR_IN);
	char szRecvBuf[MAX_BUF_LEN];
	const int strLen = strlen(CONTROL_DATA_IMG);
	while (this->GetIsEnd() == false)
	{
		int ret = recvfrom(m_socketClient, szRecvBuf, sizeof(szRecvBuf), 0, (SOCKADDR*)&addrTarget, &nLen);
		char szTmp[sizeof(CONTROL_DATA_IMG)];
		memcpy_s(szTmp, sizeof(szTmp), szRecvBuf, strLen);
		szTmp[strLen] = '\0';
		if (strcmp(szTmp, CONTROL_DATA_IMG) == 0)
		{
			
		}
	}
}

void CRemoteDeskControl::ProcessCommandData()
{
	while (this->GetIsEnd() == false)
	{

	}
}

void CRemoteDeskControl::SendImage()
{
	// 对桌面进行截图
	HBITMAP hBitmapDesk = CMyBitmap::GetDesktopBitMap();
	CxImage pxImage;
	pxImage.CreateFromHBITMAP(hBitmapDesk);

	// 转为jpeg
	BYTE* pDataBuffer = NULL;
	long nSize = 0;
	pxImage.SetJpegQuality(this->GetJpegQuality());
	pxImage.Encode(pDataBuffer, nSize, CXIMAGE_FORMAT_JPG);

	SplitSendImage(pDataBuffer, nSize);

	pxImage.FreeMemory(pDataBuffer);
}

void CRemoteDeskControl::SaveImageData(const char *lpszData)
{
	ImagePackageHeader pkgHeader;
	size_t nHeaderSize = sizeof(ImagePackageHeader);
	size_t nLen = strlen(CONTROL_DATA_IMG) + 1;		// + #

	memcpy_s(&pkgHeader, nHeaderSize, lpszData + nLen, nHeaderSize);
	
	if (pkgHeader.nPackageIndex == m_nCurrentIndex)
	{
		this->m_nCurrentIndex++;
		this->m_nCurrentImageSize += pkgHeader.nDataSize;

		memcpy_s(this->m_szImageDataBuf + pkgHeader.nDataOffset, MAX_IMG_BUFFER_SIZE,
			lpszData + nLen + nHeaderSize, pkgHeader.nDataSize);
		if (this->m_nCurrentImageSize == pkgHeader.nImageSize && m_nCurrentIndex + 1 == pkgHeader.nPackageNum)
		{
			this->DrawImageToWindow(this->m_nCurrentImageSize);
			this->m_nCurrentIndex = 0;
			this->m_nCurrentImageSize = 0;
		}
	}
	else
	{
		this->m_nCurrentIndex = 0;
		this->m_nCurrentImageSize = 0;
	}
}

void CRemoteDeskControl::DrawImageToWindow(size_t nSize)
{
	CxImage *px = new CxImage((BYTE*)this->m_szImageDataBuf, nSize, CXIMAGE_FORMAT_JPG);

	HDC hdc = GetDC(m_hWnd);
	HDC hMemDc = CreateCompatibleDC(hdc);
	HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, px->GetWidth(), px->GetHeight());
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDc, hMemBmp);
	px->Draw(hMemDc);

	BitBlt(hdc, 0, 0, px->GetWidth(), px->GetHeight(), hMemDc, 0, 0, SRCCOPY);
	SelectObject(hMemDc, hOldBmp);

	DeleteObject(hMemBmp);
	DeleteObject(hOldBmp);
	DeleteDC(hMemDc);
	delete px;
}


// heartbeat
void ThreadFuncHeartMsg(const char * lpszData)
{
}

void SendHeartbeatDataThreadFun(void* ptr)
{
	CRemoteDeskControl* pRdc = (CRemoteDeskControl*)ptr;
	while (true)
	{
		pRdc->SendHeartbeatData();
		Sleep(HEARTBEAT_INTERVAL);
	}
}

void CRemoteDeskControl::StartHeartbeat()
{
	this->m_pUdpHeartbeat = new CUDPClient(this->m_szServerIp, this->m_nServerPort,ThreadFuncHeartMsg);
	_beginthread(SendHeartbeatDataThreadFun, 0, this);
}
