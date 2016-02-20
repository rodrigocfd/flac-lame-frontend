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
 * msg_keydown
 *  wnd_proc
 *   wnd
 */

namespace winlamb {

template<typename traitsT>
class msg_keydown : virtual public wnd_proc<traitsT> {
public:
	typedef std::function<typename traitsT::ret_type()> keydown_func_type;

private:
	struct _keydown_unit final {
		WORD virtKeyCode;
		keydown_func_type callback;
	};
	std::vector<_keydown_unit> _keydowns;

protected:
	msg_keydown()
	{
		on_message(WM_KEYDOWN, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			for (const auto& kd : _keydowns) {
				if (kd.virtKeyCode == wp) {
					return kd.callback();
				}
			}
			return traitsT::default_proc(hwnd(), WM_KEYDOWN, wp, lp);
		});
	}

public:
	virtual ~msg_keydown() = default;

	void on_keydown(WORD virtKeyCode, keydown_func_type callback)
	{
		for (auto& kd : _keydowns) {
			if (kd.virtKeyCode == virtKeyCode) {
				kd.callback = std::move(callback); // replace existing
				return;
			}
		}
		_keydowns.push_back({ virtKeyCode, std::move(callback) }); // add new WM_KEYDOWN handler
	}

	void on_keydown(std::initializer_list<WORD> virtKeyCodes, keydown_func_type callback)
	{
		on_keydown(*virtKeyCodes.begin(), std::move(callback)); // store 1st message once
		size_t m0 = _keydowns.size() - 1;

		for (size_t i = 1; i < virtKeyCodes.size(); ++i) {
			if (*(virtKeyCodes.begin() + i) != *virtKeyCodes.begin()) { // avoid overwriting
				on_keydown(*(virtKeyCodes.begin() + i), [this, m0]()->typename traitsT::ret_type {
					return _keydowns[m0].callback(); // store light wrapper to 1st message
				});
			}
		}
	}
};

typedef msg_keydown<traits_window> msg_keydown_window;
typedef msg_keydown<traits_dialog> msg_keydown_dialog;

}//namespace winlamb