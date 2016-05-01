
#include "TaskBarProgress.h"

TaskBarProgress::~TaskBarProgress()
{
	if (_bar) {
		_bar->Release();
		CoUninitialize();
	}
}

TaskBarProgress& TaskBarProgress::init(HWND hWnd)
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

TaskBarProgress& TaskBarProgress::_setState(TBPFLAG state)
{
	_bar->SetProgressState(_hWnd, state);
	return *this;
}