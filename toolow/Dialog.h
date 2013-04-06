//
// Realm of the dialog windows, those who take a DLGPROC as a procedure and
// are created through a dialog resource.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Window.h"

//__________________________________________________________________________________________________
// Base class to any dialog window.
//
class Dialog : virtual public Window {
public:
	virtual ~Dialog() = 0;
protected:
	virtual INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
	static INT_PTR CALLBACK _DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
};

//__________________________________________________________________________________________________
// Intermediary class to any regular window which acts as a container of child windows.
//
class DialogPopup : public Dialog, public WindowPopup {
public:
	virtual ~DialogPopup() = 0;
protected:
	virtual INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
	BOOL endDialog(INT_PTR nResult);
};

//__________________________________________________________________________________________________
// Base class to a dialog window to run as the default program window.
//
class DialogApp : public DialogPopup {
public:
	virtual ~DialogApp() = 0;
	virtual int run(HINSTANCE hInst, int cmdShow, int dialogId, int iconId=0, int accelTableId=0);
protected:
	virtual INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
private:
	Dialog::_DialogProc;
};

//__________________________________________________________________________________________________
// Base class to a dialog window to be displayed as modal.
//
class DialogModal : public DialogPopup {
public:
	virtual ~DialogModal() = 0;
	virtual int show(Window *parent, int dialogId, int accelTableId=0);
protected:
	virtual INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
private:
	Dialog::_DialogProc;
};

//__________________________________________________________________________________________________
// Base class to a dialog window to be used as a child of another window.
//
class DialogCtrl : public Dialog, public WindowCtrl {
public:
	virtual ~DialogCtrl() = 0;
	virtual void create(int id, Window *parent, int x, int y, int cx, int cy);
protected:
	virtual INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
private:
	WindowCtrl::_drawBorders;
};