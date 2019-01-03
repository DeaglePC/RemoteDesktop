#pragma once
#include "../UDPClient/UDPClient.h"
#include "../UDPServer/UDPServer.h"
#include "ImageDataFormat.h"

#pragma comment(lib,"../Debug/UDPClient.lib")
#pragma comment(lib,"../Debug/UDPServer.lib")

#define CONTROL_DATA_HEART		"HEART"
#define CONTROL_DATA_ONLINE		"DPC"
#define CONTROL_DATA_BYE		"BYE"
#define CONTROL_DATA_CONTROL	"C"
#define CONTROL_DATA_WRONG_PW	"WP"
#define CONTROL_DATA_GO			"GO"
#define CONTROL_DATA_ID			"ID"
#define CONTROL_DATA_OK			"OK"
#define CONTROL_DATA_IMG		"IMG"
#define CONTROL_DATA_CMD		"CMD"
#define CONTROL_DATA_LAN		"LAN"

#define HEARTBEAT_INTERVAL 3000

#define MAX_BUF_LEN 1250

#define LAN_PORT 11223


class CRemoteDeskControl
{
public:
	CRemoteDeskControl(const char* lpszServerIP, const u_short& uServerPort);
	virtual ~CRemoteDeskControl();

	void SendHeartbeatData();

	int GetId();
	char* GetPassword();

	int OnRecv();
	int SendOnlineData();
	void ControlRemoteClient(const int& m_nId, const char* lpszPassword);
	void SendImage();
	void SetDrawTargetWindow(HWND hWnd);
	void EndControl();

	bool GetIsEnd()const;
	bool GetIsOk()const;
	bool GetIsEndRecvServer()const;

	void SetJpegQuality(BYTE q);
	BYTE GetJpegQuality()const;
private:
	char m_szServerIp[20];
	u_short m_nServerPort;
	char m_szTargetIp[20];
	u_short m_nTargetPort;

	char m_szMac[20];		// 本机MAC地址，如 1e-30-7d-96-1e-2d
	int m_nId;				// ID 如132456789
	char m_szPassword[8];	// 密码六位数
	bool m_bIsReady;		// 是否准备就绪，看是否get到了ID
	bool m_bIsControler;	// 是不是控制者

	bool m_bIsGo;			// 是否已经收到了go，是否在准备会话阶段
	bool m_bIsOk;			// 是否可对方客户端联通了网络，联通后才开始传输数据
	bool m_bIsEnd;			// 控制结束，将不再发送图像等数据
	bool m_bIsEndRecvServer;// 是否继续接收服务端的消息

	HWND m_hWnd;			// 要画在这个窗口上

	WSADATA m_wsad;
	SOCKET m_socketClient;
	SOCKADDR_IN m_addrServer;
	//SOCKADDR_IN m_addrTarget;
	bool m_bIsSocketOk;
	bool m_bIsStartRecv;
	char m_szRecvBuf[MAX_BUF_LEN];
	char m_szSendTmpBuf[MAX_BUF_LEN];

	BYTE m_byJpegQuality;						// jpeg格式图片的质量，0-100
	char m_szImageDataBuf[MAX_IMG_BUFFER_SIZE];	// 暂存图像数据
	size_t m_nCurrentImageSize;					// 当前已经保存了多大的图像
	size_t m_nCurrentIndex;

	unsigned int m_nImageCount;
	bool m_bIsGetAck;							// 是否收到了确认信号

	void InitFlag();							// 每次会话都要初始化的变量
	void InitSocket(const char* lpszServerIP, 
		const u_short& unServerPort);			// 初始化socket

	void StartRecvMsg();						// 开启接受消息的线程
	void OnRecvMsg(const char*lpszData);		// 处理收到的消息
	bool SendP2PMsg();							// 不停的发送
	void StartSendImage();						// 开始发送桌面图像
	void SplitSendImage(BYTE* pImgData, int nSize);
	void ProcessImageData();
	void ProcessCommandData();
	void SaveImageData(const char *lpszData);	// 保存图像数据到缓存
	void DrawImageToWindow(size_t nSize);		// 画接收到的图像到窗口


	CUDPClient* m_pUdpHeartbeat;
	void StartHeartbeat();
};
