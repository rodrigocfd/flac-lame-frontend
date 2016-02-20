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
 * msg_command
 *  wnd_proc
 *   wnd
 */

namespace winlamb {

template<typename traitsT>
class msg_command : virtual public wnd_proc<traitsT> {
public:
	typedef std::function<typename traitsT::ret_type()> command_func_type;

private:
	struct _command_unit final {
		WORD commandId;
		command_func_type callback;
	};
	std::vector<_command_unit> _commands;

protected:
	msg_command()
	{
		on_message(WM_COMMAND, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			for (const auto& cmd : _commands) {
				if (cmd.commandId == LOWORD(wp)) {
					return cmd.callback();
				}
			}
			return traitsT::default_proc(hwnd(), WM_COMMAND, wp, lp);
		});
	}

public:
	virtual ~msg_command() = default;

	void on_command(WORD commandId, command_func_type callback)
	{
		for (auto& cmd : _commands) {
			if (cmd.commandId == commandId) {
				cmd.callback = std::move(callback); // replace existing
				return;
			}
		}
		_commands.push_back({ commandId, std::move(callback) }); // add new WM_COMMAND handler
	}

	void on_command(std::initializer_list<WORD> commandIds, command_func_type callback)
	{
		on_command(*commandIds.begin(), std::move(callback)); // store 1st message once
		size_t m0 = _commands.size() - 1;

		for (size_t i = 1; i < commandIds.size(); ++i) {
			if (*(commandIds.begin() + i) != *commandIds.begin()) { // avoid overwriting
				on_command(*(commandIds.begin() + i), [this, m0]()->typename traitsT::ret_type {
					return _commands[m0].callback(); // store light wrapper to 1st message
				});
			}
		}
	}
};

typedef msg_command<traits_window> msg_command_window;
typedef msg_command<traits_dialog> msg_command_dialog;

}//namespace winlamb