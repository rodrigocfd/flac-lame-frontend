/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowGuiThread.h"

namespace wolf {

class WindowProc : public WindowGuiThread<DefWindowProc> {
protected:
	struct SetupProc {
		std::wstring className;
		HBRUSH       hBackground;
		HCURSOR      hCursor;
		SetupProc();
	};

public:
	virtual ~WindowProc() = 0;
protected:
	static ATOM _registerClass(HINSTANCE hInst, WNDCLASSEX& wc, SetupProc& setup);
private:
	static LRESULT CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	WindowMsgHandler::_processMsg;
};

}//namespace wolf