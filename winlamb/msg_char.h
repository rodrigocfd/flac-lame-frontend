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
 *                     +-- msg_char<traits_window> <-- window_msg_char
 * wnd <-- wnd_proc <--+
 *                     +-- msg_char<traits_dialog> <-- dialog_msg_char
 */

namespace winlamb {

template<typename traitsT>
class msg_char : virtual public wnd_proc<traitsT> {
public:
	struct params_char : public params {
		params_char(const params& p)        : params(p) { }
		WORD char_code() const              { return LOWORD(this->params::wParam); }
		WORD repeat_count() const           { return LOWORD(this->params::lParam); }
		bool is_extended_key() const        { return (this->params::lParam & (1 << 24)) != 0; }
		bool is_alt_key_down() const        { return (this->params::lParam & (1 << 29)) != 0; }
		bool is_previous_state_down() const { return (this->params::lParam & (1 << 30)) != 0; }
		bool is_being_released() const      { return (this->params::lParam & (1 << 31)) != 0; }
	};
	typedef std::function<typename traitsT::ret_type(params_char)> func_char_type;

private:
	callback_depot<WORD, func_char_type, params_char, traitsT> _callbacks;

protected:
	msg_char()
	{
		this->wnd_proc::on_message(WM_CHAR, [this](params p)->typename traitsT::ret_type {
			params_char pk(p);
			return this->_callbacks.process(this->wnd::hwnd(), WM_CHAR, pk.char_code(), pk);
		});
	}

public:
	virtual ~msg_char() = default;

	void on_char(WORD charCode, func_char_type callback)
	{
		this->_callbacks.add(charCode, std::move(callback));
	}

	void on_char(std::initializer_list<WORD> charCodes, func_char_type callback)
	{
		this->_callbacks.add(charCodes, std::move(callback));
	}
};

typedef msg_char<traits_window> window_msg_char;
typedef msg_char<traits_dialog> dialog_msg_char;

}//namespace winlamb