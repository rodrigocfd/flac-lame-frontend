//
// HWND wrapper.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "Window.h"
#include <process.h>
#include <Shlobj.h>
#include <VsStyle.h>
#include <UxTheme.h>
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' " \
  "name='Microsoft.Windows.Common-Controls' " \
  "version='6.0.0.0' " \
  "processorArchitecture='*' " \
  "publicKeyToken='6595b64144ccf1df' " \
  "language='*'\"")

WindowPopup::~WindowPopup()
{
}

static HHOOK _hHookMsgBox = nullptr;
static HWND  _hWndParent = nullptr;
static LRESULT CALLBACK _msgBoxHookProc(int code, WPARAM wp, LPARAM lp)
{
	// http://www.codeguru.com/cpp/w-p/win32/messagebox/print.php/c4541
	if(code == HCBT_ACTIVATE)
	{
		HWND hMsgbox = (HWND)wp;
		RECT rcMsgbox, rcParent;

		if(hMsgbox && _hWndParent && GetWindowRect(hMsgbox, &rcMsgbox) && GetWindowRect(_hWndParent, &rcParent))
		{
			RECT  rcScreen = { 0 };
			POINT pos = { 0 };

			SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcScreen, 0); // size of desktop

			// Adjusted x,y coordinates to message box window.
			pos.x = rcParent.left + (rcParent.right - rcParent.left) / 2 - (rcMsgbox.right - rcMsgbox.left) / 2;
			pos.y =	rcParent.top + (rcParent.bottom - rcParent.top) / 2 - (rcMsgbox.bottom - rcMsgbox.top) / 2;

			// Screen out-of-bounds corrections.
			if(pos.x < 0)
				pos.x = 0;
			else if(pos.x + (rcMsgbox.right - rcMsgbox.left) > rcScreen.right)
				pos.x = rcScreen.right - (rcMsgbox.right - rcMsgbox.left);
			if(pos.y < 0)
				pos.y = 0;
			else if(pos.y + (rcMsgbox.bottom - rcMsgbox.top) > rcScreen.bottom)
				pos.y = rcScreen.bottom - (rcMsgbox.bottom - rcMsgbox.top);

			MoveWindow(hMsgbox, pos.x, pos.y,
				rcMsgbox.right - rcMsgbox.left, rcMsgbox.bottom - rcMsgbox.top,
				FALSE);
		}
		UnhookWindowsHookEx(_hHookMsgBox); // release hook
	}
	return CallNextHookEx(nullptr, code, wp, lp);
}

int WindowPopup::messageBox(const wchar_t *caption, const wchar_t *body, UINT uType)
{
	// The hook is set to center the message box window on parent.
	_hWndParent = this->hWnd();
	_hHookMsgBox = SetWindowsHookEx(WH_CBT, _msgBoxHookProc, nullptr, GetCurrentThreadId());
	return MessageBox(this->hWnd(), body, caption, uType);
}

static Array<wchar_t> _formatFileFilter(const wchar_t *filterWithPipes)
{
	// Input filter follows same C# syntax:
	// L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*"

	Array<wchar_t> ret(lstrlen(filterWithPipes) + 2); // two terminating nulls
	ret.last() = L'\0';
	for(int i = 0; i < ret.size() - 1; ++i)
		ret[i] = (filterWithPipes[i] != L'|') ? filterWithPipes[i] : L'\0';
	return ret;
}

bool WindowPopup::getFileOpen(const wchar_t *filter, String& buf)
{
	OPENFILENAME ofn = { 0 };
	wchar_t tmpBuf[MAX_PATH] = { 0 };
	Array<wchar_t> zfilter = _formatFileFilter(filter);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFilter = filter;
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;// | OFN_HIDEREADONLY;

	bool ret = GetOpenFileName(&ofn) != 0;
	if(ret) buf = tmpBuf;
	return ret;
}

bool WindowPopup::getFileOpen(const wchar_t *filter, Array<String>& arrBuf)
{
	OPENFILENAME   ofn = { 0 };
	Array<wchar_t> multiBuf(65536); // http://www.askjf.com/?q=2179s http://www.askjf.com/?q=2181s
	Array<wchar_t> zfilter = _formatFileFilter(filter);

	multiBuf[0] = L'\0';

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFile   = &multiBuf[0];
	ofn.nMaxFile    = multiBuf.size() + 1; // including terminating null
	ofn.Flags       = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING;

	if(GetOpenFileName(&ofn)) {
		Array<String> strs = String::ExplodeMulti(&multiBuf[0]);
		if(!strs.size()) {
			dbg(L"No multiple strings on GetOpenFileName().\n");
			return false;
		}

		if(strs.size() == 1) { // if user selected only 1 file, the string is the full path, and that's all
			arrBuf.append(strs[0]);
		} else { // user selected 2 or more files
			String *basePath = &strs[0]; // 1st string is the base path; others are the filenames
			arrBuf.resize(strs.size() - 1); // alloc return buffer

			for(int i = 0; i < strs.size() - 1; ++i) {
				arrBuf[i].reserve(basePath->len() + strs[i + 1].len() + 1); // room for backslash
				arrBuf[i] = (*basePath);
				arrBuf[i].append(L"\\").append(strs[i + 1]); // concat folder + file
			}
			arrBuf.sort([](const String& a, const String& b)->int {
				return String::CompareCI(a.str(), b.str());
			});
		}
		return true;
	}

	DWORD errNo = CommDlgExtendedError();
	if(errNo == FNERR_BUFFERTOOSMALL)
		dbg(L"GetOpenFileName: buffer too small (%d bytes).", multiBuf.size() + 1);
	else
		dbg(L"GetOpenFileName: failed with error %d.", errNo);
	return false;
}

