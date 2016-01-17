
#include "ProgressBar.h"
#include <CommCtrl.h>

ProgressBar::ProgressBar()
	: _hWnd(nullptr)
{
}

ProgressBar::ProgressBar(HWND hwnd)
	: _hWnd(hwnd)
{
}

ProgressBar& ProgressBar::operator=(HWND hwnd)
{
	_hWnd = hwnd;
	return *this;
}

HWND ProgressBar::hWnd() const
{
	return _hWnd;
}

ProgressBar& ProgressBar::create(HWND hParent, int id, POINT pos, SIZE size)
{
	return operator=( CreateWindowEx(0, PROGRESS_CLASS, nullptr,
		WS_CHILD | WS_VISIBLE,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

ProgressBar& ProgressBar::setRange(int minVal, int maxVal)
{
	SendMessage(_hWnd, PBM_SETRANGE, 0, MAKELPARAM(minVal, maxVal));
	return *this;
}

ProgressBar& ProgressBar::setRange(int minVal, size_t maxVal)
{
	return setRange(minVal, static_cast<int>(maxVal));
}

ProgressBar& ProgressBar::setPos(int pos)
{
	SendMessage(_hWnd, PBM_SETPOS, pos, 0);
	return *this;
}

ProgressBar& ProgressBar::setPos(size_t pos)
{
	return setPos(static_cast<int>(pos));
}

ProgressBar& ProgressBar::setPos(double pos)
{
	return setPos(static_cast<int>(pos + 0.5));
}

ProgressBar& ProgressBar::setWaiting(bool isWaiting)
{
	if (isWaiting) {
		SetWindowLongPtr(_hWnd, GWL_STYLE, // set this on resource editor won't work
			GetWindowLongPtr(_hWnd, GWL_STYLE) | PBS_MARQUEE);
	}

	SendMessage(_hWnd, PBM_SETMARQUEE, static_cast<WPARAM>(isWaiting), 0);

	// http://stackoverflow.com/questions/23686724/how-to-reset-marquee-progress-bar
	if (!isWaiting) {
		SetWindowLongPtr(_hWnd, GWL_STYLE,
			GetWindowLongPtr(_hWnd, GWL_STYLE) & ~PBS_MARQUEE);
	}

	return *this;
}

int ProgressBar::getPos()
{
	return static_cast<int>(SendMessage(_hWnd, PBM_GETPOS, 0, 0));
}