#pragma once
#include <WinSock2.h>
#include <list>

#pragma comment(lib,"ws2_32.lib")

#define PASSWORD "666"

// 发送图片时候的数据标识头  一张图片的发送包括  本标识头 + 结构体头  +  图像数据
#define HEAD_MARK "TEA"

#define MAX_SERVER_LISTEN 10

const size_t MAX_RECV_BUF = 1024 * 1024;
const size_t MAX_TCP_IMG_BUF = 1400;
const u_short SERVER_PORT = 17788;
const size_t PASSWORD_LEN = 10;

enum MSGTYPE
{
	MSGTYPE_CMD,	// 数据是命令,后接 INPUT 结构体
	MSGTYPE_IMG,	// 数据是图片
	MSGTYPE_LAN,	// 发起连接，后接密码
	MSGTYPE_WP,		// 密码错误
	MSGTYPE_OK,		// 密码正确
	MSGTYPE_BYE		// 客户端告诉服务端断开
};

typedef struct _LanMsgHeader
{
	unsigned int nPackageSize;	// 包的总大小 = 包头 + 数据
	unsigned int nHeaderSize;	// 包头大小
	unsigned int nDataSize;		// 数据大小
	unsigned int nImageSize;	// 图像总大小
	MSGTYPE MsgType;			// 本条数据是什么类型
}LanMsgHeader;


class CLanRemoteDesktopControl
{
public:
	CLanRemoteDesktopControl();
	virtual ~CLanRemoteDesktopControl();

	void WaitClient();
	void ClientRecvServer();
	void EndClientConnection();	// 客户端主动断开连接
	void EndServerConnection();	// 服务端主动断开连接
	void SnedImage();
	int SendCommand(const INPUT&);
	bool GetServerStatus();	// true 表示正在和客户端会话

	void ControlLanRemoteDesk(const char* lpszServerIp, const char* lpszPassword);

	bool GetClientIsEnd();
	BYTE GetJpegQuality();
	void SetJpegQuality(BYTE);
	void SetDrawHwnd(HWND hWnd);
	
	// 发送各种命令给服务端
	void SendKeyDown(WORD wVk);
	void SendKeyUp(WORD wVk);
	void SendMouseMove(DWORD dx, DWORD dy);
	void SendLeftButtonDown();
	void SendLeftButtonUp();
	void SendLeftButtonDoubleClick();
	void SendRightButtonDown();
	void SendRightButtonUp();
	void SendRightButtonDoubleClick();
	void SendMiddleButtonDown();
	void SendMiddleButtonUp();
	void SendMiddleButtonDoubleClick();
	void SendMouseWheel(DWORD mouseData);		// 鼠标滚轮滚动
private:
	char m_szPassword[PASSWORD_LEN];
	char m_szRecvBufServer[MAX_RECV_BUF];	// 服务端接收到的数据存这里
	char m_szRecvBufClient[MAX_RECV_BUF];	// 客户端的存这里
	BYTE m_nJpegQuality;
	int m_nRemoteScreenWidth;
	int m_nRemoteScreenHeight;
	int m_nDrawOriginX;
	int m_nDrawOriginY;

	// server
	WSADATA m_wsad;
	SOCKET m_socketServer;
	SOCKET m_socketCurrentConn;
	SOCKADDR_IN m_addrServer;
	std::list<SOCKADDR_IN> m_listAddrClients;
	bool m_bIsControlledByClient;

	bool m_bIsSocketReady;
	void InitSocket();
	void DestorySocket();
	void InitServer();			// 控制前后清空状态
	void StartWaitClient();
	int ProcessServerData(const char* lpszData, int nLen);
	int ProcessCommandData();
	void StartSendImage();
	void SplitSendImage(BYTE* pImgData, int nSize);

	// client
	SOCKET m_socketClient;
	bool m_bIsClientSocketReady;
	bool m_bIsEnd;	// 是否有结束信号
	bool m_bIsControling;

	BYTE m_byImageData[MAX_RECV_BUF];
	bool m_bIsStartRecvImage;
	size_t m_nCurrentSize;
	size_t m_nImageSize;
	HWND m_hWndDraw;

	void InitClientSocket();
	void DestoryClientSocket();
	int ClientConnectServer(const char* lpszServerIp, u_short nServerPort);
	void StartRecvServerMsg();
	int ProcessClientData(const char* lpszData, int nLen);
	void ProcessImageData();		// 处理图像数据
	void DrawImageToWindow();
	bool GetIsControling();
};