bool WindowPopup::getFileSave(const wchar_t *filter, String& buf, const wchar_t *defFile)
{
	OPENFILENAME ofn = { 0 };
	wchar_t tmpBuf[MAX_PATH] = { 0 };
	Array<wchar_t> zfilter = _formatFileFilter(filter);

	if(defFile)
		lstrcpy(tmpBuf, defFile);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"txt"; // apparently could be anything, will just force append of combo selected extension

	bool ret = GetSaveFileName(&ofn) != 0;
	if(ret) buf = tmpBuf;
	return ret;
}

bool WindowPopup::getFolderChoose(String& buf)
{
	CoInitialize(nullptr);

	//LPITEMIDLIST pidlRoot = 0;
	//if(defFolder)
		//SHParseDisplayName(defFolder, nullptr, &pidlRoot, 0, nullptr);

	BROWSEINFO bi = { 0 };
	bi.hwndOwner = this->hWnd();
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
	if(!pidl)
		return false; // user cancelled

	wchar_t tmpbuf[MAX_PATH] = { 0 };
	if(!SHGetPathFromIDList(pidl, tmpbuf))
		return false; // some weird error

	CoUninitialize();
	buf = tmpbuf;
	return true;
}

void WindowPopup::setXButton(bool enable)
{
	// Enable/disable the X button to close the window; has no effect on Alt+F4.
	HMENU hMenu = GetSystemMenu(this->hWnd(), FALSE);
	if(hMenu) {
		UINT dwExtra = enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | dwExtra);
	}
}

Array<String> WindowPopup::getDroppedFiles(HDROP hDrop)
{
	Array<String> ret(DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0)); // alloc return array

	for(int i = 0; i < ret.size(); ++i) {
		ret[i].reserve(DragQueryFile(hDrop, i, 0, 0)); // alloc path string
		DragQueryFile(hDrop, i, ret[i].ptrAt(0), ret[i].reserved() + 1);
	}

	DragFinish(hDrop);
	return ret;
}

static LRESULT CALLBACK _wheelHoverProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch(msg)
	{
	case WM_MOUSEWHEEL:
		if(!(LOWORD(wp) & 0x0800)) { // bitflag not set, this is the first and unprocessed WM_MOUSEWHEEL passage
			HWND hParent = (HWND)refData;
			POINT pt = { LOWORD(lp), HIWORD(lp) };
			ScreenToClient(hParent, &pt); // to client coordinates relative to parent
			SendMessage(ChildWindowFromPoint(hParent, pt), // window below cursor
				WM_MOUSEWHEEL,
				MAKEWPARAM(LOWORD(wp) | 0x0800, HIWORD(wp)), // set 0x0800 bitflag and kick to window below cursor
				lp);
			return 0; // halt processing
		} else { // bitflag is set, WM_MOUSEWHEEL has been kicked here and can be safely processed
			wp &= ~0x0800; // unset bitflag
			break; // finally dispatch to default processing
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hChild, _wheelHoverProc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hChild, msg, wp, lp);
}

void WindowPopup::_setWheelHoverBehavior()
{
	// http://stackoverflow.com/questions/18367641/use-createthread-with-a-lambda
	EnumChildWindows(this->hWnd(), [](HWND hChild, LPARAM lp)->BOOL {
		static int uniqueSubId = 1;
		SetWindowSubclass(hChild, _wheelHoverProc, uniqueSubId++,
			(DWORD_PTR)GetParent(hChild)); // yes, subclass every control
		return TRUE;
	}, 0);
}

struct _CbPack { function<void()> cb; };
void WindowPopup::_handleSendOrPostFunction(LPARAM lp)
{
	// This method is called by FramePopup and DialogPopup wndprocs on their ordinary processing.
	_CbPack *cbPack = (_CbPack*)lp;
	cbPack->cb(); // invoke user callback
	delete cbPack; // allocated by _sendOrPostFunction()
}

void WindowPopup::_sendOrPostFunction(function<void()> callback, bool isSend)
{
	// This method is analog to Send/PostMessage, but intended to be called within a separated thread,
	// so a callback function can, tunelled by wndproc, run in the same thread of the window, thus
	// allowing GUI updates. This avoids the user to deal with a custom WM_ message.
	_CbPack *cbPack = new _CbPack{ MOVE(callback) }; // will be deleted by _handleSendOrPostFunction()
	isSend ? this->sendMessage(WM_APP-1, 0, (LPARAM)cbPack) :
		this->postMessage(WM_APP-1, 0, (LPARAM)cbPack);
}


WindowCtrl::~WindowCtrl()
{
}

bool WindowCtrl::_drawBorders(WPARAM wp, LPARAM lp)
{
	// Intended to be called within WM_NCPAINT processing.
	if((GetWindowLongPtr(this->hWnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive())
	{
		DefWindowProc(this->hWnd(), WM_NCPAINT, wp, lp); // this will make system draw the scrollbar for us; days of struggling until this enlightenment

		RECT rc = this->getWindowRect(); // window outmost coordinates, including margins
		this->screenToClient((POINT*)&rc);
		this->screenToClient((POINT*)&rc.right);
		rc.left += 2; rc.top += 2; rc.right += 2; rc.bottom += 2; // because it comes up anchored at -2,-2

		RECT rc2 = { 0 }; // clipping region; will draw only within this rectangle
		HDC hdc = GetWindowDC(this->hWnd());
		HTHEME hTheme = OpenThemeData(this->hWnd(), L"LISTVIEW"); // borrow style from listview

		SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
		SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
		SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
		SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border

		CloseThemeData(hTheme);
		ReleaseDC(this->hWnd(), hdc);
		return true; // caller should return 0
	}
	return false; // caller should call default window processing
}