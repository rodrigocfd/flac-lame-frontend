/*!
 * HWND wrapper.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#include <algorithm>
#include <process.h>
#include <Shlobj.h>
#include <VsStyle.h>
#include <UxTheme.h>
#include "Str.h"
#include "Sys.h"
#include "Window.h"
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' " \
  "name='Microsoft.Windows.Common-Controls' " \
  "version='6.0.0.0' " \
  "processorArchitecture='*' " \
  "publicKeyToken='6595b64144ccf1df' " \
  "language='*'\"")
using namespace c4w;
using std::function;
using std::vector;
using std::wstring;

wstring Window::getText() const
{
	int txtLen = GetWindowTextLength(_hWnd);
	wstring buf(txtLen + 1, L'\0');
	GetWindowText(_hWnd, &buf[0], txtLen + 1);
	buf.resize(txtLen);
	return buf;
}

WindowPopup::~WindowPopup()
{
}

static HHOOK _hHookMsgBox = nullptr;
static HWND  _hWndParent = nullptr;
static LRESULT CALLBACK _msgBoxHookProc(int code, WPARAM wp, LPARAM lp)
{
	// http://www.codeguru.com/cpp/w-p/win32/messagebox/print.php/c4541
	if (code == HCBT_ACTIVATE)
	{
		HWND hMsgbox = reinterpret_cast<HWND>(wp);
		RECT rcMsgbox = { 0 }, rcParent = { 0 };

		if (hMsgbox && _hWndParent && GetWindowRect(hMsgbox, &rcMsgbox) && GetWindowRect(_hWndParent, &rcParent))
		{
			RECT  rcScreen = { 0 };
			POINT pos = { 0 };

			SystemParametersInfo(SPI_GETWORKAREA, 0, static_cast<PVOID>(&rcScreen), 0); // size of desktop

			// Adjusted x,y coordinates to message box window.
			pos.x = rcParent.left + (rcParent.right - rcParent.left) / 2 - (rcMsgbox.right - rcMsgbox.left) / 2;
			pos.y =	rcParent.top + (rcParent.bottom - rcParent.top) / 2 - (rcMsgbox.bottom - rcMsgbox.top) / 2;

			// Screen out-of-bounds corrections.
			if (pos.x < 0) {
				pos.x = 0;
			} else if (pos.x + (rcMsgbox.right - rcMsgbox.left) > rcScreen.right) {
				pos.x = rcScreen.right - (rcMsgbox.right - rcMsgbox.left);
			}
			if (pos.y < 0) {
				pos.y = 0;
			} else if (pos.y + (rcMsgbox.bottom - rcMsgbox.top) > rcScreen.bottom) {
				pos.y = rcScreen.bottom - (rcMsgbox.bottom - rcMsgbox.top);
			}

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

static vector<wchar_t> _formatFileFilter(const wchar_t *filterWithPipes)
{
	// Input filter follows same C# syntax:
	// L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*"

	vector<wchar_t> ret(lstrlen(filterWithPipes) + 2, L'\0'); // two terminating nulls
	for (size_t i = 0; i < ret.size() - 1; ++i) {
		ret[i] = (filterWithPipes[i] != L'|') ? filterWithPipes[i] : L'\0';
	}
	return ret;
}

bool WindowPopup::getFileOpen(const wchar_t *filter, wstring& buf)
{
	OPENFILENAME    ofn = { 0 };
	wchar_t         tmpBuf[MAX_PATH] = { 0 };
	vector<wchar_t> zfilter = _formatFileFilter(filter);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFilter = filter;
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;// | OFN_HIDEREADONLY;

	bool ret = GetOpenFileName(&ofn) != 0;
	if (ret) buf = tmpBuf;
	return ret;
}

bool WindowPopup::getFileOpen(const wchar_t *filter, vector<wstring>& arrBuf)
{
	OPENFILENAME    ofn = { 0 };
	vector<wchar_t> multiBuf(65536, L'\0'); // http://www.askjf.com/?q=2179s http://www.askjf.com/?q=2181s
	vector<wchar_t> zfilter = _formatFileFilter(filter);
	arrBuf.clear();

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFile   = &multiBuf[0];
	ofn.nMaxFile    = static_cast<DWORD>(multiBuf.size()); // including terminating null
	ofn.Flags       = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING;
	//ofn.FlagsEx = OFN_EX_NOPLACESBAR;
	// Call to GetOpenFileName() causes "First-chance exception (KernelBase.dll): The RPC server is unavailable."
	// in debug mode, but nothing else happens. The only way to get rid of it was using OFN_EX_NOPLACESBAR flag,
	// don't know why!

	if (GetOpenFileName(&ofn)) {
		vector<wstring> strs = str::ExplodeMultiStr(&multiBuf[0]);
		if (!strs.size()) {
			this->messageBox(L"Error",
				str::Sprintf(L"GetOpenFileName didn't return multiple strings.\n", multiBuf.size()),
				MB_ICONERROR);
			return false;
		}

		if (strs.size() == 1) { // if user selected only 1 file, the string is the full path, and that's all
			arrBuf.emplace_back(strs[0]);
		} else { // user selected 2 or more files
			wstring& basePath = strs[0]; // 1st string is the base path; others are the filenames
			arrBuf.resize(strs.size() - 1); // alloc return buffer

			for (size_t i = 0; i < strs.size() - 1; ++i) {
				arrBuf[i].reserve(basePath.size() + strs[i + 1].size() + 1); // room for backslash
				arrBuf[i] = basePath;
				arrBuf[i].append(L"\\").append(strs[i + 1]); // concat folder + file
			}
			std::sort(arrBuf.begin(), arrBuf.end(), [](const wstring& a, const wstring& b)->bool {
				return str::LexCmp(str::Sens::NO, a, b) < 0;
			});
		}
		return true; // all good
	}

	DWORD errNo = CommDlgExtendedError();
	if (errNo == FNERR_BUFFERTOOSMALL) {
		this->messageBox(L"Error",
			str::Sprintf(L"GetOpenFileName: buffer too small (%d bytes).\n", multiBuf.size()),
			MB_ICONERROR);
	} else if(errNo) {
		this->messageBox(L"Error",
			str::Sprintf(L"GetOpenFileName: failed with error %d.\n", errNo),
			MB_ICONERROR);
	}
	return false;
}

bool WindowPopup::getFileSave(const wchar_t *filter, wstring& buf, const wchar_t *defFile)
{
	OPENFILENAME    ofn = { 0 };
	wchar_t         tmpBuf[MAX_PATH] = { 0 };
	vector<wchar_t> zfilter = _formatFileFilter(filter);

	if (defFile)
		lstrcpy(tmpBuf, defFile);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = this->hWnd();
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"txt"; // apparently could be anything, will just force append of combo selected extension

	bool ret = GetSaveFileName(&ofn) != 0;
	if (ret) buf = tmpBuf;
	return ret;
}

bool WindowPopup::getFolderChoose(wstring& buf)
{
	CoInitialize(nullptr);

	//LPITEMIDLIST pidlRoot = 0;
	//if (defFolder)
		//SHParseDisplayName(defFolder, nullptr, &pidlRoot, 0, nullptr);

	BROWSEINFO bi = { 0 };
	bi.hwndOwner = this->hWnd();
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
	if (!pidl) {
		return false; // user cancelled
	}

	wchar_t tmpbuf[MAX_PATH] = { 0 };
	if (!SHGetPathFromIDList(pidl, tmpbuf)) {
		return false; // some weird error
	}

	CoUninitialize();
	buf = tmpbuf;
	return true;
}

void WindowPopup::setXButton(bool enable)
{
	// Enable/disable the X button to close the window; has no effect on Alt+F4.
	HMENU hMenu = GetSystemMenu(this->hWnd(), FALSE);
	if (hMenu) {
		UINT dwExtra = enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | dwExtra);
	}
}

vector<wstring> WindowPopup::getDroppedFiles(HDROP hDrop)
{
	vector<wstring> files(DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0));
	for (size_t i = 0; i < files.size(); ++i) {
		files[i].resize(DragQueryFile(hDrop, static_cast<UINT>(i), nullptr, 0) + 1, L'\0'); // alloc path string
		DragQueryFile(hDrop, static_cast<UINT>(i), &files[i][0], static_cast<UINT>(files[i].size()));
		files[i].resize(files[i].size() - 1); // trim null
	}
	DragFinish(hDrop);
	std::sort(files.begin(), files.end(), [](const wstring& a, const wstring& b)->bool {
		return str::LexCmp(str::Sens::NO, a, b) < 0;
	});
	return files;
}

static LRESULT CALLBACK _wheelHoverProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg)
	{
	case WM_MOUSEWHEEL:
		if (!(LOWORD(wp) & 0x0800)) { // bitflag not set, this is the first and unprocessed WM_MOUSEWHEEL passage
			HWND hParent = reinterpret_cast<HWND>(refData);
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
			reinterpret_cast<DWORD_PTR>(GetParent(hChild)) ); // yes, subclass every control
		return TRUE;
	}, 0);
}

struct _CbPack { function<void()> cb; };
void WindowPopup::_handleSendOrPostFunction(LPARAM lp)
{
	// This method is called by FramePopup and DialogPopup wndprocs on their ordinary processing.
	_CbPack *cbPack = reinterpret_cast<_CbPack*>(lp);
	cbPack->cb(); // invoke user callback
	delete cbPack; // allocated by _sendOrPostFunction()
}

void WindowPopup::_sendOrPostFunction(function<void()> callback, bool isSend)
{
	// This method is analog to Send/PostMessage, but intended to be called within a separated thread,
	// so a callback function can, tunelled by wndproc, run in the same thread of the window, thus
	// allowing GUI updates. This avoids the user to deal with a custom WM_ message.
	_CbPack *cbPack = new _CbPack{ std::move(callback) }; // will be deleted by _handleSendOrPostFunction()
	isSend ? this->sendMessage(SENDORPOSTMSG, 0, reinterpret_cast<LPARAM>(cbPack)) :
		this->postMessage(SENDORPOSTMSG, 0, reinterpret_cast<LPARAM>(cbPack));
}


WindowCtrl::~WindowCtrl()
{
}

bool WindowCtrl::_drawBorders(WPARAM wp, LPARAM lp)
{
	// Intended to be called within WM_NCPAINT processing.
	if ((GetWindowLongPtr(this->hWnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive())
	{
		DefWindowProc(this->hWnd(), WM_NCPAINT, wp, lp); // this will make system draw the scrollbar for us; days of struggling until this enlightenment

		RECT rc = this->getWindowRect(); // window outmost coordinates, including margins
		this->screenToClient(reinterpret_cast<POINT*>(&rc));
		this->screenToClient(reinterpret_cast<POINT*>(&rc.right));
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