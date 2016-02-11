/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <functional>
#include <vector>
#include "handle.h"

/**
 * proc
 *  handle
 */

namespace winlamb {

template<typename traitsT>
class proc : public handle {
public:
	typedef std::function<typename traitsT::ret_type(WPARAM, LPARAM)> msg_func_type;
	typedef std::function<typename traitsT::ret_type()> cmd_func_type;
	typedef std::function<typename traitsT::ret_type(NMHDR&)> notif_func_type;

private:
	struct _msg_handler final {
		UINT msg;
		msg_func_type callback;
	};
	struct _cmd_handler final {
		WORD cmd;
		cmd_func_type callback;
	};
	struct _notif_handler final {
		UINT_PTR idFrom;
		UINT code;
		notif_func_type callback;
	};

	std::vector<_msg_handler> _msgs;
	std::vector<_cmd_handler> _cmds;
	std::vector<_notif_handler> _notifs;
	bool _loopStarted;

protected:
	proc() : _loopStarted(false)
	{
		on_message(WM_COMMAND, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			for (const auto& c : _cmds) {
				if (c.cmd == LOWORD(wp)) {
					return c.callback();
				}
			}
			return traitsT::default_proc(_hWnd, WM_COMMAND, wp, lp);
		});
		on_message(WM_NOTIFY, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			NMHDR& nmhdr = *reinterpret_cast<NMHDR*>(lp);
			for (const auto& n : _notifs) {
				if (n.idFrom == nmhdr.idFrom && n.code == nmhdr.code) {
					return n.callback(nmhdr);
				}
			}
			return traitsT::default_proc(_hWnd, WM_NOTIFY, wp, lp);
		});
	}

public:
	virtual ~proc() = default;

	virtual void on_message(UINT msg, msg_func_type callback)
	{
		if (!_loopStarted) {
			for (auto& m : _msgs) {
				if (m.msg == msg) {
					m.callback = std::move(callback); // replace existing
					return;
				}
			}
			_msgs.push_back({ msg, std::move(callback) }); // add new message handler
		}
	}

	virtual void on_command(WORD cmd, cmd_func_type callback)
	{
		if (!_loopStarted) {
			for (auto& c : _cmds) {
				if (c.cmd == cmd) {
					c.callback = std::move(callback); // replace existing
					return;
				}
			}
			_cmds.push_back({ cmd, std::move(callback) }); // add new WM_COMMAND handler
		}
	}

	virtual void on_notify(UINT_PTR idFrom, UINT code, notif_func_type callback)
	{
		if (!_loopStarted) {
			for (auto& n : _notifs) {
				if (n.idFrom == idFrom && n.code == code) {
					n.callback = std::move(callback); // replace existing
					return;
				}
			}
			_notifs.push_back({ idFrom, code, std::move(callback) }); // add new WM_NOTIFY handler
		}
	}

	void on_message(std::initializer_list<UINT> msgs, msg_func_type callback)
	{
		on_message(*msgs.begin(), std::move(callback)); // store first once
		size_t m0 = _msgs.size() - 1;

		for (size_t i = 1; i < msgs.size(); ++i) {
			if (*(msgs.begin() + i) != *msgs.begin()) {
				on_message(*(msgs.begin() + i), [this, m0](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
					return _msgs[m0].callback(wp, lp); // store light wrapper to first
				});
			}
		}
	}

	void on_command(std::initializer_list<WORD> cmds, cmd_func_type callback)
	{
		on_command(*cmds.begin(), std::move(callback)); // store first once
		size_t c0 = _cmds.size() - 1;

		for (size_t i = 1; i < cmds.size(); ++i) {
			if (*(cmds.begin() + i) != *cmds.begin()) {
				on_command(*(cmds.begin() + i), [this, c0]()->typename traitsT::ret_type {
					return _cmds[c0].callback(); // store light wrapper to first
				});
			}
		}
	}

	void on_notify(std::initializer_list<std::pair<UINT_PTR, UINT>> idFromAndCodes, notif_func_type callback)
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

protected:
	static typename traitsT::ret_type CALLBACK _process(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		proc *pSelf = reinterpret_cast<proc*>(
			traitsT::get_instance_pointer(hWnd, msg, lp) );
		if (pSelf) {
			if (!pSelf->_loopStarted) {
				pSelf->_loopStarted = true; // no more messages can be added
				pSelf->_hWnd = hWnd; // store HWND
			}
			for (const auto& m : pSelf->_msgs) {
				if (m.msg == msg) {
					return m.callback(wp, lp);
				}
			}
		}
		return traitsT::default_proc(hWnd, msg, wp, lp);
	}

private:
	handle::_hWnd;
};

}//namespace winlamb