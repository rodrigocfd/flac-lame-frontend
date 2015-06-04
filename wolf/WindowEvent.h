/*!
 * @file
 * @brief Mix-in class to any window which handles event messages.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <unordered_map>
#include "Window.h"

namespace wolf {

/// Base template to handle the window procedure, exposing lambdas to attach events.
template<typename RET>
class WindowEvent : virtual public Window {
private:
	std::unordered_map<UINT, std::function<RET(WPARAM, LPARAM)>> _msgs;
	std::unordered_map<WORD, std::function<RET()>> _cmdMsgs;
	std::unordered_map<UINT_PTR, std::unordered_map<UINT, std::function<RET(NMHDR*)>>> _notifMsgs;
public:
	virtual ~WindowEvent() = 0;
protected:
	virtual void events() = 0;
	bool onMessage(UINT msg, std::function<RET(WPARAM, LPARAM)> callback);
	bool onCommand(WORD cmd, std::function<RET()> callback);
	bool onNotify(UINT_PTR idFrom, UINT code, std::function<RET(NMHDR*)> callback);
protected:
	RET _processMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual void _internalEvents();
private:
	virtual RET _defProc(UINT msg, WPARAM wp, LPARAM lp) = 0;
};


/// Event handler for WNDPROC of regular (non-dialog) windows.
class WindowEventFrame : public WindowEvent<LRESULT> {
public:
	virtual ~WindowEventFrame() = 0;
private:
	LRESULT _defProc(UINT msg, WPARAM wp, LPARAM lp) override;
};


/// Event handler for DLGPROC of dialog windows.
class WindowEventDialog : public WindowEvent<INT_PTR> {
public:
	virtual ~WindowEventDialog() = 0;
private:
	INT_PTR _defProc(UINT msg, WPARAM wp, LPARAM lp) override;
};


// ----------------------------- Template implementation -------------------------------------------

template<typename RET>
WindowEvent<RET>::~WindowEvent() {
}

template<typename RET>
bool WindowEvent<RET>::onMessage(UINT msg, std::function<RET(WPARAM, LPARAM)> callback) {
	// Inserts a message handler; if already exists, does nothing and returns false;
	return _msgs.emplace(msg, std::move(callback)).second;
}

template<typename RET>
bool WindowEvent<RET>::onCommand(WORD cmd, std::function<RET()> callback) {
	return _cmdMsgs.emplace(cmd, std::move(callback)).second;
}

template<typename RET>
bool WindowEvent<RET>::onNotify(UINT_PTR idFrom, UINT code, std::function<RET(NMHDR*)> callback) {
	return _notifMsgs[idFrom].emplace(code, std::move(callback)).second;
}

template<typename RET>
RET WindowEvent<RET>::_processMessage(UINT msg, WPARAM wp, LPARAM lp) {
	auto itmsg = _msgs.find(msg);
	if (itmsg != _msgs.end()) {
		return itmsg->second(wp, lp); // invoke user callback
	}
	return this->_defProc(msg, wp, lp);
}

template<typename RET>
void WindowEvent<RET>::_internalEvents() {
	// If user adds handler to one of these messages, it his responsability to deal with them.
	this->onMessage(WM_COMMAND, [&](WPARAM wp, LPARAM lp)->RET {
		auto itcmd = _cmdMsgs.find(LOWORD(wp));
		if (itcmd != _cmdMsgs.end()) {
			return itcmd->second(); // invoke user callback
		}
		return this->_defProc(WM_COMMAND, wp, lp);
	});
	this->onMessage(WM_NOTIFY, [&](WPARAM wp, LPARAM lp)->RET {
		NMHDR *nmhdr = reinterpret_cast<NMHDR*>(lp);
		auto itid = _notifMsgs.find(nmhdr->idFrom);
		if (itid != _notifMsgs.end()) {
			auto itcode = itid->second.find(nmhdr->code);
			if (itcode != itid->second.end()) {
				return itcode->second(nmhdr); // invoke user callback
			}
		}
		return this->_defProc(WM_NOTIFY, wp, lp);
	});
}


inline WindowEventFrame::~WindowEventFrame() {
}

inline LRESULT WindowEventFrame::_defProc(UINT msg, WPARAM wp, LPARAM lp) {
	return DefWindowProc(this->hWnd(), msg, wp, lp);
}


inline WindowEventDialog::~WindowEventDialog() {
}

inline INT_PTR WindowEventDialog::_defProc(UINT msg, WPARAM wp, LPARAM lp) {
	return FALSE;
}


}//namespace wolf