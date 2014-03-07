
#include "Window.h"
#include <Shlobj.h>

WindowPopup::~WindowPopup()
{
}

static HHOOK _hHookMsgBox = NULL;
static HWND  _hWndParent = NULL;
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
	return CallNextHookEx(0, code, wp, lp);
}

int WindowPopup::messageBox(const wchar_t *caption, const wchar_t *body, UINT uType)
{
	// The hook is set to center the message box window on parent.
	_hWndParent = this->hWnd();
	_hHookMsgBox = SetWindowsHookEx(WH_CBT, _msgBoxHookProc, 0, GetCurrentThreadId());
	return MessageBox(this->hWnd(), body, caption, uType);
}

bool WindowPopup::getFileOpen(const wchar_t *formattedFilter, String *pBuf)
{
	OPENFILENAME ofn = { 0 };
	wchar_t tmpBuf[MAX_PATH] = { 0 };

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = formattedFilter;
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;// | OFN_HIDEREADONLY;

	bool ret = GetOpenFileName(&ofn) != 0;
	if(ret) (*pBuf) = tmpBuf;
	return ret;
}

bool WindowPopup::getFileOpen(const wchar_t *formattedFilter, Array<String> *pArrBuf)
{
	OPENFILENAME   ofn = { 0 };
	Array<wchar_t> multiBuf(65536); // http://www.askjf.com/?q=2179s http://www.askjf.com/?q=2181s
	
	multiBuf[0] = L'\0';

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = formattedFilter;
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
			pArrBuf->realloc(1); // alloc return buffer
			(*pArrBuf)[0] = strs[0];
		} else { // user selected 2 or more files
			String *basePath = &strs[0]; // 1st string is the base path; others are the filenames
			pArrBuf->realloc(strs.size() - 1); // alloc return buffer

			for(int i = 0; i < strs.size() - 1; ++i) {
				(*pArrBuf)[i].reserve(basePath->len() + strs[i + 1].len() + 1); // room for backslash
				(*pArrBuf)[i] = (*basePath);
				(*pArrBuf)[i].append(L"\\").append(strs[i + 1]); // concat folder + file
			}
			pArrBuf->sort([](const String& a, const String& b)->int { return String::LexicalCompare(a, b, String::Case::INSENS); });
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

bool WindowPopup::getFileSave(const wchar_t *formattedFilter, String *pBuf, const wchar_t *defFile)
{
	OPENFILENAME ofn = { 0 };
	wchar_t tmpBuf[MAX_PATH] = { 0 };

	if(defFile)
		lstrcpy(tmpBuf, defFile);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = formattedFilter;
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"txt"; // apparently could be anything, will just force append of combo selected extension

	bool ret = GetSaveFileName(&ofn) != 0;
	if(ret) (*pBuf) = tmpBuf;
	return ret;
}

bool WindowPopup::getFolderChoose(String *pBuf)
{
	CoInitialize(0);

	/*LPITEMIDLIST pidlRoot = 0;
	if(defFolder)
		SHParseDisplayName(defFolder, NULL, &pidlRoot, 0, NULL);*/

	BROWSEINFO bi = { 0 };
	bi.hwndOwner = this->hWnd();
	bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
	if(!pidl)
		return false; // user cancelled

	wchar_t buf[MAX_PATH] = { 0 };
	if(!SHGetPathFromIDList(pidl, buf))
		return false; // some weird error

	CoUninitialize();
	(*pBuf) = buf;
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

BOOL CALLBACK WindowPopup::_WheelHoverApply(HWND hChild, LPARAM lp)
{
	// Callback to EnumChildWindows(), will run on each child.
	static int uniqueSubId = 1;
	SetWindowSubclass(hChild, _WheelHoverProc, uniqueSubId++, (DWORD_PTR)GetParent(hChild)); // yes, subclass every control
	return TRUE;
}

LRESULT CALLBACK WindowPopup::_WheelHoverProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
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
		RemoveWindowSubclass(hChild, _WheelHoverProc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hChild, msg, wp, lp);
}