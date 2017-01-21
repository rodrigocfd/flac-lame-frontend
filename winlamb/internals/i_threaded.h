/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "threaded.h"

namespace wl {
namespace internals {

class i_threaded {
private:
	const threaded& _threaded;
protected:
	i_threaded(const threaded& t) : _threaded(t) { }

public:
	void ui_thread(internals::threaded::funcT func) const {
		this->_threaded.ui_thread(std::move(func));
	}
};

}//namespace internals
}//namespace wl