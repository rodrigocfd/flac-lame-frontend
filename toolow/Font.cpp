//
// Font automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma warning(disable:4996) // GetVersionEx is deprecated for Win8.1, won't affect current behaviour
#include "Font.h"

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
	_LogfontToInfo(&lf, &info);
	return info;
}

Font& Font::apply(HWND hWnd)
{
	if(_hFont)
		SendMessage(hWnd, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(FALSE, 0)); // to window itself only
	return *this;
}

Font& Font::applyOnChildren(HWND hWnd)
{
	if(_hFont) {
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
	if(ovi.dwMajorVersion < 6) // below Vista
		ncm.cbSize -= sizeof(ncm.iBorderWidth);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0); // default system font

	Info info;
	_LogfontToInfo(&ncm.lfMenuFont, &info);
	return info;
}

void Font::_LogfontToInfo(const LOGFONT *lf, Font::Info *pInfo)
{
	lstrcpy(pInfo->name, lf->lfFaceName);
	pInfo->size = -(lf->lfHeight + 3);
	pInfo->bold = (lf->lfWeight == FW_BOLD);
	pInfo->italic = (lf->lfItalic == TRUE);
}