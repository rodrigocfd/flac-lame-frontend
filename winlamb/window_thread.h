/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window_proc.h"

/**
 * window_thread
 *  window_proc
 *   window
 */

namespace winlamb {

template<typename traitsT, UINT wm_threadT = WM_APP-1>
class window_thread : public window_proc<traitsT> {
public:
	typedef std::function<void()> thread_func_type;
private:
	struct _callback_pack final {
		thread_func_type callback;
	};

protected:
	window_thread()
	{
		on_message(wm_threadT, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			_callback_pack *pack = reinterpret_cast<_callback_pack*>(lp);
			pack->callback();
			delete pack;
			return traitsT::default_proc(hwnd(), wm_threadT, wp, lp);
		});
	}

public:
	virtual ~window_thread() = default;

	void gui_thread(thread_func_type callback)
	{
		// This method is analog to SendMessage (synchronous), but intended to be called from another
		// thread, so a callback function can, tunelled by wndproc, run in the original thread of the
		// window, thus allowing GUI updates. This avoids the user to deal with a custom WM_ message.
		_callback_pack *pack = new _callback_pack{ std::move(callback) };
		SendMessage(hwnd(), wm_threadT, 0, reinterpret_cast<LPARAM>(pack));
	}
};

}//namespace winlamb