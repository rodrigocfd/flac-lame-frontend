
#include "DC.h"
using std::vector;
using std::wstring;

DC::Pen::Pen(Style style, int width, COLORREF color)
	: _hPen(CreatePen(static_cast<int>(style), width, color))
{
}

void DC::Pen::release()
{
	if (_hPen) {
		DeleteObject(_hPen);
		_hPen = nullptr;
	}
}


DC::Brush::Brush(COLORREF color)
	: _hBrush(CreateSolidBrush(color))
{
}

DC::Brush::Brush(Pattern hatch, COLORREF color)
	: _hBrush(CreateHatchBrush(static_cast<int>(hatch), color))
{
}

DC::Brush::Brush(int sysColor)
	: _hBrush(GetSysColorBrush(static_cast<int>(sysColor)))
{
	// COLOR_BTNFACE COLOR_DESKTOP COLOR_BTNTEXT COLOR_WINDOW COLOR_APPWORKSPACE
}

void DC::Brush::release()
{
	if (_hBrush) {
		DeleteObject(_hBrush);
		_hBrush = nullptr;
	}
}


DC::DC(HWND hWnd, HDC hDC)
	: _hWnd(hWnd), _hDC(hDC)
{
	RECT rcClient = { 0 };
	GetClientRect(_hWnd, &rcClient); // let's keep available width & height
	_sz.cx = rcClient.right;
	_sz.cy = rcClient.bottom;
}

DC& DC::deleteObject(HGDIOBJ obj)
{
	DeleteObject(obj);
	return *this;
}

DC& DC::moveTo(int x, int y)
{
	MoveToEx(_hDC, x, y, nullptr);
	return *this;
}

DC& DC::lineTo(int x, int y)
{
	LineTo(_hDC, x, y);
	return *this;
}

DC& DC::lineRect(int left, int top, int right, int bottom)
{
	return moveTo(left, top)
		.lineTo(right, top)
		.lineTo(right, bottom)
		.lineTo(left, bottom);
}

DC& DC::setBkTransparent(bool yes)
{
	SetBkMode(_hDC, yes ? TRANSPARENT : OPAQUE);
	return *this;
}

DC& DC::setBkColor(COLORREF color)
{
	SetBkColor(_hDC, (color == -1) ? // default?
		getBkBrushColor() : color);
	return *this;
}

COLORREF DC::getBkBrushColor()
{
	ULONG_PTR hbrBg = GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND);
	if (hbrBg > 100) {
		// The hbrBackground is a brush handle, not a system color constant.
		// This 100 value is arbitrary, based on system color constants like COLOR_BTNFACE.
		LOGBRUSH logBrush;
		GetObject(reinterpret_cast<HBRUSH>(hbrBg), sizeof(LOGBRUSH), &logBrush);
		return logBrush.lbColor;
	}
	return GetSysColor(static_cast<int>(hbrBg - 1));
}

DC& DC::setTextColor(COLORREF color)
{
	SetTextColor(_hDC, color);
	return *this;
}

DC& DC::textOut(int x, int y, const wchar_t *text, size_t numChars)
{
	TextOut(_hDC, x, y, text,
		(numChars == wstring::npos) ? lstrlen(text) : static_cast<int>(numChars));
	return *this;
}

DC& DC::textOut(int x, int y, const wstring& text, size_t numChars)
{
	return textOut(x, y, text.c_str(),
		(numChars == wstring::npos) ? text.size() : numChars);
}

DC& DC::drawText(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags, size_t numChars)
{
	RECT rc = { x, y, x + cx, y + cy };
	DrawText(_hDC, text,
		(numChars == wstring::npos) ? lstrlen(text) : static_cast<int>(numChars),
		&rc, fmtFlags); // DT_LEFT|DT_TOP is zero
	return *this;
}

DC& DC::drawText(int x, int y, int cx, int cy, const wstring& text, UINT fmtFlags, size_t numChars)
{
	return drawText(x, y, cx, cy, text.c_str(), fmtFlags,
		(numChars == wstring::npos) ? text.size() : numChars);
}

SIZE DC::getTextExtent(const wchar_t *text, size_t numChars)
{
	SIZE sz;
	GetTextExtentPoint32(_hDC, text,
		(numChars == wstring::npos) ? lstrlen(text) : static_cast<int>(numChars),
		&sz);
	return sz;
}

SIZE DC::getTextExtent(const wstring& text, size_t numChars)
{
	return getTextExtent(text.c_str(),
		(numChars == wstring::npos) ? text.size() : numChars);
}

DC& DC::fillRect(int left, int top, int right, int bottom, HBRUSH hBrush)
{
	RECT rc = { left, top, right, bottom };
	FillRect(_hDC, &rc, hBrush);
	return *this;
}

DC& DC::fillRgn(HRGN hrgn, HBRUSH hBrush)
{
	FillRgn(_hDC, hrgn, hBrush);
	return *this;
}

DC& DC::polygon(const POINT *points, size_t numPoints)
{
	Polygon(_hDC, points, static_cast<int>(numPoints));
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
	return polygon(pts, 4);
}

DC& DC::drawEdge(RECT rc, int edgeType, int flags)
{
	DrawEdge(_hDC, &rc, edgeType, flags);
	return *this;
}

DC& DC::drawEdge(int left, int top, int right, int bottom, int edgeType, int flags)
{
	RECT rc = { left, top, right, bottom };
	return drawEdge(std::move(rc), edgeType, flags);
}


DCBuffered::~DCBuffered()
{
	BITMAP bm = { 0 }; // http://www.ureader.com/msg/14721900.aspx
	GetObject(_hBmp, sizeof(bm), &bm);
	BitBlt(_ps.hdc, 0, 0, bm.bmWidth, bm.bmHeight, _hDC, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(_hDC, _hBmpOld));
	DeleteObject(_hBmp);
	DeleteDC(_hDC);
	// ~DCSimple() kicks in
}

DCBuffered::DCBuffered(HWND hWnd)
	: DCSimple(hWnd)
{
	_hDC = CreateCompatibleDC(_ps.hdc); // overwrite our painting HDC
	_hBmp = CreateCompatibleBitmap(_ps.hdc, _sz.cx, _sz.cy);
	_hBmpOld = reinterpret_cast<HBITMAP>(SelectObject(_hDC, _hBmp));

	RECT rcClient = { 0, 0, _sz.cx, _sz.cy };
	FillRect(_hDC, &rcClient,
		reinterpret_cast<HBRUSH>(GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND)) );
}