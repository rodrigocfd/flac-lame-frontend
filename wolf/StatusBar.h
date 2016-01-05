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
	StatusBar&   setText(const wchar_t *text, size_t iPart);
	StatusBar&   setText(const std::wstring& text, size_t iPart);
	std::wstring getText(size_t iPart) const;
	StatusBar&   setIcon(HICON hIcon, size_t iPart);
private:
	void _putParts();
	void _createOnce();
	Window::getText;
	Window::setText;
};


}//namespace wolf