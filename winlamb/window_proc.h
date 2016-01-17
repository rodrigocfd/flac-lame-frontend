/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <functional>
#include <vector>
#include "window.h"

/**
 * window_proc
 *  window
 */

namespace winlamb {

template<typename traitsT>
class window_proc : public window {
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
	window_proc() : _loopStarted(false)
	{
		on_message(WM_COMMAND, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			for (const auto& c : _cmds) {
				if (c.cmd == LOWORD(wp)) {
					return c.callback();
				}
			}
			return traitsT::default_proc(_hwnd, WM_COMMAND, wp, lp);
		});
		on_message(WM_NOTIFY, [this](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
			NMHDR& nmhdr = *reinterpret_cast<NMHDR*>(lp);
			for (const auto& n : _notifs) {
				if (n.idFrom == nmhdr.idFrom && n.code == nmhdr.code) {
					return n.callback(nmhdr);
				}
			}
			return traitsT::default_proc(_hwnd, WM_NOTIFY, wp, lp);
		});
	}

public:
	virtual ~window_proc() = default;

	virtual void on_message(UINT msg, msg_func_type callback)
	{
		if (!_loopStarted) {
			for (auto& m : _msgs) {
				if (m.msg == msg) {
					m.callback = std::move(callback); // replace existing
					return;
				}
			}
			this->_msgs.push_back({ msg, std::move(callback) }); // add new message handler
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
			this->_cmds.push_back({ cmd, std::move(callback) }); // add new WM_COMMAND handler
		}
	}

	virtual void on_notify(UINT idFrom, UINT code, notif_func_type callback)
	{
		if (!_loopStarted) {
			for (auto& n : _notifs) {
				if (n.idFrom == idFrom && n.code == code) {
					n.callback = std::move(callback); // replace existing
					return;
				}
			}
			this->_notifs.push_back({ idFrom, code, std::move(callback) }); // add new WM_NOTIFY handler
		}
	}

protected:
	static typename traitsT::ret_type CALLBACK _proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		window_proc *pSelf = reinterpret_cast<window_proc*>(
			traitsT::get_instance_pointer(hWnd, msg, lp) );
		if (pSelf) {
			if (!pSelf->_loopStarted) {
				pSelf->_loopStarted = true; // no more messages can be added
				pSelf->_hwnd = hWnd; // store HWND
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
	window::_hwnd;
};

}//namespace winlamb