/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "progressbar.h"
#include <CommCtrl.h>
using namespace winutil;

progressbar& progressbar::operator=(HWND hwnd)
{
	_hWnd = hwnd;
	return *this;
}

progressbar& progressbar::create(HWND hParent, int id, POINT pos, SIZE size)
{
	return operator=( CreateWindowEx(0, PROGRESS_CLASS, nullptr,
		WS_CHILD | WS_VISIBLE,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

progressbar& progressbar::set_range(int minVal, int maxVal)
{
	SendMessage(_hWnd, PBM_SETRANGE, 0, MAKELPARAM(minVal, maxVal));
	return *this;
}

progressbar& progressbar::set_pos(int pos)
{
	SendMessage(_hWnd, PBM_SETPOS, pos, 0);
	return *this;
}

progressbar& progressbar::set_waiting(bool isWaiting)
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

int progressbar::get_pos()
{
	return static_cast<int>(SendMessage(_hWnd, PBM_GETPOS, 0, 0));
}