//
// GDI device context automation.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
//

#include "DeviceContext.h"

DC::DC(HWND hwnd, HDC hDC)
	: _hWnd(hwnd), _hdc(hDC)
{
	RECT rcClient = { 0 };
	GetClientRect(_hWnd, &rcClient); // let's keep available width & height
	this->cx = rcClient.right; // these variables are public
	this->cy = rcClient.bottom;
}

DC& DC::setBkColor(COLORREF color)
{
	SetBkColor(_hdc, color == -1 ? // default?
		this->getBkBrushColor() : color);
	return *this;
}

COLORREF DC::getBkBrushColor()
{
	ULONG_PTR hbrBg = GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND);
	if (hbrBg > 100) {
		// The hbrBackground is a brush handle, not a system color constant.
		// This 100 value is arbitrary, based on system color constants like COLOR_BTNFACE.
		LOGBRUSH logBrush;
		GetObject((HBRUSH)hbrBg, sizeof(LOGBRUSH), &logBrush);
		return logBrush.lbColor;
	}
	return GetSysColor((int)hbrBg - 1);
}

DC& DC::drawText(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags)
{
	RECT rc = { x, y, x + cx, y + cy };
	DrawText(_hdc, text, lstrlen(text), &rc, fmtFlags); // DT_LEFT|DT_TOP is zero
	return *this;
}

DC& DC::polygon(int left, int top, int right, int bottom)
{
	POINT pts[] = {
		{ left, top },
		{ left, bottom },
		{ right, bottom },
		{ right, top }
	};
	return this->polygon(pts, 4);
}


DCBuffered::DCBuffered(HWND hwnd)
	: DCSimple(hwnd)
{
	_hdc = CreateCompatibleDC(_ps.hdc); // overwrite our painting HDC
	_hBmp = CreateCompatibleBitmap(_ps.hdc, this->cx, this->cy);
	_hBmpOld = (HBITMAP)SelectObject(_hdc, _hBmp);
	
	RECT rcClient = { 0, 0, this->cx, this->cy };
	FillRect(_hdc, &rcClient, (HBRUSH)GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND));
}

DCBuffered::~DCBuffered()
{
	BITMAP bm = { 0 }; // http://www.ureader.com/msg/14721900.aspx
	GetObject(_hBmp, sizeof(bm), &bm);
	BitBlt(_ps.hdc, 0, 0, bm.bmWidth, bm.bmHeight, _hdc, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(_hdc, _hBmpOld));
	DeleteObject(_hBmp);
	DeleteDC(_hdc);
}