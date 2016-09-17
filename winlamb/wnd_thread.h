/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "wnd_proc.h"

/**
 * wnd <-- wnd_proc <-- wnd_thread
 */

namespace winlamb {

template<
	typename traitsT,
	UINT wm_threadT = WM_APP + 0x3FFF
>
class wnd_thread : virtual public wnd_proc<traitsT> {
private:
	struct _callback_pack final {
		std::function<void()> callback;
	};

public:
	virtual ~wnd_thread() = default;

protected:
	wnd_thread()
	{
		this->wnd_proc::on_message(wm_threadT, [this](wnd_proc::params p)->typename traitsT::ret_type {
			_callback_pack* pack = reinterpret_cast<_callback_pack*>(p.lParam);
			pack->callback();
			delete pack;
			return traitsT::default_proc(this->wnd::hwnd(), wm_threadT, p.wParam, p.lParam);
		});
	}

	void on_ui_thread(std::function<void()> callback)
	{
		// This method is analog to SendMessage (synchronous), but intended to be called from another
		// thread, so a callback function can, tunelled by wndproc, run in the original thread of the
		// window, thus allowing GUI updates. This avoids the user to deal with a custom WM_ message.
		_callback_pack* pack = new _callback_pack{ std::move(callback) };
		SendMessage(this->wnd::hwnd(), wm_threadT, 0, reinterpret_cast<LPARAM>(pack));
	}
};

}//namespace winlamb