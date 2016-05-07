/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace winutil {

class device_context {
public:
	class pen final {
	public:
		enum class style : int {
			SOLID      = PS_SOLID,
			DASH       = PS_DASH,
			DOT        = PS_DOT,
			DASHDOT    = PS_DASHDOT,
			DASHDOTDOT = PS_DASHDOTDOT
		};
	private:
		HPEN _hPen;
	public:
		~pen() { release(); }
		pen(style styleType, int width, COLORREF color);
		HPEN hpen() const { return _hPen; }
		void release();
	};

	class brush final {
	public:
		enum class pattern : int {
			BDIAGONAL  = HS_BDIAGONAL,
			CROSS      = HS_CROSS,
			DIAGCROSS  = HS_DIAGCROSS,
			FDIAGONAL  = HS_FDIAGONAL,
			HORIZONTAL = HS_HORIZONTAL,
			VERTICAL   = HS_VERTICAL
		};
	private:
		HBRUSH _hBrush;
	public:
		~brush() { release(); }
		brush(COLORREF color);
		brush(pattern hatch, COLORREF color);
		brush(int sysColor);
		HBRUSH hbrush() const { return _hBrush; }
		void   release();
	};

protected:
	HWND _hWnd;
	HDC  _hDC;
	SIZE _sz;
public:
	virtual ~device_context() = default;
	explicit device_context(HWND hWnd, HDC hDC = nullptr);

	HDC             hdc() const                { return _hDC; }
	HWND            hwnd() const               { return _hWnd; }
	SIZE            size() const               { return _sz; }
	device_context& delete_object(HGDIOBJ obj);
	HGDIOBJ         select_object(HGDIOBJ obj) { return SelectObject(_hDC, obj); }
	HGDIOBJ         select_stock_font()        { return select_object(GetStockObject(SYSTEM_FONT)); }
	HGDIOBJ         select_stock_pen()         { return select_object(GetStockObject(BLACK_PEN)); }
	HGDIOBJ         select_stock_brush()       { return select_object(GetStockObject(WHITE_BRUSH)); }
	device_context& move_to(int x, int y);
	device_context& line_to(int x, int y);
	device_context& line_rect(int left, int top, int right, int bottom);
	device_context& line_rect(RECT rc)         { return line_rect(rc.left, rc.top, rc.right, rc.bottom); }
	device_context& set_bk_transparent(bool yes);
	device_context& set_bk_color(COLORREF color = -1);
	COLORREF        get_bk_brush_color();
	device_context& set_text_color(COLORREF color);
	device_context& text_out(int x, int y, const wchar_t *text, size_t numChars = std::wstring::npos);
	device_context& text_out(int x, int y, const std::wstring& text, size_t numChars = std::wstring::npos);
	device_context& draw_text(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags = 0, size_t numChars = std::wstring::npos);
	device_context& draw_text(int x, int y, int cx, int cy, const std::wstring& text, UINT fmtFlags = 0, size_t numChars = std::wstring::npos);
	SIZE            get_text_extent(const wchar_t *text, size_t numChars = std::wstring::npos);
	SIZE            get_text_extent(const std::wstring& text, size_t numChars = std::wstring::npos);
	device_context& fill_rect(int left, int top, int right, int bottom, HBRUSH hBrush);
	device_context& fill_rect(int left, int top, int right, int bottom, const brush& brush) { return fill_rect(left, top, right, bottom, brush.hbrush()); }
	device_context& fill_rgn(HRGN hrgn, HBRUSH hBrush);
	device_context& fill_rgn(HRGN hrgn, const brush& brush)   { return fill_rgn(hrgn, brush.hbrush()); }
	device_context& polygon(const POINT *points, size_t numPoints);
	device_context& polygon(const std::vector<POINT>& points) { return polygon(&points[0], points.size()); }
	device_context& polygon(int left, int top, int right, int bottom);
	device_context& draw_edge(RECT rc, int edgeType, int flags);
	device_context& draw_edge(int left, int top, int right, int bottom, int edgeType, int flags);
};


// For within WM_PAINT, uses BeginPaint/EndPaint.
class device_context_simple : public device_context {
protected:
	PAINTSTRUCT _ps;
public:
	virtual ~device_context_simple()          { EndPaint(_hWnd, &_ps); }
	explicit device_context_simple(HWND hWnd) : device_context(hWnd, BeginPaint(hWnd, &_ps)) { }
};


// For within WM_PAINT, uses BeginPaint/EndPaint with double-buffer.
// You must return zero on WM_ERASEBKGND message.
class device_context_buffered final : public device_context_simple {
private:
	HBITMAP _hBmp, _hBmpOld;
public:
	~device_context_buffered();
	explicit device_context_buffered(HWND hWnd);
};

}//namespace winutil