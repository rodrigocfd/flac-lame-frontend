/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "taskbar_progress.h"
using namespace winutil;

taskbar_progress::~taskbar_progress()
{
	if (_bar) {
		_bar->Release();
		CoUninitialize();
	}
}

taskbar_progress& taskbar_progress::init(HWND hWnd)
{
	_hWnd = hWnd;
	CoInitialize(nullptr);
	CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
		IID_ITaskbarList3, reinterpret_cast<LPVOID*>(&_bar));
	return *this;
}

taskbar_progress& taskbar_progress::set_pos(size_t percent, size_t total)
{
	_bar->SetProgressValue(_hWnd, static_cast<ULONGLONG>(percent),
		static_cast<ULONGLONG>(total));
	return *this;
}

taskbar_progress& taskbar_progress::_set_state(TBPFLAG state)
{
	_bar->SetProgressState(_hWnd, state);
	return *this;
}