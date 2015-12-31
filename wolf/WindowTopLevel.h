/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowParent.h"

namespace wolf {

class WindowTopLevel : public WindowParent {
protected:
	struct SetupTopLevel : public WindowParent::SetupParent {
		std::wstring title;
		bool   resize;
		bool   maximize;
		bool   minimize;
		bool   dropFiles;
		SIZE   size;
		HACCEL hAccel;
		SetupTopLevel();
	};

public:
	virtual ~WindowTopLevel() = 0;
protected:
	bool _loadIfTemplate(HINSTANCE hInst, SetupTopLevel& setup);
	int  _loop(const SetupTopLevel& setup);
	static DWORD _calcStyle(const SetupTopLevel& setup);
	static DWORD _calcStyleEx(const SetupTopLevel& setup);
	static bool  _compensateBorders(DWORD style, bool hasMenu, SetupTopLevel& setup);
private:
	WindowParent::SetupParent;
	WindowParent::_loadIfTemplate;
};

}//namespace wolf