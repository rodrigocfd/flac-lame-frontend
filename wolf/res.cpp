/*!
 * @file
 * @brief Assorted resources.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "res.h"
#include "str.h"

using namespace wolf;
using namespace wolf::res;
using std::initializer_list;
using std::vector;
using std::wstring;

Date& Date::setNow()
{
	SYSTEMTIME st1 = { 0 };
	GetSystemTime(&st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &_st);

	return *this;
}

Date& Date::setFromFt(const FILETIME& ft)
{
	SYSTEMTIME st1 = { 0 };
	FileTimeToSystemTime(&ft, &st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &_st);

	return *this;
}

Date& Date::setFromMs(LONGLONG ms)
{
	SecureZeroMemory(&_st, sizeof(SYSTEMTIME));
	
	_st.wMilliseconds = ms % 1000;
	ms = (ms - _st.wMilliseconds) / 1000; // now in seconds
	_st.wSecond = ms % 60;
	ms = (ms - _st.wSecond) / 60; // now in minutes
	_st.wMinute = ms % 60;
	ms = (ms - _st.wMinute) / 60; // now in hours
	_st.wHour = ms % 24;
	ms = (ms - _st.wHour) / 24; // now in days

	return *this;
}

LONGLONG Date::getTimestamp() const
{
	// http://www.frenk.com/2009/12/convert-filetime-to-unix-timestamp/
	LARGE_INTEGER date, adjust;
	_StToLi(_st, date);
	adjust.QuadPart = 11644473600000 * 10000; // 100-nanoseconds = milliseconds * 10000
	date.QuadPart -= adjust.QuadPart; // removes the diff between 1970 and 1601
	//return date.QuadPart / 10000000; // converts back from 100-nanoseconds to seconds
	return date.QuadPart / 10000; // to milliseconds; to printf use %I64u
}

LONGLONG Date::minus(const Date &other) const
{
	LARGE_INTEGER liUs, liThem;
	_StToLi(_st, liUs);
	_StToLi(other._st, liThem);
	return (liUs.QuadPart - liThem.QuadPart) / 10000; // 100-nanoseconds to milliseconds; to printf use %I64u
}

Date& Date::addMs(LONGLONG ms)
{
	LARGE_INTEGER li;
	_StToLi(_st, li);
	li.QuadPart += ms * 10000; // milliseconds to 100-nanoseconds
	_LiToSt(li, _st);
	return *this;
}

void Date::_StToLi(const SYSTEMTIME& st, LARGE_INTEGER& li)
{
	FILETIME ft = { 0 };
	SystemTimeToFileTime(&st, &ft);

	li.HighPart = ft.dwHighDateTime;
	li.LowPart = ft.dwLowDateTime;
}

void Date::_LiToSt(const LARGE_INTEGER& li, SYSTEMTIME& st)
{
	FILETIME ft = { 0 };
	ft.dwHighDateTime = li.HighPart;
	ft.dwLowDateTime = li.LowPart;

	FileTimeToSystemTime(&ft, &st);
}


Menu& Menu::addSeparator()
{
	// Intended to be called within WM_CREATE or WM_INITDIALOG.
	this->_createIfNotYet();
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
	return *this;
}

Menu& Menu::addItem(const wchar_t *caption, WORD cmdId)
{
	// Intended to be called within WM_CREATE or WM_INITDIALOG.
	this->_createIfNotYet();
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_STRING, cmdId, caption);
	return *this;
}

Menu& Menu::enableItem(initializer_list<WORD> cmdIds, bool doEnable)
{
	for (const WORD& cmd : cmdIds) {
		EnableMenuItem(_hMenu, cmd, MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
	}
	return *this;
}

Menu Menu::addSubmenu(const wchar_t *caption)
{
	// Intended to be called within WM_CREATE or WM_INITDIALOG.
	this->_createIfNotYet();	
	Menu sub = CreatePopupMenu();
	AppendMenu(_hMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(sub.hMenu()), caption);
	return sub; // return new submenu, so it can be edited
}

void MenuContext::popAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo)
{
	// The pt argument coordinates can be relative to the hWndCoordsRelativeTo window,
	// otherwise they are relative to the hParent window.

	if (!_hMenu) return;
	ClientToScreen(hWndCoordsRelativeTo ? hWndCoordsRelativeTo : hParent, &pt); // to screen coordinates
	SetForegroundWindow(hParent);
	TrackPopupMenu(_hMenu, 0, pt.x, pt.y, 0, hParent, nullptr); // owned by window, so messages go to it
	PostMessage(hParent, WM_NULL, 0, 0); // http://msdn.microsoft.com/en-us/library/ms648002%28VS.85%29.aspx
}


void AccelTable::destroy()
{
	if (_hAccel) {
		DestroyAcceleratorTable(_hAccel);
		_hAccel = nullptr;
	}
}

void AccelTable::add(initializer_list<Key> keys)
{
	ACCEL *entries = new ACCEL[keys.size()];
	int i = 0;

	for (Key k : keys) {
		entries[i].key = k.key;
		entries[i].cmd = k.cmdId;
		entries[i].fVirt = FNOINVERT |
			static_cast<BYTE>(k.type) |
			static_cast<BYTE>(k.modifiers);
		++i;
	}

	_hAccel = CreateAcceleratorTable(entries, static_cast<int>(keys.size()));
	if (!_hAccel) {
		str::Dbg(L"ERROR: CreateAcceleratorTable failed, error #%d.\n", GetLastError());
	}

	delete[] entries;
}


Font& Font::create(const wchar_t *name, int size, bool bold, bool italic)
{
	this->release();

	LOGFONT lf = { 0 };
	lstrcpy(lf.lfFaceName, name);
	lf.lfHeight = -(size + 3);
	lf.lfWeight = bold ? FW_BOLD : FW_DONTCARE;
	lf.lfItalic = static_cast<BYTE>(italic);
	_hFont = CreateFontIndirect(&lf);
	return *this;
}

Font& Font::cloneFrom(const Font& font)
{
	this->release();

	LOGFONT lf = { 0 };
	GetObject(font._hFont, sizeof(LOGFONT), &lf);
	_hFont = CreateFontIndirect(&lf);
	return *this;
}

Font::Info Font::getInfo() const
{
	LOGFONT lf = { 0 };
	GetObject(_hFont, sizeof(LOGFONT), &lf);

	Info info;
	_LogfontToInfo(lf, info);
	return info;
}

Font& Font::apply(HWND hWnd)
{
	if (_hFont) {
		SendMessage(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(_hFont), MAKELPARAM(FALSE, 0)); // to window itself only
	}
	return *this;
}

Font& Font::applyOnChildren(HWND hWnd)
{
	if (_hFont) {
		// http://stackoverflow.com/questions/18367641/use-createthread-with-a-lambda
		EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lp)->BOOL { // propagate to children
			SendMessage(hWnd, WM_SETFONT,
				reinterpret_cast<WPARAM>(reinterpret_cast<HFONT>(lp)),
				MAKELPARAM(FALSE, 0)); // will run on each child
			return TRUE;
		}, reinterpret_cast<LPARAM>(_hFont));
	}
	return *this;
}

bool Font::Exists(const wchar_t *name)
{
	// http://cboard.cprogramming.com/windows-programming/90066-how-determine-if-font-support-unicode.html
	bool isInstalled = false;
	HDC hdc = GetDC(nullptr);
	EnumFontFamilies(hdc, name,
		(FONTENUMPROC)[](const LOGFONT *lpelf, const TEXTMETRIC *lpntm, DWORD fontType, LPARAM lp)->int {
			bool *pIsInstalled = reinterpret_cast<bool*>(lp);
			*pIsInstalled = true; // if we're here, font does exist
			return 0;
		}, reinterpret_cast<LPARAM>(&isInstalled));
	ReleaseDC(nullptr, hdc);
	return isInstalled;
}

Font::Info Font::GetDefaultDialogFontInfo()
{
	OSVERSIONINFO ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

	NONCLIENTMETRICS ncm = { 0 };
	ncm.cbSize = sizeof(ncm);
	if (ovi.dwMajorVersion < 6) { // below Vista
		ncm.cbSize -= sizeof(ncm.iBorderWidth);
	}
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0); // default system font

	Info info;
	_LogfontToInfo(ncm.lfMenuFont, info);
	return info;
}

void Font::_LogfontToInfo(const LOGFONT& lf, Font::Info& info)
{
	lstrcpy(info.name, lf.lfFaceName);
	info.size = -(lf.lfHeight + 3);
	info.bold = (lf.lfWeight == FW_BOLD);
	info.italic = (lf.lfItalic == TRUE);
}


Icon& Icon::getFromExplorer(const wchar_t *fileExtension)
{
	this->free();
	wchar_t extens[10];
	lstrcpy(extens, L"*.");
	lstrcat(extens, fileExtension); // prefix extension; caller must pass just the 3 letters (or 4, whathever)
	SHFILEINFO shfi = { 0 };
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
	_hIcon = shfi.hIcon;
	return *this;
}

Icon& Icon::getFromResource(int iconId, int size, HINSTANCE hInst)
{
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	_hIcon = static_cast<HICON>(LoadImage(hInst ? hInst : GetModuleHandle(nullptr),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, size, size, LR_DEFAULTCOLOR) );
	return *this;
}

void Icon::IconToLabel(HWND hStatic, int idIconRes, BYTE size)
{
	// Loads an icon resource into a static control placed on a dialog.
	// On the resource editor, change "Type" property to "Icon".
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	SendMessage(hStatic, STM_SETIMAGE, IMAGE_ICON,
		reinterpret_cast<LPARAM>(static_cast<HICON>( LoadImage(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(GetParent(hStatic), GWLP_HINSTANCE)),
			MAKEINTRESOURCE(idIconRes), IMAGE_ICON, size, size, LR_DEFAULTCOLOR) )));
}


DC::DC(HWND hwnd, HDC hDC)
	: _hWnd(hwnd), _hdc(hDC)
{
	RECT rcClient = { 0 };
	GetClientRect(_hWnd, &rcClient); // let's keep available width & height
	this->cx = rcClient.right; // these variables are public
	this->cy = rcClient.bottom;
}

DC& DC::setBkColor(COLORREF color)
{
	SetBkColor(_hdc, color == -1 ? // default?
		this->getBkBrushColor() : color);
	return *this;
}

COLORREF DC::getBkBrushColor()
{
	ULONG_PTR hbrBg = GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND);
	if(hbrBg > 100) {
		// The hbrBackground is a brush handle, not a system color constant.
		// This 100 value is arbitrary, based on system color constants like COLOR_BTNFACE.
		LOGBRUSH logBrush;
		GetObject(reinterpret_cast<HBRUSH>(hbrBg), sizeof(LOGBRUSH), &logBrush);
		return logBrush.lbColor;
	}
	return GetSysColor(static_cast<int>(hbrBg) - 1);
}

DC& DC::drawText(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags)
{
	RECT rc = { x, y, x + cx, y + cy };
	DrawText(_hdc, text, lstrlen(text), &rc, fmtFlags); // DT_LEFT|DT_TOP is zero
	return *this;
}

DC& DC::polygon(int left, int top, int right, int bottom)
{
	POINT pts[] = {
		{ left, top },
		{ left, bottom },
		{ right, bottom },
		{ right, top }
	};
	return this->polygon(pts, 4);
}

DCBuffered::DCBuffered(HWND hwnd)
	: DCSimple(hwnd)
{
	_hdc = CreateCompatibleDC(_ps.hdc); // overwrite our painting HDC
	_hBmp = CreateCompatibleBitmap(_ps.hdc, this->cx, this->cy);
	_hBmpOld = static_cast<HBITMAP>(SelectObject(_hdc, _hBmp));
	
	RECT rcClient = { 0, 0, this->cx, this->cy };
	FillRect(_hdc, &rcClient,
		reinterpret_cast<HBRUSH>(GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND)));
}

DCBuffered::~DCBuffered()
{
	BITMAP bm = { 0 }; // http://www.ureader.com/msg/14721900.aspx
	GetObject(_hBmp, sizeof(bm), &bm);
	BitBlt(_ps.hdc, 0, 0, bm.bmWidth, bm.bmHeight, nullptr, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(_hdc, _hBmpOld));
	DeleteObject(_hBmp);
	DeleteDC(_hdc);
}