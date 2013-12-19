//
// Status bar automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Window.h"
#include <CommCtrl.h>

class StatusBar {
public:
	StatusBar& create(HWND hOwner, int numPartsItWillHave);
	StatusBar& addFixedPart(BYTE sizePixels);
	StatusBar& addResizablePart(float resizeWeight);
	StatusBar& setText(const wchar_t *text, int iPart) { _sb.sendMessage(SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0), (LPARAM)text); return *this; }
	String     getText(int iPart) const;
	void       setIcon(HICON hIcon, int iPart)         { _sb.sendMessage(SB_SETICON, iPart, (LPARAM)hIcon); }
	void       doResize(WPARAM wp, LPARAM lp)          { if(wp != SIZE_MINIMIZED && _sb.hWnd()) _putParts(LOWORD(lp)); }

private:
	struct _Part { BYTE sizePixels; float resizeWeight; };
	void _putParts(int cx);

	Window       _sb;
	Array<_Part> _parts;
	int          _lastInsertedPart;
	Array<int>   _rightEdges;
};