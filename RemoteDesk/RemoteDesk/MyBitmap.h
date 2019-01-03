#pragma once
class CMyBitmap
{
public:
	CMyBitmap();
	~CMyBitmap();

	static HBITMAP GetDesktopBitMap();
	static HDC GetDesktopImageDC();
	static BOOL SaveBmp(HBITMAP, WCHAR*);
	static BOOL SaveToClipboard(HWND, HBITMAP);
	static HBITMAP CopyBitmap(HDC, HBITMAP);
	static HBITMAP GetDarkBitmap(HDC, HBITMAP);
};

