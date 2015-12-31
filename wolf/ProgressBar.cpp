/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "ProgressBar.h"
using namespace wolf;

ProgressBar::ProgressBar()
{
}

ProgressBar::ProgressBar(HWND hwnd)
	: Window(hwnd)
{
}

ProgressBar::ProgressBar(Window&& w)
	: Window(std::move(w))
{
}

ProgressBar& ProgressBar::operator=(HWND hwnd)
{
	this->Window::operator=(hwnd);
	return *this;
}

ProgressBar& ProgressBar::operator=(Window&& w)
{
	this->Window::operator=(std::move(w));
	return *this;
}

ProgressBar& ProgressBar::create(const WindowParent *parent, int id, POINT pos, SIZE size)
{
	return this->create(parent->hWnd(), id, pos, size);
}

ProgressBar& ProgressBar::create(HWND hParent, int id, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(0, PROGRESS_CLASS, nullptr,
		WS_CHILD | WS_VISIBLE,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
	return *this;
}

ProgressBar& ProgressBar::setRange(int minVal, int maxVal)
{
	this->sendMessage(PBM_SETRANGE, 0, MAKELPARAM(minVal, maxVal));
	return *this;
}

ProgressBar& ProgressBar::setRange(int minVal, size_t maxVal)
{
	return this->setRange(minVal, static_cast<int>(maxVal));
}

ProgressBar& ProgressBar::setPos(int pos)
{
	this->sendMessage(PBM_SETPOS, pos, 0);
	return *this;
}

ProgressBar& ProgressBar::setPos(size_t pos)
{
	return this->setPos(static_cast<int>(pos));
}

ProgressBar& ProgressBar::setPos(double pos)
{
	return this->setPos(static_cast<int>(pos + 0.5));
}

ProgressBar& ProgressBar::setWaiting(bool isWaiting)
{
	if (isWaiting) {
		SetWindowLongPtr(this->hWnd(), GWL_STYLE, // set this on resource editor won't work
			GetWindowLongPtr(this->hWnd(), GWL_STYLE) | PBS_MARQUEE);
	}

	this->sendMessage(PBM_SETMARQUEE, static_cast<WPARAM>(isWaiting), 0);

	// http://stackoverflow.com/questions/23686724/how-to-reset-marquee-progress-bar
	if (!isWaiting) {
		SetWindowLongPtr(this->hWnd(), GWL_STYLE,
			GetWindowLongPtr(this->hWnd(), GWL_STYLE) & ~PBS_MARQUEE);
	}

	return *this;
}

int ProgressBar::getPos()
{
	return static_cast<int>(this->sendMessage(PBM_GETPOS, 0, 0));
}