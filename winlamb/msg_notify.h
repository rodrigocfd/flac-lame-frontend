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

// Adds on_notify() method to handle WM_NOTIFY messages.
class msg_notify : virtual public base::msgs {
public:
	using idT = std::pair<UINT_PTR, UINT>;
	using funcT = base::inventory<idT>::funcT;
private:
	base::inventory<idT> _nfyInventory;

protected:
	msg_notify(size_t msgsReserve = 0) : _nfyInventory(msgsReserve) {
		this->on_message(WM_NOTIFY, [&](params& p)->LONG_PTR {
			funcT* pFunc = this->_nfyInventory.find({
				reinterpret_cast<NMHDR*>(p.lParam)->idFrom,
				reinterpret_cast<NMHDR*>(p.lParam)->code
			});
			return pFunc ? (*pFunc)(p) : this->default_proc(p);
		});
	}

public:
	void on_notify(idT idFromAndCode, funcT func) {
		this->_nfyInventory.add(idFromAndCode, std::move(func));
	}

	void on_notify(UINT_PTR idFrom, UINT code, funcT func) {
		this->_nfyInventory.add({idFrom, code}, std::move(func));
	}

	void on_notify(std::initializer_list<idT> idFromAndCodes, funcT func) {
		this->_nfyInventory.add(idFromAndCodes, std::move(func));
	}
};

}//namespace wl