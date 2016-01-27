
#include "StatusBar.h"
#include <CommCtrl.h>
using std::wstring;

StatusBar::StatusBar()
	: _hWnd(nullptr)
{
}

StatusBar& StatusBar::create(HWND hParent)
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

void StatusBar::stretch(WPARAM wp, LPARAM lp)
{
	// Intended to be called with parent's WM_SIZE processing.

	if (wp != SIZE_MINIMIZED && _hWnd) {
		int cx = LOWORD(lp); // available width
		SendMessage(_hWnd, WM_SIZE, 0, 0); // tell statusbar to fit parent

		// Find the space to be divided among variable-width parts,
		// and total weight of variable-width parts.
		UINT totalWeight = 0;
		int  cxVariable = cx;
		for (const Part& part : _parts) {
			if (!part.resizeWeight) { // fixed-width?
				cxVariable -= part.sizePixels;
			} else {
				totalWeight += part.resizeWeight;
			}
		}

		// Fill right edges array with the right edge of each part.
		int cxTotal = cx;
		for (auto i = _parts.size(); i-- > 0; ) {
			_rightEdges[i] = cxTotal;
			cxTotal -= (!_parts[i].resizeWeight) ? // fixed-width?
				_parts[i].sizePixels :
				static_cast<int>( (cxVariable / totalWeight) * _parts[i].resizeWeight );
		}
		SendMessage(_hWnd, SB_SETPARTS, _rightEdges.size(),
			reinterpret_cast<LPARAM>(&_rightEdges[0]));
	}
}

StatusBar& StatusBar::addFixedPart(UINT sizePixels)
{
	if (_hWnd) {
		_parts.push_back({ sizePixels, 0 });
		_rightEdges.emplace_back(0);
		stretch(SIZE_RESTORED, MAKELPARAM(_getParentCx(), 0));
	}
	return *this;
}

StatusBar& StatusBar::addResizablePart(UINT resizeWeight)
{
	// How resizeWeight works:
	// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
	// If available client area is 400px, respective part widths will be 100, 100 and 200px.
	// Zero weight means a fixed-width part, which therefore should have sizePixels set (otherwise zero).
	if (_hWnd) {
		_parts.push_back({ 0, resizeWeight });
		_rightEdges.emplace_back(0);
		stretch(SIZE_RESTORED, MAKELPARAM(_getParentCx(), 0));
	}
	return *this;
}

StatusBar& StatusBar::setText(const wchar_t *text, size_t iPart)
{
	SendMessage(_hWnd, SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0),
		reinterpret_cast<LPARAM>(text));
	return *this;
}

StatusBar& StatusBar::setText(const wstring& text, size_t iPart)
{
	return setText(text.c_str(), iPart);
}

wstring StatusBar::getText(size_t iPart) const
{
	int len = LOWORD(SendMessage(_hWnd, SB_GETTEXTLENGTH, iPart, 0)) + 1;
	wstring buf(len, L'\0');
	SendMessage(_hWnd, SB_GETTEXT, iPart, reinterpret_cast<LPARAM>(&buf[0]));
	buf.resize(len);
	return buf;
}

StatusBar& StatusBar::setIcon(HICON hIcon, size_t iPart)
{
	SendMessage(_hWnd, SB_SETICON, iPart, reinterpret_cast<LPARAM>(hIcon));
	return *this;
}

int StatusBar::_getParentCx()
{
	static int cx = 0; // cache, since parts are intended to be added during window creation only
	if (!cx && _hWnd) {
		RECT rc = { 0 };
		GetClientRect(GetParent(_hWnd), &rc);
		cx = rc.right;
	}
	return cx;
}