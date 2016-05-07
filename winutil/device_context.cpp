/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "device_context.h"
using namespace winutil;
using std::vector;
using std::wstring;

device_context::pen::pen(style styleType, int width, COLORREF color)
	: _hPen(CreatePen(static_cast<int>(styleType), width, color))
{
}

void device_context::pen::release()
{
	if (_hPen) {
		DeleteObject(_hPen);
		_hPen = nullptr;
	}
}


device_context::brush::brush(COLORREF color)
	: _hBrush(CreateSolidBrush(color))
{
}

device_context::brush::brush(pattern hatch, COLORREF color)
	: _hBrush(CreateHatchBrush(static_cast<int>(hatch), color))
{
}

device_context::brush::brush(int sysColor)
	: _hBrush(GetSysColorBrush(static_cast<int>(sysColor)))
{
	// COLOR_BTNFACE COLOR_DESKTOP COLOR_BTNTEXT COLOR_WINDOW COLOR_APPWORKSPACE
}

void device_context::brush::release()
{
	if (_hBrush) {
		DeleteObject(_hBrush);
		_hBrush = nullptr;
	}
}


device_context::device_context(HWND hWnd, HDC hDC)
	: _hWnd(hWnd), _hDC(hDC)
{
	RECT rcClient = { 0 };
	GetClientRect(_hWnd, &rcClient); // let's keep available width & height
	_sz.cx = rcClient.right;
	_sz.cy = rcClient.bottom;
}

device_context& device_context::delete_object(HGDIOBJ obj)
{
	DeleteObject(obj);
	return *this;
}

device_context& device_context::move_to(int x, int y)
{
	MoveToEx(_hDC, x, y, nullptr);
	return *this;
}

device_context& device_context::line_to(int x, int y)
{
	LineTo(_hDC, x, y);
	return *this;
}

device_context& device_context::line_rect(int left, int top, int right, int bottom)
{
	return move_to(left, top)
		.line_to(right, top)
		.line_to(right, bottom)
		.line_to(left, bottom);
}

device_context& device_context::set_bk_transparent(bool yes)
{
	SetBkMode(_hDC, yes ? TRANSPARENT : OPAQUE);
	return *this;
}

device_context& device_context::set_bk_color(COLORREF color)
{
	SetBkColor(_hDC, (color == -1) ? // default?
		get_bk_brush_color() : color);
	return *this;
}

COLORREF device_context::get_bk_brush_color()
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

device_context& device_context::set_text_color(COLORREF color)
{
	SetTextColor(_hDC, color);
	return *this;
}

device_context& device_context::text_out(int x, int y, const wchar_t *text, size_t numChars)
{
	TextOut(_hDC, x, y, text,
		(numChars == wstring::npos) ? lstrlen(text) : static_cast<int>(numChars));
	return *this;
}

device_context& device_context::text_out(int x, int y, const wstring& text, size_t numChars)
{
	return text_out(x, y, text.c_str(),
		(numChars == wstring::npos) ? text.size() : numChars);
}

device_context& device_context::draw_text(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags, size_t numChars)
{
	RECT rc = { x, y, x + cx, y + cy };
	DrawText(_hDC, text,
		(numChars == wstring::npos) ? lstrlen(text) : static_cast<int>(numChars),
		&rc, fmtFlags); // DT_LEFT|DT_TOP is zero
	return *this;
}

device_context& device_context::draw_text(int x, int y, int cx, int cy, const wstring& text, UINT fmtFlags, size_t numChars)
{
	return draw_text(x, y, cx, cy, text.c_str(), fmtFlags,
		(numChars == wstring::npos) ? text.size() : numChars);
}

SIZE device_context::get_text_extent(const wchar_t *text, size_t numChars)
{
	SIZE sz;
	GetTextExtentPoint32(_hDC, text,
		(numChars == wstring::npos) ? lstrlen(text) : static_cast<int>(numChars),
		&sz);
	return sz;
}

SIZE device_context::get_text_extent(const wstring& text, size_t numChars)
{
	return get_text_extent(text.c_str(),
		(numChars == wstring::npos) ? text.size() : numChars);
}

device_context& device_context::fill_rect(int left, int top, int right, int bottom, HBRUSH hBrush)
{
	RECT rc = { left, top, right, bottom };
	FillRect(_hDC, &rc, hBrush);
	return *this;
}

device_context& device_context::fill_rgn(HRGN hrgn, HBRUSH hBrush)
{
	FillRgn(_hDC, hrgn, hBrush);
	return *this;
}

device_context& device_context::polygon(const POINT *points, size_t numPoints)
{
	Polygon(_hDC, points, static_cast<int>(numPoints));
	return *this;
}

device_context& device_context::polygon(int left, int top, int right, int bottom)
{
	POINT pts[] = {
		{ left, top },
		{ left, bottom },
		{ right, bottom },
		{ right, top }
	};
	return polygon(pts, 4);
}

device_context& device_context::draw_edge(RECT rc, int edgeType, int flags)
{
	DrawEdge(_hDC, &rc, edgeType, flags);
	return *this;
}

device_context& device_context::draw_edge(int left, int top, int right, int bottom, int edgeType, int flags)
{
	RECT rc = { left, top, right, bottom };
	return draw_edge(std::move(rc), edgeType, flags);
}


device_context_buffered::~device_context_buffered()
{
	BITMAP bm = { 0 }; // http://www.ureader.com/msg/14721900.aspx
	GetObject(_hBmp, sizeof(bm), &bm);
	BitBlt(_ps.hdc, 0, 0, bm.bmWidth, bm.bmHeight, _hDC, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(_hDC, _hBmpOld));
	DeleteObject(_hBmp);
	DeleteDC(_hDC);
	// ~DCSimple() kicks in
}

device_context_buffered::device_context_buffered(HWND hWnd)
	: device_context_simple(hWnd)
{
	_hDC = CreateCompatibleDC(_ps.hdc); // overwrite our painting HDC
	_hBmp = CreateCompatibleBitmap(_ps.hdc, _sz.cx, _sz.cy);
	_hBmpOld = reinterpret_cast<HBITMAP>(SelectObject(_hDC, _hBmp));

	RECT rcClient = { 0, 0, _sz.cx, _sz.cy };
	FillRect(_hDC, &rcClient,
		reinterpret_cast<HBRUSH>(GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND)) );
}