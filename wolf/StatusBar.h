/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowTopLevel.h"

namespace wolf {

class StatusBar final : public Window {
private:
	struct Part final { UINT sizePixels; UINT resizeWeight; };
private:
	WindowTopLevel&   _parent;
	std::vector<Part> _parts;
	std::vector<int>  _rightEdges;
public:
	explicit StatusBar(WindowTopLevel *parent);
	StatusBar&   operator=(const StatusBar& sb) = delete;
	StatusBar&   operator=(StatusBar&& sb) = delete;
	StatusBar&   addFixedPart(UINT sizePixels);
	StatusBar&   addResizablePart(UINT resizeWeight);
	StatusBar&   setText(const wchar_t *text, int iPart);
	StatusBar&   setText(const std::wstring& text, int iPart);
	std::wstring getText(int iPart) const;
	StatusBar&   setIcon(HICON hIcon, int iPart);
private:
	void _putParts();
	void _createOnce();
	Window::getText;
	Window::setText;
};


}//namespace wolf