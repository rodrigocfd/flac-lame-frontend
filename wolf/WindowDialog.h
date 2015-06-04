/*!
 * @file
 * @brief Dialog windows, created from a dialog resource and using DLGPROC.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowEvent.h"

/*
          +-- WindowTopLevel <---------------+                     +-- [DialogApp]
          |                                  +-- DialogTopLevel <--+
          |                          <-------+                     +-- [DialogModal]
          +-- WindowEvent <-- Dialog
Window <--+                          <-------+
          |                                  +-- [DialogChild]
          +-- WindowChild <------------------+
*/

namespace wolf {

/// Base class to any dialog window.
class Dialog : public WindowEventDialog {
protected:
	int _dialogId;
public:
	Dialog(int dialogId) : _dialogId(dialogId) { }
	virtual ~Dialog() = 0;
protected:
	static INT_PTR CALLBACK _DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
	virtual void _onInitDialog() = 0;
	virtual void _internalEvents() override;
private:
	WindowEventDialog::_processMessage;
};


/// Intermediary class to any dialog window which acts as a top-level window.
class DialogTopLevel : public Dialog, public WindowTopLevel {
public:
	DialogTopLevel(int dialogId) : Dialog(dialogId) { }
	virtual ~DialogTopLevel() = 0;
protected:
	BOOL endDialog(INT_PTR nResult);
protected:
	virtual void _onInitDialog() override;
	virtual void _internalEvents() override;
private:
	WindowTopLevel::_WM_ORIGTHREAD;
};


/// Inherit from this class to create a dialog window to run as the default top-level program window.
class DialogApp : public DialogTopLevel {
private:
	int _iconId;
public:
	DialogApp(int dialogId, int iconId=0) : DialogTopLevel(dialogId), _iconId(iconId) { }
	virtual ~DialogApp() = 0;
	int run(HINSTANCE hInst, int cmdShow);
private:
	void _onInitDialog() override;
	void _internalEvents() override;
	Dialog::_dialogId;
	Dialog::_DialogProc;
};


/// Inherit from this class to create a dialog window to be displayed as modal top-level window.
class DialogModal : public DialogTopLevel {
public:
	DialogModal(int dialogId) : DialogTopLevel(dialogId) { }
	virtual ~DialogModal() = 0;
	int show(Window *owner);
private:
	void _onInitDialog() override;
	void _internalEvents() override;
	Dialog::_dialogId;
	Dialog::_DialogProc;
};


/// Inherit from this class class to create a dialog window to be used as a child of another window.
class DialogChild : public Dialog, public WindowChild {
private:
	DWORD _border;
public:
	enum class Border { YES=WS_EX_CLIENTEDGE, NO=0 };
public:
	DialogChild(int dialogId, Border b)
		: Dialog(dialogId), _border(static_cast<DWORD>(b)) { }
	virtual ~DialogChild() = 0;
	void create(Window *parent, POINT pos, SIZE sz);
private:
	void _onInitDialog() override;
	void _internalEvents() override;
	WindowChild::_drawThemeBorders;
	Dialog::_dialogId;
	Dialog::_DialogProc;
};

}//namespace wolf
