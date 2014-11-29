//
// GDI device context automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "System.h"

//__________________________________________________________________________________________________
// Base device context class with basic operations.
//
class DC {
public:
	class Pen final {
	private:
		HPEN _hPen;
	public:
		enum class Style { SOLID=PS_SOLID, DASH=PS_DASH, DOT=PS_DOT, DASHDOT=PS_DASHDOT, DASHDOTDOT=PS_DASHDOTDOT };
		Pen(Style style, int width, COLORREF color) : _hPen(::CreatePen((int)style, width, color)) { }
		~Pen()                                      { ::DeleteObject(_hPen); }
		HPEN hPen() const                           { return _hPen; }
	};

	class Brush final {
	private:
		HBRUSH _hBrush;
	public:
		enum class Pattern { BDIAGONAL=HS_BDIAGONAL, CROSS=HS_CROSS, DIAGCROSS=HS_DIAGCROSS, FDIAGONAL=HS_FDIAGONAL, HORIZONTAL=HS_HORIZONTAL, VERTICAL=HS_VERTICAL };
		Brush(COLORREF color)                : _hBrush(::CreateSolidBrush(color)) { }
		Brush(Pattern hatch, COLORREF color) : _hBrush(::CreateHatchBrush((int)hatch, color)) { }
		Brush(System::Color color)           : _hBrush(::GetSysColorBrush((int)color)) { }
		~Brush()                             { ::DeleteObject(_hBrush); }
		HBRUSH hBrush() const                { return _hBrush; }
	};

protected:
	HDC  _hdc;
	HWND _hWnd;
public:
	int cx, cy;

	explicit DC(HWND hwnd, HDC hDC=nullptr);
	virtual ~DC() { }

	HDC      hdc() const                { return _hdc; }
	HWND     hWnd() const               { return _hWnd; }
	DC&      deleteObject(HGDIOBJ obj)  { ::DeleteObject(obj); return *this; }
	HGDIOBJ  selectObject(HGDIOBJ obj)  { return ::SelectObject(_hdc, obj); }
	HGDIOBJ  selectStockFont()          { return selectObject(::GetStockObject(SYSTEM_FONT)); }
	HGDIOBJ  selectStockPen()           { return selectObject(::GetStockObject(BLACK_PEN)); }
	HGDIOBJ  selectStockBrush()         { return selectObject(::GetStockObject(WHITE_BRUSH)); }
	DC&      moveTo(int x, int y)       { ::MoveToEx(_hdc, x, y, nullptr); return *this; }
	DC&      lineTo(int x, int y)       { ::LineTo(_hdc, x, y); return *this; }
	DC&      lineRect(int left, int top, int right, int bottom)       { return moveTo(left, top).lineTo(right, top).lineTo(right, bottom).lineTo(left, bottom); }
	DC&      lineRect(const RECT *rc)                                 { return lineRect(rc->left, rc->top, rc->right, rc->bottom); }
	DC&      setBkTransparent(bool yes)                               { ::SetBkMode(_hdc, yes ? TRANSPARENT : OPAQUE); return *this; }
	DC&      setBkColor(COLORREF color=-1);
	COLORREF getBkBrushColor();
	DC&      setTextColor(COLORREF color)                             { ::SetTextColor(_hdc, color); return *this; }
	DC&      textOut(int x, int y, const wchar_t *text, int numChars) { ::TextOut(_hdc, x, y, text, numChars); return *this; }
	DC&      textOut(int x, int y, const wchar_t *text)               { return textOut(x, y, text, ::lstrlen(text)); }
	DC&      textOut(int x, int y, const String& text)                { return textOut(x, y, text.str(), text.len()); }
	DC&      drawText(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags=0);
	DC&      drawText(int x, int y, int cx, int cy, const String& text, UINT fmtFlags=0) { return drawText(x, y, cx, cy, text.str(), fmtFlags); }
	SIZE     getTextExtent(const wchar_t *text, int numChars)                            { SIZE sz; ::GetTextExtentPoint32(_hdc, text, numChars, &sz); return sz; }
	SIZE     getTextExtent(const wchar_t *text)                                          { return getTextExtent(text, ::lstrlen(text)); }
	SIZE     getTextExtent(const String& text)                                           { return getTextExtent(text.str(), text.len()); }
	DC&      fillRect(int left, int top, int right, int bottom, HBRUSH hBrush)           { RECT rc = { left, top, right, bottom }; ::FillRect(_hdc, &rc, hBrush); return *this; }
	DC&      fillRgn(HRGN hrgn, HBRUSH hBrush)                                           { ::FillRgn(_hdc, hrgn, hBrush); return *this; }
	DC&      polygon(const POINT *points, int numPoints)                                 { ::Polygon(_hdc, points, numPoints); return *this; }
	DC&      polygon(int left, int top, int right, int bottom);
	DC&      drawEdge(const RECT *rc, int edge, int flags)                               { ::DrawEdge(_hdc, (RECT*)rc, edge, flags); return *this; }
	DC&      drawEdge(int left, int top, int right, int bottom, int edge, int flags)     { RECT rc = { left, top, right, bottom }; return drawEdge(&rc, edge, flags); }
};

//__________________________________________________________________________________________________
// For within WM_PAINT, uses BeginPaint/EndPaint.
//
class DCSimple : public DC {
protected:
	PAINTSTRUCT _ps;
public:
	explicit DCSimple(HWND hwnd) : DC(hwnd, ::BeginPaint(hwnd, &_ps)) { }
	virtual ~DCSimple()          { ::EndPaint(_hWnd, &_ps); }
};

//__________________________________________________________________________________________________
// For within WM_PAINT, uses BeginPaint/EndPaint with double-buffer.
// You must return zero on WM_ERASEBKGND message.
//
class DCBuffered : public DCSimple {
protected:
	HBITMAP _hBmp, _hBmpOld;
public:
	explicit DCBuffered(HWND hwnd);
	virtual ~DCBuffered();
};