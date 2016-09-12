/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "wnd_msgs.h"
#include "wnd_thread.h"
#include "traits_dialog.h"

/**
 *                     +--- wnd_msgs <----+
 * wnd <-- wnd_proc <--+                  +-- dialog
 *                     +-- wnd_thread <---+
 */

namespace winlamb {

struct setup_dialog {
	int dialogId;
	setup_dialog() : dialogId(0) { }
};


template<typename setupT = setup_dialog>
class dialog :
	public wnd_msgs<traits_dialog>,
	public wnd_thread<traits_dialog>
{
public:
	virtual ~dialog() = default;

protected:
	setupT setup;
	dialog() = default;
};

}//namespace winlamb