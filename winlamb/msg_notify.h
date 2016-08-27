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
 *                     +-- msg_notify<traits_window> <-- window_msg_notify
 * wnd <-- wnd_proc <--+
 *                     +-- msg_notify<traits_dialog> <-- dialog_msg_notify
 */

namespace winlamb {

template<typename traitsT>
class msg_notify : virtual public wnd_proc<traitsT> {
public:
	struct params_notify : public params {
		params_notify(const params& p) : params(p) { }
		NMHDR& nmhdr() const           { return *reinterpret_cast<NMHDR*>(this->params::lParam); }
	};
	typedef std::function<typename traitsT::ret_type(params_notify)> func_notify_type;

private:
	callback_depot<std::pair<UINT_PTR, UINT>, func_notify_type, params_notify, traitsT> _callbacks; // idFrom, code

protected:
	msg_notify()
	{
		this->wnd_proc::on_message(WM_NOTIFY, [this](params p)->typename traitsT::ret_type {
			params_notify pn(p);
			return this->_callbacks.process(this->wnd::hwnd(), WM_NOTIFY,
				{ pn.nmhdr().idFrom, pn.nmhdr().code }, pn);
		});
	}

public:
	virtual ~msg_notify() = default;

	void on_notify(UINT_PTR idFrom, UINT code, func_notify_type callback)
	{
		this->_callbacks.add(std::make_pair(idFrom, code), std::move(callback));
	}

	void on_notify(std::initializer_list<std::pair<UINT_PTR, UINT>> idFromAndCodes, func_notify_type callback)
	{
		this->_callbacks.add(idFromAndCodes, std::move(callback));
	}
};

typedef msg_notify<traits_window> window_msg_notify;
typedef msg_notify<traits_dialog> dialog_msg_notify;

}//namespace winlamb