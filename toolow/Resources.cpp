//
// Automation for some resources.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
//

#pragma warning(disable:4996) // GetVersionEx is deprecated for Win8.1, won't affect current behaviour
#include "Resources.h"

Font& Font::create(const wchar_t *name, int size, bool bold, bool italic)
{
	this->release();

	LOGFONT lf = { 0 };
	lstrcpy(lf.lfFaceName, name);
	lf.lfHeight = -(size + 3);
	lf.lfWeight = bold ? FW_BOLD : FW_DONTCARE;
	lf.lfItalic = (BYTE)italic;
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
	if (_hFont)
		SendMessage(hWnd, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(FALSE, 0)); // to window itself only
	return *this;
}

Font& Font::applyOnChildren(HWND hWnd)
{
	if (_hFont) {
		// http://stackoverflow.com/questions/18367641/use-createthread-with-a-lambda
		EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lp)->BOOL { // propagate to children
			SendMessage(hWnd, WM_SETFONT,
				(WPARAM)(HFONT)lp, MAKELPARAM(FALSE, 0)); // will run on each child
			return TRUE;
		}, (LPARAM)_hFont);
	}
	return *this;
}

bool Font::Exists(const wchar_t *name)
{
	// http://cboard.cprogramming.com/windows-programming/90066-how-determine-if-font-support-unicode.html
	bool isInstalled = false;
	HDC hdc = GetDC(nullptr);
	EnumFontFamilies(hdc, name, (FONTENUMPROC)[](const LOGFONT *lpelf, const TEXTMETRIC *lpntm, DWORD fontType, LPARAM lp)->int {
		bool *pIsInstalled = (bool*)lp;
		*pIsInstalled = true; // if we're here, font does exist
		return 0;
	}, (LPARAM)&isInstalled);
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
	if (ovi.dwMajorVersion < 6) // below Vista
		ncm.cbSize -= sizeof(ncm.iBorderWidth);
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
	_hIcon = (HICON)LoadImage(hInst ? hInst : GetModuleHandle(nullptr),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
	return *this;
}

void Icon::IconToLabel(HWND hStatic, int idIconRes, BYTE size)
{
	// Loads an icon resource into a static control placed on a dialog.
	// On the resource editor, change "Type" property to "Icon".
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	SendMessage(hStatic, STM_SETIMAGE, IMAGE_ICON,
		(LPARAM)(HICON)LoadImage(
			(HINSTANCE)GetWindowLongPtr(GetParent(hStatic), GWLP_HINSTANCE),
			MAKEINTRESOURCE(idIconRes), IMAGE_ICON, size, size, LR_DEFAULTCOLOR) );
}


Menu& Menu::createMain(HWND owner) {
	this->destroy();
	_hMenu = CreateMenu(); // to be used as a main window menu
	this->appendItem(L"_DUMMY_", WM_APP-2); // avoids further call to DrawMenuBar(), which would demand HWND again
	SetMenu(owner, _hMenu);
	return *this;
}

Menu& Menu::createPopup() {
	this->destroy();
	_hMenu = CreatePopupMenu(); // to be used as a popup menu
	return *this;
}

Menu& Menu::appendSeparator() {
	this->_checkDummyEntry();
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
	return *this;
}

Menu& Menu::appendItem(const wchar_t *caption, WORD cmdId) {
	this->_checkDummyEntry();
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_STRING, cmdId, caption);
	return *this;
}

Menu& Menu::enableItem(initializer_list<WORD> cmdIds, bool doEnable) {
	for (const WORD& cmd : cmdIds)
		EnableMenuItem(_hMenu, cmd, MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
	return *this;
}

Menu Menu::appendSubmenu(const wchar_t *caption) {
	this->_checkDummyEntry();
	
	Menu sub;
	sub.createPopup();
	AppendMenu(_hMenu, MF_STRING | MF_POPUP, (UINT_PTR)sub.hMenu(), caption);
	return sub; // return new submenu, so it can be edited
}

void Menu::_checkDummyEntry() {
	if (size() == 1 && GetMenuItemID(_hMenu, 0) == WM_APP-2)
		DeleteMenu(_hMenu, 0, MF_BYPOSITION); // delete dummy, if any
}