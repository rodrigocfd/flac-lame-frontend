/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <vector>
#include "wnd.h"

/**
 * wnd <-- wnd_loop
 */

namespace winlamb {

class wnd_loop : virtual public wnd {
private:
	std::vector<HWND> _modelessChildren;

public:
	virtual ~wnd_loop() = default;

protected:
	bool modeless_add(HWND hWndModeless)
	{
		// Allows a modeless HWND to have dialog messages processed.
		bool alreadyAdded = modeless_remove(hWndModeless);
		this->_modelessChildren.emplace_back(hWndModeless);
		return alreadyAdded;
	}

	bool modeless_remove(HWND hWndModeless)
	{
		// Call if you destroy a modeless HWND.
		for (size_t i = 0; i < this->_modelessChildren.size(); ++i) {
			if (this->_modelessChildren[i] == hWndModeless) {
				this->_modelessChildren.erase(this->_modelessChildren.begin() + i);
				return true;
			}
		}
		return false;
	}

	int _msg_loop(HACCEL hAccel = nullptr)
	{
		MSG  msg = { 0 };
		BOOL ret = 0;
		while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
			if (ret == -1) return -1;
			if (this->_belongs_to_modeless(&msg) ||
				(hAccel && TranslateAccelerator(this->wnd::hwnd(), hAccel, &msg)) ||
				IsDialogMessage(this->wnd::hwnd(), &msg) ) continue;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return static_cast<int>(msg.wParam); // this can be used as program return value
	}

private:
	bool _belongs_to_modeless(MSG* pmsg) const
	{
		for (const HWND hModl : this->_modelessChildren) {
			if (hModl && IsWindow(hModl) && IsDialogMessage(hModl, pmsg)) {
				return true;
			}
		}
		return false;
	}
};

}//namespace winlamb