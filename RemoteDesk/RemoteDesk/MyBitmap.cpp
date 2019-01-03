#include "stdafx.h"
#include "MyBitmap.h"


CMyBitmap::CMyBitmap()
{
}


CMyBitmap::~CMyBitmap()
{
}


/**
*	@brief	得到桌面图像，记得delete
*
*	@return	HBITMAP	位图图像句柄
*/
HBITMAP CMyBitmap::GetDesktopBitMap()
{
	// 得到桌面句柄和DC
	HWND hWndDesk = ::GetDesktopWindow();
	HDC hDcDesk = ::GetDC(hWndDesk);
	// 创建兼容位图
	HBITMAP hBitMap = ::CreateCompatibleBitmap(hDcDesk, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	// 保存位图信息
	BITMAP bmDesktop;
	::GetObject(hBitMap, sizeof(BITMAP), &bmDesktop);

	// 创建兼容DC
	HDC hDcMem = ::CreateCompatibleDC(hDcDesk);
	// 将位图选入DC
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hDcMem, hBitMap);
	// 将桌面DC复制到兼容DC
	::BitBlt(hDcMem, 0, 0, bmDesktop.bmWidth, bmDesktop.bmHeight, hDcDesk, 0, 0, SRCCOPY);
	hBitMap = (HBITMAP)::SelectObject(hDcMem, hOldBmp);

	::DeleteObject(hOldBmp);
	::ReleaseDC(NULL, hDcMem);
	::ReleaseDC(NULL, hDcDesk);
	return hBitMap;
}


/**
*	@brief	得到桌面DC句柄，记得delete
*
*	@return	HDC	画有桌面图像的设备上下文句柄
*/
HDC CMyBitmap::GetDesktopImageDC()
{
	HWND hWndDesk = ::GetDesktopWindow();
	HDC hDcDesk = ::GetDC(hWndDesk);
	HBITMAP hBitMap = ::CreateCompatibleBitmap(hDcDesk, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	BITMAP bmDesktop;
	::GetObject(hBitMap, sizeof(BITMAP), &bmDesktop);

	HDC hdcTmp = ::CreateCompatibleDC(hDcDesk);
	::SelectObject(hdcTmp, hBitMap);
	::BitBlt(hdcTmp, 0, 0, bmDesktop.bmWidth, bmDesktop.bmHeight, hDcDesk, 0, 0, SRCCOPY);

	::DeleteObject(hBitMap);
	::ReleaseDC(NULL, hDcDesk);
	return hdcTmp;
}


/**
*	@brief	保存位图为文件
*
*	@param	hBitmap		位图文件句柄
*	@param	FileName	文件名
*	@return	BOOL		是否成功，成功返回TRUE否则FALSE
*/
BOOL CMyBitmap::SaveBmp(HBITMAP hBitmap, WCHAR* lpszFileName)
{
	if (hBitmap == NULL || lpszFileName == NULL)
	{
		MessageBox(NULL, L"参数错误", L"Error", MB_OK);
		return false;
	}
	HDC hDC;
	//当前分辨率下每象素所占字节数
	int iBits;
	//位图中每象素所占字节数
	WORD wBitCount;
	//定义调色板大小， 位图中像素字节大小 ，位图文件大小 ， 写入文件字节数 
	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//位图属性结构 
	BITMAP Bitmap;
	//位图文件头结构
	BITMAPFILEHEADER bmfHdr;
	//位图信息头结构 
	BITMAPINFOHEADER bi;
	//指向位图信息头结构  
	LPBITMAPINFOHEADER lpbi;
	//定义文件，分配内存句柄，调色板句柄 
	HANDLE fh, hDib, hPal, hOldPal = NULL;
	//计算位图文件每个像素所占字节数 
	hDC = ::CreateDC(L"DISPLAY", NULL, NULL, NULL);
	iBits = ::GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	::DeleteDC(hDC);
	if (iBits <= 1)  wBitCount = 1;
	else if (iBits <= 4)  wBitCount = 4;
	else if (iBits <= 8)  wBitCount = 8;
	else if (iBits <= 24) wBitCount = 24;
	else wBitCount = 32;
	::GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//为位图内容分配内存 
	hDib = ::GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock(hDib);
	*lpbi = bi;

	// 处理调色板  
	hPal = ::GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		::RealizePalette(hDC);
	}

	// 获取该调色板下新的像素值 
	::GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
		+ dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//恢复调色板  
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		::RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//创建位图文件  
	fh = ::CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)  return FALSE;

	// 设置位图文件头 
	bmfHdr.bfType = 0x4D42; // "BM" 
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	// 写入位图文件头 
	::WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	// 写入位图文件其余内容 
	::WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//清除  
	::GlobalUnlock(hDib);
	::GlobalFree(hDib);
	::CloseHandle(fh);
	return TRUE;
}


