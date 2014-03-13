//
// GDI device context automation.
// Friday night of October 28, 2011.
// Double-buffered at evening of Thursday, November 3, 2011.
// Class hierarchy at morning of Thursday, December 29, 2011.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "util.h"

//__________________________________________________________________________________________________
// Base device context class with basic operations.
//
class DC {
public:
	class Pen {
	public:
		enum class Style { SOLID=PS_SOLID, DASH=PS_DASH, DOT=PS_DOT, DASHDOT=PS_DASHDOT, DASHDOTDOT=PS_DASHDOTDOT };
		Pen(Style style, int width, COLORREF color) : _hPen(::CreatePen((int)style, width, color)) { }
		~Pen()                                      { ::DeleteObject(_hPen); }
		HPEN hPen() const                           { return _hPen; }
	private:
		HPEN _hPen;
	};

	class Brush {
	public:
		enum class Pattern { BDIAGONAL=HS_BDIAGONAL, CROSS=HS_CROSS, DIAGCROSS=HS_DIAGCROSS, FDIAGONAL=HS_FDIAGONAL, HORIZONTAL=HS_HORIZONTAL, VERTICAL=HS_VERTICAL };
		Brush(COLORREF color)                : _hBrush(::CreateSolidBrush(color)) { }
		Brush(Pattern hatch, COLORREF color) : _hBrush(::CreateHatchBrush((int)hatch, color)) { }
		Brush(SysColor color)                : _hBrush(::GetSysColorBrush((int)color)) { }
		~Brush()                             { ::DeleteObject(_hBrush); }
		HBRUSH hBrush() const                { return _hBrush; }
	private:
		HBRUSH _hBrush;
	};

public:
	int cx, cy;

	explicit DC(HWND hwnd, HDC hDC=0);
	virtual ~DC() { }

	HDC      hdc() const                { return _hdc; }
	HWND     hWnd() const               { return _hWnd; }
	DC&      deleteObject(HGDIOBJ obj)  { ::DeleteObject(obj); return *this; }
	HGDIOBJ  selectObject(HGDIOBJ obj)  { return ::SelectObject(_hdc, obj); }
	HGDIOBJ  selectStockFont()          { return selectObject(::GetStockObject(SYSTEM_FONT)); }
	HGDIOBJ  selectStockPen()           { return selectObject(::GetStockObject(BLACK_PEN)); }
	HGDIOBJ  selectStockBrush()         { return selectObject(::GetStockObject(WHITE_BRUSH)); }
	DC&      moveTo(int x, int y)       { ::MoveToEx(_hdc, x, y, 0); return *this; }
	DC&      lineTo(int x, int y)       { ::LineTo(_hdc, x, y); return *this; }
	DC&      lineRect(int left, int top, int right, int bottom) { return moveTo(left, top).lineTo(right, top).lineTo(right, bottom).lineTo(left, bottom); }
	DC&      lineRect(const RECT *rc)                           { return lineRect(rc->left, rc->top, rc->right, rc->bottom); }
	DC&      setBkTransparent(bool yes) { ::SetBkMode(_hdc, yes ? TRANSPARENT : OPAQUE); return *this; }
	DC&      setBkColor(COLORREF color=-1);
	COLORREF getBkBrushColor();
	DC&      setTextColor(COLORREF color)                             { ::SetTextColor(_hdc, color); return *this; }
	DC&      textOut(int x, int y, const wchar_t *text, int numChars) { ::TextOut(_hdc, x, y, text, numChars); return *this; }
	DC&      textOut(int x, int y, const wchar_t *text)               { return textOut(x, y, text, ::lstrlen(text)); }
	DC&      drawText(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags=0);
	SIZE     getTextExtent(const wchar_t *text, int numChars)                        { SIZE sz; ::GetTextExtentPoint32(_hdc, text, numChars, &sz); return sz; }
	SIZE     getTextExtent(const wchar_t *text)                                      { return getTextExtent(text, ::lstrlen(text)); }
	DC&      fillRect(int left, int top, int right, int bottom, HBRUSH hBrush)       { RECT rc = { left, top, right, bottom }; ::FillRect(_hdc, &rc, hBrush); return *this; }
	DC&      fillRgn(HRGN hrgn, HBRUSH hBrush)                                       { ::FillRgn(_hdc, hrgn, hBrush); return *this; }
	DC&      polygon(const POINT *points, int numPoints)                             { ::Polygon(_hdc, points, numPoints); return *this; }
	DC&      polygon(int left, int top, int right, int bottom);
	DC&      drawEdge(const RECT *rc, int edge, int flags)                           { ::DrawEdge(_hdc, (RECT*)rc, edge, flags); return *this; }
	DC&      drawEdge(int left, int top, int right, int bottom, int edge, int flags) { RECT rc = { left, top, right, bottom }; return drawEdge(&rc, edge, flags); }

protected:
	HDC  _hdc;
	HWND _hWnd;
};

//__________________________________________________________________________________________________
// For within WM_PAINT, uses BeginPaint/EndPaint.
//
class DCSimple : public DC {
public:
	explicit DCSimple(HWND hwnd) : DC(hwnd, ::BeginPaint(hwnd, &_ps)) { }
	virtual ~DCSimple()          { ::EndPaint(_hWnd, &_ps); }
protected:
	PAINTSTRUCT _ps;
};

//__________________________________________________________________________________________________
// For within WM_PAINT, uses BeginPaint/EndPaint with double-buffer.
// You must return zero on WM_ERASEBKGND message.
//
class DCBuffered : public DCSimple {
public:
	explicit DCBuffered(HWND hwnd);
	virtual ~DCBuffered();
protected:
	HBITMAP _hBmp, _hBmpOld;
};