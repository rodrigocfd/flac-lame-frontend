/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <vector>
#include "base_wnd.h"

/**
 * base_wnd <-- base_wnd_loop
 */

namespace wet {

class base_wnd_loop : virtual public base_wnd {
private:
	std::vector<HWND> _modelessChildren;

protected:
	base_wnd_loop() = default;

	bool modeless_add(HWND hWndModeless) {
		// Allows a modeless HWND to have dialog messages processed.
		bool alreadyAdded = modeless_remove(hWndModeless);
		this->_modelessChildren.emplace_back(hWndModeless);
		return alreadyAdded;
	}

	bool modeless_remove(HWND hWndModeless) {
		// Call if you destroy a modeless HWND.
		for (size_t i = 0; i < this->_modelessChildren.size(); ++i) {
			if (this->_modelessChildren[i] == hWndModeless) {
				this->_modelessChildren.erase(this->_modelessChildren.begin() + i);
				return true;
			}
		}
		return false;
	}

	int _msg_loop(HACCEL hAccel = nullptr) const {
		MSG  msg = { 0 };
		BOOL ret = 0;
		while ((ret = GetMessageW(&msg, nullptr, 0, 0)) != 0) {
			if (ret == -1) return -1;
			if (this->_belongs_to_modeless(&msg) ||
				(hAccel && TranslateAcceleratorW(this->base_wnd::hwnd(), hAccel, &msg)) ||
				IsDialogMessageW(this->base_wnd::hwnd(), &msg) ) continue;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return static_cast<int>(msg.wParam); // this can be used as program return value
	}

private:
	bool _belongs_to_modeless(MSG* pmsg) const {
		for (const HWND hModl : this->_modelessChildren) {
			if (hModl && IsWindow(hModl) && IsDialogMessageW(hModl, pmsg)) {
				return true;
			}
		}
		return false;
	}
};

}//namespace wet