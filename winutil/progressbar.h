/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <utility>
#include <Windows.h>

namespace winutil {

class progressbar final {
private:
	HWND _hWnd;
public:
	progressbar()          : _hWnd(nullptr) { }
	progressbar(HWND hwnd) : _hWnd(hwnd) { }
	progressbar& operator=(HWND hwnd);

	HWND         hwnd() const                         { return _hWnd; }
	progressbar& create(HWND hParent, int id, POINT pos, SIZE size);
	progressbar& set_range(int minVal, int maxVal);
	progressbar& set_range(int minVal, size_t maxVal) { return set_range(minVal, static_cast<int>(maxVal)); }
	progressbar& set_pos(int pos);
	progressbar& set_pos(size_t pos)                  { return set_pos(static_cast<int>(pos)); }
	progressbar& set_pos(double pos)                  { return set_pos(static_cast<int>(pos + 0.5)); }
	progressbar& set_waiting(bool isWaiting);
	int          get_pos();
};

}//namespace winutil