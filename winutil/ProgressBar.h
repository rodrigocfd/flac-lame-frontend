
#pragma once
#include <utility>
#include <Windows.h>

class ProgressBar final {
private:
	HWND _hWnd;
public:
	ProgressBar()          : _hWnd(nullptr) { }
	ProgressBar(HWND hwnd) : _hWnd(hwnd) { }
	ProgressBar& operator=(HWND hwnd);

	HWND         hWnd() const { return _hWnd; }
	ProgressBar& create(HWND hParent, int id, POINT pos, SIZE size);
	ProgressBar& setRange(int minVal, int maxVal);
	ProgressBar& setRange(int minVal, size_t maxVal) { return setRange(minVal, static_cast<int>(maxVal)); }
	ProgressBar& setPos(int pos);
	ProgressBar& setPos(size_t pos) { return setPos(static_cast<int>(pos)); }
	ProgressBar& setPos(double pos) { return setPos(static_cast<int>(pos + 0.5)); }
	ProgressBar& setWaiting(bool isWaiting);
	int          getPos();
};