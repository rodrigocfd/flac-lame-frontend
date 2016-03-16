/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "wnd_proc.h"
#include "traits_dialog.h"

/**
 * dialog
 *  wnd_proc<traits_dialog>
 *   wnd
 */

namespace winlamb {

struct setup_dialog {
	int dialogId;
	setup_dialog() : dialogId(0) { }
};


template<typename setupT = setup_dialog>
class dialog : virtual public wnd_proc<traits_dialog> {
public:
	setupT setup;
	virtual ~dialog() = default;

protected:
	dialog() = default;
};

}//namespace winlamb