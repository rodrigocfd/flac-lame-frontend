/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <functional>
#include "base_wnd_loop.h"
#include "params.h"

/**
 * base_wnd <-- base_wnd_loop <-- base_wnd_thread
 */

namespace wet {

class base_wnd_thread : public base_wnd_loop {
private:
	struct _callback_pack final {
		std::function<void()> callback;
	};

protected:
	static const UINT WM_THREAD_MESSAGE = WM_APP + 0x3FFF;
	base_wnd_thread() = default;

	void _process_thread(params p) const {
		_callback_pack* pack = reinterpret_cast<_callback_pack*>(p.lParam);
		pack->callback();
		delete pack;
	}

	void ui_thread(std::function<void()> callback) const {
		// This method is analog to SendMessage (synchronous), but intended to be called from another
		// thread, so a callback function can, tunelled by wndproc, run in the original thread of the
		// window, thus allowing GUI updates. This avoids the user to deal with a custom WM_ message.
		_callback_pack* pack = new _callback_pack{ std::move(callback) };
		SendMessageW(this->base_wnd::hwnd(), WM_THREAD_MESSAGE, 0, reinterpret_cast<LPARAM>(pack));
	}
};

}//namespace wet