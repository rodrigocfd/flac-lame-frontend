/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowTopLevel.h"

namespace wolf {

class WindowModal : public WindowTopLevel {
private:
	struct SetupModal final : public WindowTopLevel::SetupTopLevel {
	};

public:
	SetupModal setup;
	virtual ~WindowModal() = 0;
	WindowModal();
	void show(HWND hOwner);
	void show(const WindowParent *owner);
private:
	ATOM _registerClass(HINSTANCE hInst);
	WindowMsgHandler::_errorShout;
	WindowProc::_registerClass;
	WindowParent::_dialogTemplate;
	WindowTopLevel::SetupTopLevel;
	WindowTopLevel::_loadIfTemplate;
	WindowTopLevel::_loop;
	WindowTopLevel::_calcStyle;
	WindowTopLevel::_calcStyleEx;
	WindowTopLevel::_compensateBorders;
};

}//namespace wolf