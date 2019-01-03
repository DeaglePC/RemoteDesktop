// RemoteDesk.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include "RemoteDesktop.h"
#include "RemoteDeskControl.h"
#include <CommCtrl.h>
#include "LanRemoteDesktopControl.h"
#include "PublicString.h"

#define MAX_LOADSTRING 100
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

#define ID_EDIT_ID	2000
#define ID_EDIT_PW	2001
#define ID_BTN_OK	2002

#define COLOR_EDIT RGB(18, 32, 61)
#define COLOR_BTN RGB(255,102,0)


// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

HWND g_hWnd;
HWND g_hRemoteWnd;
HWND g_hEditId;
HWND g_hEditPw;
HWND g_hBtnOk;

TCHAR szRemoteWindowClass[MAX_LOADSTRING] = L"MyRemoteWindow";
CRemoteDeskControl* g_pRemoteControl = NULL;
char g_szPassword[8] = { 0 };
int g_nId = 0;
bool g_bIsLan = false;
bool g_bIsControling = false;


CLanRemoteDesktopControl* g_pLanControl = NULL;

// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

// 自定义函数声明
ATOM				MyRegisterRemoteWindowClass(HINSTANCE hInstance);
LRESULT CALLBACK	RemoteWndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO:  在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_REMOTEDESK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	MyRegisterRemoteWindowClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REMOTEDESK));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REMOTEDESK));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCE(IDC_REMOTEDESK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


ATOM MyRegisterRemoteWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = RemoteWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REMOTEDESK));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCE(IDC_REMOTEDESK);
	wcex.lpszClassName = szRemoteWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW^WS_THICKFRAME^WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

VOID InitRemoteControl()
{
	//g_pRemoteControl = new CRemoteDeskControl("119.29.160.95", 18888);
	g_pRemoteControl = new CRemoteDeskControl("192.168.226.1", 18888);
	g_pLanControl = new CLanRemoteDesktopControl();
	g_pLanControl->SetJpegQuality(70);
}

VOID DrawUserInfo()
{
	HDC hDc = GetDC(g_hWnd);
	SetBkMode(hDc, TRANSPARENT);
	SetTextColor(hDc, RGB(0, 0, 0));
	if (g_nId != 0)
	{
		char Buf[20];
		_itoa_s(g_nId, Buf, 10);
		SetTextColor(hDc, RGB(255, 255, 255));
		TextOutA(hDc, 10, 10, Buf, strlen(Buf));
		TextOutA(hDc, 10, 30, g_szPassword, strlen(g_szPassword));
	}
	else
	{
		// TextOutW(hDc, 10, 10, L"请检查你的网络", 7);
	}
}

void ThreadFuncGetUserInfo(void *)
{
	while (true)
	{
		if (g_nId != 0)
		{
			break;
		}
		g_pRemoteControl->SendOnlineData();
		g_nId = g_pRemoteControl->GetId();
		strcpy_s(g_szPassword, g_pRemoteControl->GetPassword());
		Sleep(100);
	}
	DrawUserInfo();
}

