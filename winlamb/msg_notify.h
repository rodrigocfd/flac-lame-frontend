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
 * msg_notify
 *  wnd_proc
 *   wnd
 */

namespace winlamb {

template<typename traitsT>
class msg_notify : virtual public wnd_proc<traitsT> {
public:
	typedef std::function<typename traitsT::ret_type(NMHDR&)> notify_func_type;

private:
	struct _notify_unit final {
		UINT_PTR idFrom;
		UINT code;
		notify_func_type callback;
	};
	std::vector<_notify_unit> _notifs;

protected:
	msg_notify()
	{
		on_message(WM_NOTIFY, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			NMHDR& nmhdr = *reinterpret_cast<NMHDR*>(lp);
			for (const auto& n : _notifs) {
				if (n.idFrom == nmhdr.idFrom && n.code == nmhdr.code) {
					return n.callback(nmhdr);
				}
			}
			return traitsT::default_proc(hwnd(), WM_COMMAND, wp, lp);
		});
	}

public:
	virtual ~msg_notify() = default;

	void on_notify(UINT_PTR idFrom, UINT code, notify_func_type callback)
	{
		for (auto& n : _notifs) {
			if (n.idFrom == idFrom && n.code == code) {
				n.callback = std::move(callback); // replace existing
				return;
			}
		}
		_notifs.push_back({ idFrom, code, std::move(callback) }); // add new WM_NOTIFY handler
	}

	void on_notify(std::initializer_list<std::pair<UINT_PTR, UINT>> idFromAndCodes, notify_func_type callback)
	{
		UINT_PTR idFrom0 = idFromAndCodes.begin()->first;
		UINT code0 = idFromAndCodes.begin()->second;
		on_notify(idFrom0, code0, std::move(callback)); // store first once
		size_t n0 = _notifs.size() - 1;

		for (size_t i = 1; i < idFromAndCodes.size(); ++i) {
			UINT_PTR idFrom = (idFromAndCodes.begin() + i)->first;
			UINT code = (idFromAndCodes.begin() + i)->second;
			if (idFrom != idFrom0 && code != code0) {
				on_notify(idFrom, code, [this, n0](NMHDR& nmhdr)->typename traitsT::ret_type {
					return _notifs[n0].callback(nmhdr); // store light wrapper to first
				});
			}
		}
	}
};

typedef msg_notify<traits_window> msg_notify_window;
typedef msg_notify<traits_dialog> msg_notify_dialog;

}//namespace winlamb