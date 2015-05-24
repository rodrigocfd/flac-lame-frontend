/*!
 * Dialog windows, created from a dialog resource and using DLGPROC.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#pragma once
#include "Window.h"

/*
          +-- WindowPopup <--+                  +-- [DialogApp]
          |                  +-- DialogPopup <--+
          |          <-------+                  +-- [DialogModal]
          +-- Dialog
Window <--+          <-------+
          |                  +-- [DialogCtrl]
          +-- WindowCtrl <---+
*/

namespace c4w {

// Base class to any dialog window.
class Dialog : virtual public Window {
protected:
	int _dialogId;
public:
	Dialog(int dialogId) : _dialogId(dialogId) { }
	virtual ~Dialog() = 0;
protected:
	virtual INT_PTR dlgProc(UINT msg, WPARAM wp, LPARAM lp);
	static INT_PTR CALLBACK _DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
};


// Intermediary class to any regular window which acts as a container of child windows.
class DialogPopup : public Dialog, public WindowPopup {
protected:
	int _menuId;
	int _accelTableId;
public:
	DialogPopup(int dialogId, int menuId, int accelTableId)
		: Dialog(dialogId), _menuId(menuId), _accelTableId(accelTableId) { }
	virtual ~DialogPopup() = 0;
protected:
	virtual INT_PTR dlgProc(UINT msg, WPARAM wp, LPARAM lp) override;
	BOOL endDialog(INT_PTR nResult);
private:
	WindowPopup::_setWheelHoverBehavior;
	WindowPopup::_handleSendOrPostFunction;
};


// Base class to a dialog window to run as the default program window.
class DialogApp : public DialogPopup {
private:
	int _iconId;
public:
	DialogApp(int dialogId, int iconId, int menuId=0, int accelTableId=0)
		: DialogPopup(dialogId, menuId, accelTableId), _iconId(iconId) { }
	virtual ~DialogApp() = 0;
	int run(HINSTANCE hInst, int cmdShow);
protected:
	virtual INT_PTR dlgProc(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	Dialog::_dialogId;
	Dialog::_DialogProc;
	DialogPopup::_menuId;
	DialogPopup::_accelTableId;
};


// Base class to a dialog window to be displayed as modal.
class DialogModal : public DialogPopup {
public:
	DialogModal(int dialogId, int menuId=0, int accelTableId=0)
		: DialogPopup(dialogId, menuId, accelTableId) { }
	virtual ~DialogModal() = 0;
	int show(Window *parent);
protected:
	virtual INT_PTR dlgProc(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	Dialog::_dialogId;
	Dialog::_DialogProc;
	DialogPopup::_menuId;
	DialogPopup::_accelTableId;
};


// Base class to a dialog window to be used as a child of another window.
class DialogCtrl : public Dialog, public WindowCtrl {
private:
	DWORD _border;
public:
	enum class Border { YES=WS_EX_CLIENTEDGE, NO=0 };
public:
	DialogCtrl(int dialogId, Border b)
		: Dialog(dialogId), _border(static_cast<DWORD>(b)) { }
	virtual ~DialogCtrl() = 0;
	void create(Window *parent, POINT pos, SIZE sz);
protected:
	virtual INT_PTR dlgProc(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	WindowCtrl::_drawBorders;
	Dialog::_dialogId;
	Dialog::_DialogProc;
};

}//namespace c4w