// 初始化控件
void InitControl()
{
	DWORD dwStyle = ES_LEFT  | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER;
	DWORD dwExStyle = WS_TABSTOP;
	g_hEditId = ::CreateWindowEx(
		dwExStyle,			//dwExStyle 扩展样式
		L"Edit",			//lpClassName 窗口类名
		NULL,				//lpWindowName 窗口标题
		dwStyle,			//dwStyle 窗口样式
		199,				//x 左边位置
		160,				//y 顶边位置
		200,				//nWidth 宽度
		25,					//nHeight 高度
		g_hWnd,				//hWndParent 父窗口句柄
		(HMENU)ID_EDIT_ID,	//hMenu 菜单句柄
		NULL,				//hInstance 应用程序句柄
		NULL				//lpParam 附加参数
		);
	dwStyle = ES_LEFT | ES_PASSWORD | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER;
	g_hEditPw = ::CreateWindowEx(
		dwExStyle,			//dwExStyle 扩展样式
		L"Edit",			//lpClassName 窗口类名
		NULL,				//lpWindowName 窗口标题
		dwStyle,			//dwStyle 窗口样式
		199,				//x 左边位置
		200,				//y 顶边位置
		200,				//nWidth 宽度
		25,					//nHeight 高度
		g_hWnd,				//hWndParent 父窗口句柄
		(HMENU)ID_EDIT_PW,	//hMenu 菜单句柄
		NULL,				//hInstance 应用程序句柄
		NULL				//lpParam 附加参数
	);
	dwStyle = BS_PUSHBUTTON | BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE;
	dwExStyle = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
	TCHAR szWindowName[MAX_LOADSTRING] = L"确定";
	g_hBtnOk = ::CreateWindowEx(
		dwExStyle,			//dwExStyle 扩展样式
		L"Button",			//lpClassName 窗口类名
		szWindowName,		//lpWindowName 窗口标题
		dwStyle,			//dwStyle 窗口样式
		239,				//x 左边位置
		250,				//y 顶边位置
		120,				//nWidth 宽度
		25,					//nHeight 高度
		g_hWnd,				//hWndParent 父窗口句柄
		(HMENU)ID_BTN_OK,	//hMenu 菜单句柄
		NULL,				//hInstance 应用程序句柄
		NULL				//lpParam 附加参数
		);
	SetWindowLong(g_hBtnOk, GWL_STYLE, GetWindowLong(g_hBtnOk, GWL_STYLE) | BS_OWNERDRAW | BS_FLAT);

	SetWindowTextA(g_hEditId, "192.168.226.133");
	SetWindowTextA(g_hEditPw, "666");
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		g_hWnd = hWnd;
		InitControl();
		InitRemoteControl();
		_beginthread(ThreadFuncGetUserInfo, 0, NULL);
		break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  在此添加任意绘图代码...
		HBITMAP hBitMap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		HDC hMemDc = CreateCompatibleDC(hdc);
		SelectObject(hMemDc, hBitMap);
		BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDc, 0, 0, SRCCOPY);
		DeleteDC(hMemDc);
		DeleteObject(hBitMap);
		//DrawUserInfo();
		EndPaint(hWnd, &ps);
	}
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_BTN_OK:
			if (g_bIsControling)
			{
				break;
			}
			char szBufId[30], szBufPw[8];
			GetWindowTextA(g_hEditId, szBufId, 30);
			GetWindowTextA(g_hEditPw, szBufPw, 7);
			if (strlen(szBufPw) == 0 || strlen(szBufId) == 0)
			{
				MessageBox(hWnd, L"输入不合法", L"提示", MB_OK);
				break;
			}
			if (IsValidIp(szBufId))
			{
				g_bIsLan = true;
				g_hRemoteWnd = CreateWindow(szRemoteWindowClass, L"远程桌面",
					WS_OVERLAPPEDWINDOW, 500, 500, 500, 500, NULL, NULL, NULL, 0);
				g_pLanControl->SetDrawHwnd(g_hRemoteWnd);
				g_pLanControl->ControlLanRemoteDesk(szBufId, szBufPw);
				g_bIsControling = true;
			}
			else if (IsValidId(szBufId))
			{
				g_bIsLan = false;
				g_hRemoteWnd = CreateWindow(szRemoteWindowClass, L"远程桌面",
					WS_OVERLAPPEDWINDOW, 500, 500, 500, 500, NULL, NULL, NULL, 0);
				g_pRemoteControl->SetDrawTargetWindow(g_hRemoteWnd);
				g_pRemoteControl->ControlRemoteClient(atoi(szBufId), szBufPw);
				g_bIsControling = true;
				
				/*g_pLanControl->SetDrawHwnd(g_hRemoteWnd);
				g_pLanControl->ControlLanRemoteDesk("192.168.226.133", "666");*/
			}
			else
			{
				MessageBox(hWnd, L"输入不合法", L"提示", MB_OK);
				break;
				break;
			}
			//g_pRemoteControl->ControlRemoteClient(atoi(szBufId), szBufPw);
			break;
		default:
			break;
		}
		break;

	case WM_CTLCOLOREDIT:
	{
		HWND hWndParam = (HWND)lParam;
		if (hWndParam == g_hEditId || hWndParam == g_hEditPw)
		{
			HDC hdc = (HDC)wParam;
			HBRUSH hbrush;
			/* 创建画刷 */
			hbrush = CreateSolidBrush(COLOR_EDIT);
			SetBkColor(hdc, COLOR_EDIT);
			SetTextColor(hdc, RGB(255, 255, 255));
			/*
			** 处理WM_CTLCOLOREDIT消息必须返回一个画刷
			** windows使用这个画刷绘制edit控件的背景
			** 注意这个背景和文字背景颜色不同
			*/
			return (LRESULT)hbrush;
		}
	}
		break;
	case WM_CTLCOLORBTN:
	{
		HWND hWndParam = (HWND)lParam;
		if (hWndParam == g_hBtnOk)
		{
			HDC hdc = (HDC)wParam;
			HBRUSH hbrush;
			RECT rc;
			TCHAR text[64];
			GetWindowText(g_hBtnOk, text, 63);
			GetClientRect(g_hBtnOk, &rc);
			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkMode(hdc, TRANSPARENT);
			DrawText(hdc, text, _tcslen(text), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			SetBkColor(hdc, COLOR_BTN);
			hbrush = CreateSolidBrush(COLOR_BTN);
			return (LRESULT)hbrush;
		}
	}
		break;
	case WM_DESTROY:
		//delete g_pRemoteControl;
		g_pLanControl->EndClientConnection();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 远程控制窗口的消息处理函数
LRESULT CALLBACK RemoteWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	INPUT input = { 0 };

	switch (message)
	{
	case WM_CREATE:
		g_hRemoteWnd = hWnd;
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (g_bIsLan)
		{
			g_pLanControl->EndClientConnection();
			g_bIsControling = false;
		}
		else
		{

		}
		DestroyWindow(hWnd);
		break;
	case WM_KEYDOWN:
		g_pLanControl->SendKeyDown((int)wParam);
		break;
	case WM_KEYUP:
		g_pLanControl->SendKeyUp((int)wParam);
		break;
	case WM_MOUSEMOVE:
		g_pLanControl->SendMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		g_pLanControl->SendLeftButtonDown();
		break;
	case WM_LBUTTONUP:
		g_pLanControl->SendLeftButtonUp();
		break;
	case WM_LBUTTONDBLCLK:
		g_pLanControl->SendLeftButtonDoubleClick();
		break;
	case WM_RBUTTONDOWN:
		g_pLanControl->SendRightButtonDown();
		break;
	case WM_RBUTTONUP:
		g_pLanControl->SendRightButtonUp();
		break;
	case WM_RBUTTONDBLCLK:
		g_pLanControl->SendRightButtonDoubleClick();
		break;
	case WM_MBUTTONDOWN:
		g_pLanControl->SendMiddleButtonDown();
		break;
	case WM_MBUTTONUP:
		g_pLanControl->SendMiddleButtonUp();
		break;
	case WM_MBUTTONDBLCLK:
		g_pLanControl->SendMiddleButtonDoubleClick();
		break;
	case WM_MOUSEWHEEL:
		g_pLanControl->SendMouseWheel((short)HIWORD(wParam));
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


















// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
