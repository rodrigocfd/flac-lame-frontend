
#include "TaskBarProgress.h"

TaskBarProgress::~TaskBarProgress()
{
	if (_bar) {
		_bar->Release();
		CoUninitialize();
	}
}

TaskBarProgress::TaskBarProgress()
	: _hWnd(nullptr), _bar(nullptr)
{
}

TaskBarProgress& TaskBarProgress::operator<<(HWND hWnd)
{
	_hWnd = hWnd;
	CoInitialize(nullptr);
	CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
		IID_ITaskbarList3, reinterpret_cast<LPVOID*>(&_bar));
	return *this;
}

TaskBarProgress& TaskBarProgress::setPos(size_t percent, size_t total)
{
	_bar->SetProgressValue(_hWnd, static_cast<ULONGLONG>(percent),
		static_cast<ULONGLONG>(total));
	return *this;
}

TaskBarProgress& TaskBarProgress::setPos(double percent)
{
	return setPos(static_cast<size_t>(percent + 0.5), 100); // round
}

TaskBarProgress& TaskBarProgress::setWaiting(bool isWaiting)
{
	return _setState(isWaiting ? TBPF_INDETERMINATE : TBPF_NORMAL);
}

TaskBarProgress& TaskBarProgress::setPause(bool isPaused)
{
	return _setState(isPaused ? TBPF_PAUSED : TBPF_NORMAL);
}

TaskBarProgress& TaskBarProgress::setError(bool hasError)
{
	return _setState(hasError ? TBPF_ERROR : TBPF_NORMAL);
}

TaskBarProgress& TaskBarProgress::dismiss()
{
	return _setState(TBPF_NOPROGRESS);
}

TaskBarProgress& TaskBarProgress::_setState(TBPFLAG state)
{
	_bar->SetProgressState(_hWnd, state);
	return *this;
}