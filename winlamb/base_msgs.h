/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_wnd.h"
#include "base_inventory.h"

/**
 * base_wnd <-- base_msgs
 */

namespace wl {

class subclass;

namespace base {

	class dialog;
	class window;
	class threaded;

	class msgs : virtual public wnd {
	public:
		friend subclass;
		friend dialog;
		friend window;
		friend threaded;

		using funcT = inventory<UINT>::funcT;
	private:
		funcT _defProc; // default return procedure, like DefWindowProc
		inventory<UINT> _msgInventory;

	protected:
		msgs() = default;

		LONG_PTR default_proc(params& p) {
			return this->_defProc(p);
		}

	public:
		void on_message(UINT msg, funcT func) {
			this->_msgInventory.add(msg, std::move(func));
		}

		void on_message(std::initializer_list<UINT> msgs, funcT func) {
			this->_msgInventory.add(msgs, std::move(func));
		}
	};

}//namespace base
}//namespace wl