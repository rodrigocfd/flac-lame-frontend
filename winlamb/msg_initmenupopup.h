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
 * msg_initmenupopup
 *  wnd_proc
 *   wnd
 */

namespace winlamb {

template<typename traitsT>
class msg_initmenupopup : virtual public wnd_proc<traitsT> {
public:
	typedef std::function<typename traitsT::ret_type(HMENU)> initmenupopup_func_type;

private:
	struct _imp_unit final {
		WORD commandIdOfFirstItem;
		initmenupopup_func_type callback;
	};
	std::vector<_imp_unit> _imps;

protected:
	msg_initmenupopup()
	{
		on_message(WM_INITMENUPOPUP, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			HMENU hMenu = reinterpret_cast<HMENU>(wp);
			WORD cmdIdOfFirstItem = GetMenuItemID(hMenu, 0);
			for (auto& imp : _imps) {
				if (imp.commandIdOfFirstItem == cmdIdOfFirstItem) {
					return imp.callback(hMenu);
				}
			}
			return traitsT::default_proc(hwnd(), WM_INITMENUPOPUP, wp, lp);
		});
	}

public:
	virtual ~msg_initmenupopup() = default;

	void on_initmenupopup(WORD commandIdOfFirstItem, initmenupopup_func_type callback)
	{
		for (auto& imp : _imps) {
			if (imp.commandIdOfFirstItem == commandIdOfFirstItem) {
				imp.callback = std::move(callback); // replace existing
				return;
			}
		}
		_imps.push_back({ commandIdOfFirstItem, std::move(callback) }); // add new WM_INITMENUPOPUP handler
	}

	void on_initmenupopup(std::initializer_list<WORD> commandIdOfFirstItems, initmenupopup_func_type callback)
	{
		on_initmenupopup(*commandIdOfFirstItems.begin(), std::move(callback)); // store 1st message once
		size_t m0 = _imps.size() - 1;

		for (size_t i = 1; i < commandIdOfFirstItems.size(); ++i) {
			if (*(commandIdOfFirstItems.begin() + i) != *commandIdOfFirstItems.begin()) { // avoid overwriting
				on_initmenupopup(*(commandIdOfFirstItems.begin() + i), [this, m0]()->typename traitsT::ret_type {
					return _imps[m0].callback(); // store light wrapper to 1st message
				});
			}
		}
	}
};

typedef msg_initmenupopup<traits_window> msg_initmenupopup_window;
typedef msg_initmenupopup<traits_dialog> msg_initmenupopup_dialog;

}//namespace winlamb