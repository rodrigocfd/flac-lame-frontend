/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_msgs.h"

/**
 * base_wnd <-- base_msgs <-- msg_command
 */

namespace wl {

// Adds on_command() method to handle WM_COMMAND messages.
class msg_command : virtual public base::msgs {
public:
	using funcT = base::inventory<WORD>::funcT;
private:
	base::inventory<WORD> _cmdInventory;

protected:
	msg_command(size_t msgsReserve = 0) : _cmdInventory(msgsReserve) {
		this->on_message(WM_COMMAND, [&](params& p)->LONG_PTR {
			funcT* pFunc = this->_cmdInventory.find(LOWORD(p.wParam));
			return pFunc ? (*pFunc)(p) : this->default_proc(p);
		});
	}

public:
	void on_command(WORD cmd, funcT func) {
		this->_cmdInventory.add(cmd, std::move(func));
	}

	void on_command(std::initializer_list<WORD> cmds, funcT func) {
		this->_cmdInventory.add(cmds, std::move(func));
	}
};

}//namespace wl