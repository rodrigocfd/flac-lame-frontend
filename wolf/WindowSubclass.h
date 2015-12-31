/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowGuiThread.h"

namespace wolf {

class WindowSubclass : public WindowGuiThread<DefSubclassProc> {
public:
	virtual ~WindowSubclass() = 0;
	WindowSubclass();
	WindowSubclass(HWND hwnd);
	WindowSubclass(Window&& w);
	WindowSubclass(WindowSubclass&& w);
	WindowSubclass& operator=(HWND hwnd);
	WindowSubclass& operator=(Window&& w);
	WindowSubclass& operator=(WindowSubclass&& w);
	WindowSubclass& create(HWND hParent, const wchar_t *className, const wchar_t *title,
		int id, POINT pos, SIZE size, DWORD style, DWORD exStyle);
	WindowSubclass& create(const Window *parent, const wchar_t *className, const wchar_t *title,
		int id, POINT pos, SIZE size, DWORD style, DWORD exStyle);
private:
	static const UINT SUBCLASSID;
	void _clear();
	void _setNew();
	static LRESULT CALLBACK _subclassProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
		UINT_PTR idSubclass, DWORD_PTR refData);
	WindowMsgHandler::_processMsg;
	WindowMsgHandler::_errorShout;
};

}//namespace wolf