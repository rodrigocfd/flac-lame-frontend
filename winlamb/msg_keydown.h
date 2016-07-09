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
 * msg_keydown
 *  wnd_proc
 *   wnd
 */

namespace winlamb {

template<typename traitsT>
class msg_keydown : virtual public wnd_proc<traitsT> {
public:
	struct params_keydown : public params {
		params_keydown(const params& p)     : params(p) { }
		WORD virt_key_code() const          { return LOWORD(wParam); }
		WORD repeat_count() const           { return LOWORD(lParam); }
		bool is_extended_key() const        { return (lParam & (1 << 24)) != 0; }
		bool is_previous_state_down() const { return (lParam & (1 << 30)) != 0; }
	};
	typedef std::function<typename traitsT::ret_type(params_keydown)> func_keydown_type;

private:
	callback_depot<WORD, func_keydown_type, params_keydown, traitsT> _callbacks;

protected:
	msg_keydown()
	{
		on_message(WM_KEYDOWN, [this](params p)->typename traitsT::ret_type {
			params_keydown pk(p);
			return _callbacks.process(hwnd(), WM_KEYDOWN, pk.virt_key_code(), pk);
		});
	}

public:
	virtual ~msg_keydown() = default;

	void on_keydown(WORD virtKeyCode, func_keydown_type callback)
	{
		_callbacks.add(virtKeyCode, std::move(callback));
	}

	void on_keydown(std::initializer_list<WORD> virtKeyCodes, func_keydown_type callback)
	{
		_callbacks.add(virtKeyCodes, std::move(callback));
	}
};

typedef msg_keydown<traits_window> window_msg_keydown;
typedef msg_keydown<traits_dialog> dialog_msg_keydown;

}//namespace winlamb