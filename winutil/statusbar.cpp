/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "statusbar.h"
#include <CommCtrl.h>
using namespace winutil;
using std::wstring;

statusbar& statusbar::create(HWND hParent)
{
	if (!_hWnd) {
		DWORD parentStyle = static_cast<DWORD>(GetWindowLongPtr(hParent, GWL_STYLE));
		bool isStretch = (parentStyle & WS_MAXIMIZEBOX) != 0 ||
			(parentStyle & WS_SIZEBOX) != 0;

		_hWnd = CreateWindowEx(0, STATUSCLASSNAME, nullptr,
			(WS_CHILD | WS_VISIBLE) | (isStretch ? SBARS_SIZEGRIP : 0),
			0, 0, 0, 0, hParent, nullptr,
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
			nullptr);
	}
	return *this;
}

void statusbar::stretch(WPARAM wp, LPARAM lp)
{
	// Intended to be called with parent's WM_SIZE processing.

	if (wp != SIZE_MINIMIZED && _hWnd) {
		int cx = LOWORD(lp); // available width
		SendMessage(_hWnd, WM_SIZE, 0, 0); // tell statusbar to fit parent

		// Find the space to be divided among variable-width parts,
		// and total weight of variable-width parts.
		UINT totalWeight = 0;
		int  cxVariable = cx;
		for (const part& onePart : _parts) {
			if (!onePart.resizeWeight) { // fixed-width?
				cxVariable -= onePart.sizePixels;
			} else {
				totalWeight += onePart.resizeWeight;
			}
		}

		// Fill right edges array with the right edge of each part.
		int cxTotal = cx;
		for (size_t i = _parts.size(); i-- > 0; ) {
			_rightEdges[i] = cxTotal;
			cxTotal -= (!_parts[i].resizeWeight) ? // fixed-width?
				_parts[i].sizePixels :
				static_cast<int>( (cxVariable / totalWeight) * _parts[i].resizeWeight );
		}
		SendMessage(_hWnd, SB_SETPARTS, _rightEdges.size(),
			reinterpret_cast<LPARAM>(&_rightEdges[0]));
	}
}

statusbar& statusbar::add_fixed_part(UINT sizePixels)
{
	if (_hWnd) {
		_parts.push_back({ sizePixels, 0 });
		_rightEdges.emplace_back(0);
		stretch(SIZE_RESTORED, MAKELPARAM(_get_parent_cx(), 0));
	}
	return *this;
}

statusbar& statusbar::add_resizable_part(UINT resizeWeight)
{
	// How resizeWeight works:
	// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
	// If available client area is 400px, respective part widths will be 100, 100 and 200px.
	// Zero weight means a fixed-width part, which therefore should have sizePixels set (otherwise zero).
	if (_hWnd) {
		_parts.push_back({ 0, resizeWeight });
		_rightEdges.emplace_back(0);
		stretch(SIZE_RESTORED, MAKELPARAM(_get_parent_cx(), 0));
	}
	return *this;
}

statusbar& statusbar::set_text(const wchar_t* text, size_t iPart)
{
	SendMessage(_hWnd, SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0),
		reinterpret_cast<LPARAM>(text));
	return *this;
}

wstring statusbar::get_text(size_t iPart) const
{
	int len = LOWORD(SendMessage(_hWnd, SB_GETTEXTLENGTH, iPart, 0)) + 1;
	wstring buf(len, L'\0');
	SendMessage(_hWnd, SB_GETTEXT, iPart, reinterpret_cast<LPARAM>(&buf[0]));
	buf.resize(len);
	return buf;
}

statusbar& statusbar::set_icon(HICON hIcon, size_t iPart)
{
	SendMessage(_hWnd, SB_SETICON, iPart, reinterpret_cast<LPARAM>(hIcon));
	return *this;
}

int statusbar::_get_parent_cx()
{
	static int cx = 0; // cache, since parts are intended to be added during window creation only
	if (!cx && _hWnd) {
		RECT rc = { 0 };
		GetClientRect(GetParent(_hWnd), &rc);
		cx = rc.right;
	}
	return cx;
}