
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

class DC {
public:
	class Pen final {
	public:
		enum class Style : int {
			SOLID      = PS_SOLID,
			DASH       = PS_DASH,
			DOT        = PS_DOT,
			DASHDOT    = PS_DASHDOT,
			DASHDOTDOT = PS_DASHDOTDOT
		};
	private:
		HPEN _hPen;
	public:
		~Pen() { release(); }
		Pen(Style style, int width, COLORREF color);
		HPEN hPen() const { return _hPen; }
		void release();
	};

	class Brush final {
	public:
		enum class Pattern : int {
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
		~Brush() { release(); }
		Brush(COLORREF color);
		Brush(Pattern hatch, COLORREF color);
		Brush(int sysColor);
		HBRUSH hBrush() const { return _hBrush; }
		void   release();
	};

protected:
	HWND _hWnd;
	HDC  _hDC;
	SIZE _sz;
public:
	virtual ~DC() = default;
	explicit DC(HWND hWnd, HDC hDC = nullptr);

	HDC      hDC() const               { return _hDC; }
	HWND     hWnd() const              { return _hWnd; }
	SIZE     size() const              { return _sz; }
	DC&      deleteObject(HGDIOBJ obj);
	HGDIOBJ  selectObject(HGDIOBJ obj) { return SelectObject(_hDC, obj); }
	HGDIOBJ  selectStockFont()         { return selectObject(GetStockObject(SYSTEM_FONT)); }
	HGDIOBJ  selectStockPen()          { return selectObject(GetStockObject(BLACK_PEN)); }
	HGDIOBJ  selectStockBrush()        { return selectObject(GetStockObject(WHITE_BRUSH)); }
	DC&      moveTo(int x, int y);
	DC&      lineTo(int x, int y);
	DC&      lineRect(int left, int top, int right, int bottom);
	DC&      lineRect(RECT rc)         { return lineRect(rc.left, rc.top, rc.right, rc.bottom); }
	DC&      setBkTransparent(bool yes);
	DC&      setBkColor(COLORREF color = -1);
	COLORREF getBkBrushColor();
	DC&      setTextColor(COLORREF color);
	DC&      textOut(int x, int y, const wchar_t *text, size_t numChars = std::wstring::npos);
	DC&      textOut(int x, int y, const std::wstring& text, size_t numChars = std::wstring::npos);
	DC&      drawText(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags = 0, size_t numChars = std::wstring::npos);
	DC&      drawText(int x, int y, int cx, int cy, const std::wstring& text, UINT fmtFlags = 0, size_t numChars = std::wstring::npos);
	SIZE     getTextExtent(const wchar_t *text, size_t numChars = std::wstring::npos);
	SIZE     getTextExtent(const std::wstring& text, size_t numChars = std::wstring::npos);
	DC&      fillRect(int left, int top, int right, int bottom, HBRUSH hBrush);
	DC&      fillRect(int left, int top, int right, int bottom, const Brush& brush) { return fillRect(left, top, right, bottom, brush.hBrush()); }
	DC&      fillRgn(HRGN hrgn, HBRUSH hBrush);
	DC&      fillRgn(HRGN hrgn, const Brush& brush)    { return fillRgn(hrgn, brush.hBrush()); }
	DC&      polygon(const POINT *points, size_t numPoints);
	DC&      polygon(const std::vector<POINT>& points) { return polygon(&points[0], points.size()); }
	DC&      polygon(int left, int top, int right, int bottom);
	DC&      drawEdge(RECT rc, int edgeType, int flags);
	DC&      drawEdge(int left, int top, int right, int bottom, int edgeType, int flags);
};


// For within WM_PAINT, uses BeginPaint/EndPaint.
class DCSimple : public DC {
protected:
	PAINTSTRUCT _ps;
public:
	virtual ~DCSimple()          { EndPaint(_hWnd, &_ps); }
	explicit DCSimple(HWND hWnd) : DC(hWnd, BeginPaint(hWnd, &_ps)) { }
};


// For within WM_PAINT, uses BeginPaint/EndPaint with double-buffer.
// You must return zero on WM_ERASEBKGND message.
class DCBuffered final : public DCSimple {
private:
	HBITMAP _hBmp, _hBmpOld;
public:
	~DCBuffered();
	explicit DCBuffered(HWND hWnd);
};