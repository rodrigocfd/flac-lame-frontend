/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowParent.h"

namespace wolf {

class WindowControl : public WindowProc {
private:
	struct SetupControl final : public WindowProc::SetupProc {
		bool border;
		bool scrollVert;
		bool scrollHorz;
		bool tabStop;
		SetupControl();
	};

public:
	SetupControl setup;
	virtual ~WindowControl() = 0;
	WindowControl();
	void create(HWND hParent, int ctrlId, POINT pos, SIZE sz);
	void create(const WindowParent *parent, int ctrlId, POINT pos, SIZE sz);
private:
	ATOM _registerClass(HINSTANCE hInst);
	WindowMsgHandler::_errorShout;
	WindowProc::SetupProc;
	WindowProc::_registerClass;
};

}//namespace wolf