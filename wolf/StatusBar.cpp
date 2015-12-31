/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "StatusBar.h"
using namespace wolf;
using std::wstring;

StatusBar::StatusBar(WindowTopLevel *parent)
	: _parent(*parent)
{
}

StatusBar& StatusBar::addFixedPart(UINT sizePixels)
{
	this->_parts.push_back({ sizePixels, 0 });
	this->_rightEdges.emplace_back(0);
	this->_createOnce();
	this->_putParts();
	return *this;
}

StatusBar& StatusBar::addResizablePart(UINT resizeWeight)
{
	// How resizeWeight works:
	// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
	// If available client area is 400px, respective part widths will be 100, 100 and 200px.
	// Zero weight means a fixed-width part, which therefore should have sizePixels set (otherwise zero).
	this->_parts.push_back({ 0, resizeWeight });
	this->_rightEdges.emplace_back(0);
	this->_createOnce();
	this->_putParts();
	return *this;
}

StatusBar& StatusBar::setText(const wchar_t *text, int iPart)
{
	this->sendMessage(SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0),
		reinterpret_cast<LPARAM>(text));
	return *this;
}

StatusBar& StatusBar::setText(const wstring& text, int iPart)
{
	return this->setText(text.c_str(), iPart);
}

wstring StatusBar::getText(int iPart) const
{
	int len = LOWORD(this->sendMessage(SB_GETTEXTLENGTH, iPart, 0)) + 1;
	wstring buf(len, L'\0');
	this->sendMessage(SB_GETTEXT, iPart, reinterpret_cast<LPARAM>(&buf[0]));
	buf.resize(len);
	return buf;
}

StatusBar& StatusBar::setIcon(HICON hIcon, int iPart)
{
	this->sendMessage(SB_SETICON, iPart, reinterpret_cast<LPARAM>(hIcon));
	return *this;
}

void StatusBar::_putParts()
{
	for (auto i = 0U; i < this->_parts.size(); ++i) {
		this->_rightEdges[i] = i * 10; // arbitrary, just to force parts creation, will be fixed on WM_SIZE
	}
	this->sendMessage(SB_SETPARTS, this->_rightEdges.size(),
		reinterpret_cast<LPARAM>(&this->_rightEdges[0]));
}

void StatusBar::_createOnce()
{
	if (this->hWnd()) {
		return;
	}

	DWORD parentStyle = static_cast<DWORD>(GetWindowLongPtr(this->_parent.hWnd(), GWL_STYLE));
	bool isStretch = (parentStyle & WS_MAXIMIZEBOX) != 0 ||
		(parentStyle & WS_SIZEBOX) != 0;

	this->Window::operator=( CreateWindowEx(0, STATUSCLASSNAME, nullptr,
		(WS_CHILD | WS_VISIBLE) | (isStretch ? SBARS_SIZEGRIP : 0),
		0, 0, 0, 0, this->_parent.hWnd(), nullptr, this->_parent.hInst(), nullptr) );

	this->_parent.onMessage(WM_SIZE, [this](WPARAM wp, LPARAM lp)->LRESULT { // equivalent of subclass parent
		if (wp != SIZE_MINIMIZED && this->hWnd()) {
			int cx = LOWORD(lp); // available width
			this->sendMessage(WM_SIZE, 0, 0); // tell statusbar to fit parent

			// Find the space to be divided among variable-width parts,
			// and total weight of variable-width parts.
			UINT totalWeight = 0;
			int  cxVariable = cx;
			for (const Part& part : this->_parts) {
				if (!part.resizeWeight) { // fixed-width?
					cxVariable -= part.sizePixels;
				} else {
					totalWeight += part.resizeWeight;
				}
			}

			// Fill right edges array with the right edge of each part.
			int cxTotal = cx;
			for (auto i = this->_parts.size(); i-- > 0; ) {
				this->_rightEdges[i] = cxTotal;
				cxTotal -= (!this->_parts[i].resizeWeight) ? // fixed-width?
					this->_parts[i].sizePixels :
					static_cast<int>( (cxVariable / totalWeight) * this->_parts[i].resizeWeight );
			}
			this->sendMessage(SB_SETPARTS, this->_rightEdges.size(),
				reinterpret_cast<LPARAM>(&this->_rightEdges[0]));
		}
		return 0;
	});
}