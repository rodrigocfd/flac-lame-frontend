
#pragma once
#include <utility>
#include <Windows.h>

class ProgressBar final {
private:
	HWND _hWnd;
public:
	ProgressBar();
	ProgressBar(HWND hwnd);
	ProgressBar& operator=(HWND hwnd);
	ProgressBar& operator=(std::pair<HWND, int> hWndAndCtrlId);

	HWND         hWnd() const;
	ProgressBar& create(HWND hParent, int id, POINT pos, SIZE size);
	ProgressBar& setRange(int minVal, int maxVal);
	ProgressBar& setRange(int minVal, size_t maxVal);
	ProgressBar& setPos(int pos);
	ProgressBar& setPos(size_t pos);
	ProgressBar& setPos(double pos);
	ProgressBar& setWaiting(bool isWaiting);
	int          getPos();
};