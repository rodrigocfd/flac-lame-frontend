
#pragma once
#include <Windows.h>
#include <ShObjIdl.h>

class TaskBarProgress final {
private:
	HWND           _hWnd;
	ITaskbarList3 *_bar;
public:
	~TaskBarProgress();
	TaskBarProgress() : _hWnd(nullptr), _bar(nullptr) { }

	TaskBarProgress& init(HWND hWnd);
	TaskBarProgress& setPos(size_t percent, size_t total);
	TaskBarProgress& setPos(double percent)     { return setPos(static_cast<size_t>(percent + 0.5), 100); }
	TaskBarProgress& setWaiting(bool isWaiting) { return _setState(isWaiting ? TBPF_INDETERMINATE : TBPF_NORMAL); }
	TaskBarProgress& setPause(bool isPaused)    { return _setState(isPaused ? TBPF_PAUSED : TBPF_NORMAL); }
	TaskBarProgress& setError(bool hasError)    { return _setState(hasError ? TBPF_ERROR : TBPF_NORMAL); }
	TaskBarProgress& clear()                    { return _setState(TBPF_NOPROGRESS); }
private:
	TaskBarProgress& _setState(TBPFLAG state);
};