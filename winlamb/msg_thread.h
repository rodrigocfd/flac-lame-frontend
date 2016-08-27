/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "wnd_proc.h"
#include "traits_window.h"
#include "traits_dialog.h"

/**
 *                     +-- msg_thread<traits_window> <-- window_msg_thread
 * wnd <-- wnd_proc <--+
 *                     +-- msg_thread<traits_dialog> <-- dialog_msg_thread
 */

namespace winlamb {

template<typename traitsT, UINT wm_threadT = WM_APP-1>
class msg_thread : virtual public wnd_proc<traitsT> {
public:
	typedef std::function<void()> func_thread_type;

private:
	struct _callback_pack final {
		func_thread_type callback;
	};

protected:
	msg_thread()
	{
		this->wnd_proc::on_message(wm_threadT, [this](params p)->typename traitsT::ret_type {
			_callback_pack* pack = reinterpret_cast<_callback_pack*>(p.lParam);
			pack->callback();
			delete pack;
			return traitsT::default_proc(this->wnd::hwnd(), wm_threadT, p.wParam, p.lParam);
		});
	}

public:
	virtual ~msg_thread() = default;

	void ui_thread(func_thread_type callback)
	{
		// This method is analog to SendMessage (synchronous), but intended to be called from another
		// thread, so a callback function can, tunelled by wndproc, run in the original thread of the
		// window, thus allowing GUI updates. This avoids the user to deal with a custom WM_ message.
		_callback_pack* pack = new _callback_pack{ std::move(callback) };
		SendMessage(this->wnd::hwnd(), wm_threadT, 0, reinterpret_cast<LPARAM>(pack));
	}
};

typedef msg_thread<traits_window> window_msg_thread;
typedef msg_thread<traits_dialog> dialog_msg_thread;

}//namespace winlamb