/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "inventory.h"
#include "base_wnd.h"

namespace wl {
namespace internals {

class threaded final {
public:
	using funcT = std::function<void()>;
	static const UINT WM_THREAD_MESSAGE = WM_APP + 0x3FFF;
private:
	struct _callback_pack final {
		funcT func;
	};

	const base_wnd& _wnd;
	LONG_PTR _processedVal;

public:
	threaded(const base_wnd& w, inventory& theInventory, LONG_PTR processedVal) :
		_wnd(w), _processedVal(processedVal)
	{
		theInventory.add_message(WM_THREAD_MESSAGE, [&](const params& p)->LONG_PTR {
			this->_process_threaded(p);
			return this->_processedVal;
		});
	}

	void ui_thread(funcT func) const {
		// This method is analog to SendMessage (synchronous), but intended to be called from another
		// thread, so a callback function can, tunelled by wndproc, run in the original thread of the
		// window, thus allowing GUI updates. This avoids the user to deal with a custom WM_ message.
		_callback_pack* pack = new _callback_pack{ std::move(func) };
		SendMessageW(this->_wnd.hwnd(), WM_THREAD_MESSAGE, 0, reinterpret_cast<LPARAM>(pack));
	}

private:
	void _process_threaded(const params& p) const {
		_callback_pack* pack = reinterpret_cast<_callback_pack*>(p.lParam);
		pack->func();
		delete pack;
	}
};

}//namespace internals
}//namespace wl