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
 *                     +-- msg_command<traits_window> <-- window_msg_command
 * wnd <-- wnd_proc <--+
 *                     +-- msg_command<traits_dialog> <-- dialog_msg_command
 */

namespace winlamb {

template<typename traitsT>
class msg_command : virtual public wnd_proc<traitsT> {
public:
	struct params_command : public params {
		params_command(const params& p)  : params(p) { }
		WORD control_id() const          { return LOWORD(this->params::wParam); }
		HWND control_hwnd() const        { return reinterpret_cast<HWND>(this->params::lParam); }
		bool is_from_menu() const        { return HIWORD(this->params::wParam) == 0; }
		bool is_from_accelerator() const { return HIWORD(this->params::wParam) == 1; }
	};
	typedef std::function<typename traitsT::ret_type(params_command)> func_command_type;

private:
	callback_depot<WORD, func_command_type, params_command, traitsT> _callbacks;

protected:
	msg_command()
	{
		this->wnd_proc::on_message(WM_COMMAND, [this](params p)->typename traitsT::ret_type {
			params_command pc(p);
			return this->_callbacks.process(this->wnd::hwnd(), WM_COMMAND, pc.control_id(), pc);
		});
	}

public:
	virtual ~msg_command() = default;

	void on_command(WORD commandId, func_command_type callback)
	{
		this->_callbacks.add(commandId, std::move(callback));
	}

	void on_command(std::initializer_list<WORD> commandIds, func_command_type callback)
	{
		this->_callbacks.add(commandIds, std::move(callback));
	}
};

typedef msg_command<traits_window> window_msg_command;
typedef msg_command<traits_dialog> dialog_msg_command;

}//namespace winlamb