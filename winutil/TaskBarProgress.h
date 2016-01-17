
#pragma once
#include <Windows.h>
#include <ShObjIdl.h>

class TaskBarProgress final {
private:
	HWND           _hWnd;
	ITaskbarList3 *_bar;
public:
	~TaskBarProgress();
	TaskBarProgress();
	TaskBarProgress& operator<<(HWND hWnd);

	TaskBarProgress& setPos(size_t percent, size_t total);
	TaskBarProgress& setPos(double percent);
	TaskBarProgress& setWaiting(bool isWaiting);
	TaskBarProgress& setPause(bool isPaused);
	TaskBarProgress& setError(bool hasError);
	TaskBarProgress& dismiss();
private:
	TaskBarProgress& _setState(TBPFLAG state);
};