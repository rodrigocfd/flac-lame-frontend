/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <Windows.h>
#include <ShObjIdl.h>

namespace winutil {

class taskbar_progress final {
private:
	HWND           _hWnd;
	ITaskbarList3 *_bar;
public:
	~taskbar_progress();
	taskbar_progress() : _hWnd(nullptr), _bar(nullptr) { }

	taskbar_progress& init(HWND hWnd);
	taskbar_progress& set_pos(size_t percent, size_t total);
	taskbar_progress& set_pos(double percent)     { return set_pos(static_cast<size_t>(percent + 0.5), 100); }
	taskbar_progress& set_waiting(bool isWaiting) { return _set_state(isWaiting ? TBPF_INDETERMINATE : TBPF_NORMAL); }
	taskbar_progress& set_pause(bool isPaused)    { return _set_state(isPaused ? TBPF_PAUSED : TBPF_NORMAL); }
	taskbar_progress& set_error(bool hasError)    { return _set_state(hasError ? TBPF_ERROR : TBPF_NORMAL); }
	taskbar_progress& clear()                     { return _set_state(TBPF_NOPROGRESS); }
private:
	taskbar_progress& _set_state(TBPFLAG state);
};

}//namespace winutil