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
 *                     +-- msg_getdlgcode<traits_window> <-- window_msg_getdlgcode
 * wnd <-- wnd_proc <--+
 *                     +-- msg_getdlgcode<traits_dialog> <-- dialog_msg_getdlgcode
 */

namespace winlamb {

template<typename traitsT>
class msg_getdlgcode : virtual public wnd_proc<traitsT> {
public:
	struct params_getdlgcode : public params {
		params_getdlgcode(const params& p) : params(p) { }
		WORD virt_key_code() const         { return this->params::wParam; }
		MSG* system_msg() const            { return reinterpret_cast<MSG*>(this->params::lParam); }
	};
	typedef std::function<typename traitsT::ret_type(params_getdlgcode)> func_getdlgcode_type;

private:
	callback_depot<WORD, func_getdlgcode_type, params_getdlgcode, traitsT> _callbacks;

protected:
	msg_getdlgcode()
	{
		this->wnd_proc::on_message(WM_GETDLGCODE, [this](params p)->typename traitsT::ret_type {
			params_getdlgcode pg(p);
			return pg.system_msg() ? // when pointer to MSG is null, system is performing a query; bypass
				this->_callbacks.process(this->wnd::hwnd(), WM_GETDLGCODE, pg.virt_key_code(), pg) :
				traitsT::default_proc(this->wnd::hwnd(), WM_GETDLGCODE, p.wParam, p.lParam);
		});
	}

public:
	virtual ~msg_getdlgcode() = default;

	void on_getdlgcode(WORD virtKeyCode, func_getdlgcode_type callback)
	{
		this->_callbacks.add(virtKeyCode, std::move(callback));
	}

	void on_getdlgcode(std::initializer_list<WORD> virtKeyCodes, func_getdlgcode_type callback)
	{
		this->_callbacks.add(virtKeyCodes, std::move(callback));
	}
};

typedef msg_getdlgcode<traits_window> window_msg_getdlgcode;
typedef msg_getdlgcode<traits_dialog> dialog_msg_getdlgcode;

}//namespace winlamb