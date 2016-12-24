/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd.h"
#include <ShObjIdl.h>

namespace wet {

class progress_taskbar final {
private:
	base_wnd       _wnd;
	ITaskbarList3* _bar;

public:
	progress_taskbar() : _bar(nullptr) { }
	progress_taskbar& operator=(const progress_taskbar&) = delete;
	progress_taskbar& operator=(progress_taskbar&&) = delete;

	~progress_taskbar() {
		if (this->_bar) {
			this->_bar->Release();
			CoUninitialize();
		}
	}

	progress_taskbar& init(HWND hWnd) {
		if (this->_bar) {
			DBG(L"ERROR: progress_taskbar already created.\n");
		} else {
			this->_wnd = hWnd;
			CoInitialize(nullptr);
			CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
				IID_ITaskbarList3, reinterpret_cast<LPVOID*>(&this->_bar));
		}
		return *this;
	}

	progress_taskbar& init(const base_wnd* wnd) {
		return this->init(wnd->hwnd());
	}

	progress_taskbar& set_pos(size_t percent, size_t total) {
		this->_bar->SetProgressValue(this->_wnd.hwnd(), static_cast<ULONGLONG>(percent),
			static_cast<ULONGLONG>(total));
		return *this;
	}

	progress_taskbar& set_pos(double percent)     { return this->set_pos(static_cast<size_t>(percent + 0.5), 100); }
	progress_taskbar& set_waiting(bool isWaiting) { return this->_set_state(isWaiting ? TBPF_INDETERMINATE : TBPF_NORMAL); }
	progress_taskbar& set_pause(bool isPaused)    { return this->_set_state(isPaused ? TBPF_PAUSED : TBPF_NORMAL); }
	progress_taskbar& set_error(bool hasError)    { return this->_set_state(hasError ? TBPF_ERROR : TBPF_NORMAL); }
	progress_taskbar& clear()                     { return this->_set_state(TBPF_NOPROGRESS); }

private:
	progress_taskbar& _set_state(TBPFLAG state) {
		this->_bar->SetProgressState(this->_wnd.hwnd(), state);
		return *this;
	}
};

}//namespace wet