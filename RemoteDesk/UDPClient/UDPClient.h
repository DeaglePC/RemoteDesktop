// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 UDPCLIENT_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// UDPCLIENT_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef UDPCLIENT_EXPORTS
#define UDPCLIENT_API __declspec(dllexport)
#else
#define UDPCLIENT_API __declspec(dllimport)
#endif

#include <WinSock2.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")

#define BUF_LEN 548

typedef void(*PTR_ON_RECV_FUN)(const char*);


// 此类是从 UDPClient.dll 导出的
class UDPCLIENT_API CUDPClient {
public:

	CUDPClient(const char* lpszServerIP, const u_short& unServerPort, PTR_ON_RECV_FUN pOnRevcFun);
	CUDPClient(const char* lpszServerIP, const u_short& unServerPort, void* pOnRevcFun);
	virtual ~CUDPClient();
	// TODO:  在此添加您的方法。
	void SetServerIP(const char* lpszServerIP);
	void SetServerPort(const u_short& unServerPort);
	BOOL StartRecv();
	int OnRecv();
	int SendToServer(const char* lpszSendBuf);
	
private:
	char m_szRecvBuf[BUF_LEN];
	PTR_ON_RECV_FUN m_pOnRevcFun;

	WSADATA m_wsad;
	SOCKET m_socketClient;
	SOCKADDR_IN m_addrServer;

	BOOL m_bIsReady;
	BOOL m_bIsStart;

protected:
};
