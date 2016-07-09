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
 * msg_keyup
 *  wnd_proc
 *   wnd
 */

namespace winlamb {

template<typename traitsT>
class msg_keyup : virtual public wnd_proc<traitsT> {
public:
	struct params_keyup : public params {
		params_keyup(const params& p) : params(p) { }
		WORD virt_key_code() const    { return LOWORD(wParam); }
		bool is_extended_key() const  { return (lParam & (1 << 24)) != 0; }
	};
	typedef std::function<typename traitsT::ret_type(params_keyup)> func_keyup_type;

private:
	callback_depot<WORD, func_keyup_type, params_keyup, traitsT> _callbacks;

protected:
	msg_keyup()
	{
		on_message(WM_KEYUP, [this](params p)->typename traitsT::ret_type {
			params_keyup pk(p);
			return _callbacks.process(hwnd(), WM_KEYUP, pk.virt_key_code(), pk);
		});
	}

public:
	virtual ~msg_keyup() = default;

	void on_keyup(WORD virtKeyCode, func_keyup_type callback)
	{
		_callbacks.add(virtKeyCode, std::move(callback));
	}

	void on_keyup(std::initializer_list<WORD> virtKeyCodes, func_keyup_type callback)
	{
		_callbacks.add(virtKeyCodes, std::move(callback));
	}
};

typedef msg_keyup<traits_window> window_msg_keyup;
typedef msg_keyup<traits_dialog> dialog_msg_keyup;

}//namespace winlamb