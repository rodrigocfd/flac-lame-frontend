/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowProc.h"
#include "WindowDialogTemplate.h"

namespace wolf {

class WindowParent : public WindowProc {
protected:
	struct SetupParent : public WindowProc::SetupProc {
		int dialogId;
		SetupParent();
	};

private:
	HWND _hWndCurFocus;
protected:
	WindowDialogTemplate _dialogTemplate;
public:
	virtual ~WindowParent() = 0;
	WindowParent();
protected:
	Window getChild(int controlId);
	Window createChild(const wchar_t *className, const wchar_t *title, int ctrlId,
		POINT pos, SIZE sz, DWORD style, DWORD exStyle);
	bool _loadIfTemplate(HINSTANCE hInst, SetupParent& setup);
private:
	WindowProc::SetupProc;
};

}//namespace wolf