/**
*	@brief	保存位图到剪切板
*
*	@param	hWnd	窗口句柄
*	@param	hBitmap	位图句柄
*	@retuen	BOOL	成功 TRUE, 失败 FALSE
*/
BOOL CMyBitmap::SaveToClipboard(HWND hWnd, HBITMAP hBitmap)
{
	if (OpenClipboard(hWnd))
	{
		//清空剪贴板
		EmptyClipboard();
		//把屏幕内容粘贴到剪贴板上,
		//hBitmap 为刚才的屏幕位图句柄
		SetClipboardData(CF_BITMAP, hBitmap);
		//关闭剪贴板
		CloseClipboard();
		return TRUE;
	}
	return FALSE;
}


/**
*	@brief	复制一份位图，记得delete
*
*	@param	hDc			和位图兼容的Dc句柄
*	@param	hSourceBmp	位图句柄作为复制源
*	@return	HBITMAP		位图句柄
*/
HBITMAP CMyBitmap::CopyBitmap(HDC hDc, HBITMAP hSourceBmp)
{
	if (hSourceBmp == NULL)
	{
		MessageBox(NULL, L"Copy null error", L"Copy error", MB_OK);
		return NULL;
	}
	HDC hDcSource = CreateCompatibleDC(hDc);
	HDC hDcDest = CreateCompatibleDC(hDc);
	BITMAP bmp;
	GetObject(hSourceBmp, sizeof(bmp), &bmp);

	HBITMAP hResBmp = CreateCompatibleBitmap(hDc, bmp.bmWidth, bmp.bmHeight);
	HBITMAP hOldBmpSource = (HBITMAP)SelectObject(hDcSource, hSourceBmp);
	HBITMAP hOldBmpDest = (HBITMAP)SelectObject(hDcDest, hResBmp);
	BitBlt(hDcDest, 0, 0, bmp.bmWidth, bmp.bmHeight, hDcSource, 0, 0, SRCCOPY);

	hResBmp = (HBITMAP)SelectObject(hDcDest, hOldBmpDest);
	SelectObject(hDcSource, hOldBmpSource);

	DeleteDC(hDcSource);
	DeleteDC(hDcDest);
	DeleteObject(hOldBmpSource);
	DeleteObject(hOldBmpDest);
	return hResBmp;
}


/**
*	@brief	获得暗化处理后的位图句柄,记得delete

*	@param	hDc		和位图兼容的DC的句柄
*	@param	hBitmap	位图句柄
*	@return	HBITMAP	位图句柄
*/
HBITMAP CMyBitmap::GetDarkBitmap(HDC hDc, HBITMAP hBitmap)
{
	if (hBitmap == NULL || hDc == NULL)
	{
		return NULL;
	}
	BITMAP bmp;
	BITMAPINFO bmpInfo;
	UINT* pData;
	HBITMAP hResBmp;

	GetObject(hBitmap, sizeof(bmp), &bmp);
	pData = new UINT[bmp.bmWidth * bmp.bmHeight];
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
	bmpInfo.bmiHeader.biHeight = -bmp.bmHeight;	// 倒过来
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biBitCount = 32;

	GetDIBits(hDc, hBitmap, 0, bmp.bmHeight, pData, &bmpInfo, DIB_RGB_COLORS);
	UINT color, r, g, b;
	for (int i = 0; i < bmp.bmWidth * bmp.bmHeight; i++)
	{
		color = pData[i];
		b = (UINT)((color << 8 >> 24) * 0.6);
		g = (UINT)((color << 16 >> 24) * 0.6);
		r = (UINT)((color << 24 >> 24) * 0.6);
		//note   that   infact,   the   r   is   Blue,   b   =   Red,
		//r   =   0;//mask   the   blue   bits
		pData[i] = RGB(r, g, b);
	}
	hResBmp = CreateCompatibleBitmap(hDc, bmp.bmWidth, bmp.bmHeight);
	SetDIBits(hDc, hResBmp, 0, bmp.bmHeight, pData, &bmpInfo, DIB_RGB_COLORS);

	delete[] pData;
	return hResBmp;
}
