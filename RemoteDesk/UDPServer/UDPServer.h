// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 UDPSERVER_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// UDPSERVER_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef UDPSERVER_EXPORTS
#define UDPSERVER_API __declspec(dllexport)
#else
#define UDPSERVER_API __declspec(dllimport)
#endif

#include <WinSock2.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")

#define BUF_LEN 548

typedef void(*PTR_THREAD_FUN)(void*);
typedef VOID(*PTR_THREAD_FUN)(PVOID);
typedef void(*PTR_ON_RECV_FUN)(const char*);

// 此类是从 UDPServer.dll 导出的
class UDPSERVER_API CUDPServer {
public:
	CUDPServer(USHORT unPort, PTR_ON_RECV_FUN pOnRevcFun);
	~CUDPServer();
	// TODO:  在此添加您的方法。
	BOOL StartRecv();
	int OnRecv();
	int SendToClient(const char* lpszSendBuf);


private:
	char m_szRecvBuf[BUF_LEN];
	PTR_ON_RECV_FUN m_pOnRevcFun;

	WSADATA m_wsad;
	SOCKET m_socketServer;
	SOCKADDR_IN m_addrServer;
	SOCKADDR_IN m_addrClient;

	BOOL m_bIsReady;
	BOOL m_bIsStart;
};
