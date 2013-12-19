
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

Font& Font::cloneFrom(const Font *pFont)
{
	this->release();

	LOGFONT lf = { 0 };
	GetObject(pFont->_hFont, sizeof(LOGFONT), &lf);
	_hFont = CreateFontIndirect(&lf);
	return *this;
}

Font::Info* Font::getInfo(Font::Info *pInfo) const
{
	LOGFONT lf = { 0 };
	GetObject(_hFont, sizeof(LOGFONT), &lf);
	_LogfontToInfo(&lf, pInfo);
	return pInfo; // return same passed buffer
}

Font& Font::apply(HWND hWnd)
{
	if(_hFont)
		SendMessage(hWnd, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(FALSE, 0)); // to window itself only
	return *this;
}

Font& Font::applyOnChildren(HWND hWnd)
{
	if(_hFont)
		EnumChildWindows(hWnd, _ApplyOnChild, (LPARAM)_hFont); // propagate to children
	return *this;
}

bool Font::Exists(const wchar_t *name)
{
	// http://cboard.cprogramming.com/windows-programming/90066-how-determine-if-font-support-unicode.html
	bool isInstalled = false;
	HDC hdc = GetDC(0);
	EnumFontFamilies(hdc, name, (FONTENUMPROC)_EnumFontFamProc, (LPARAM)&isInstalled);
	ReleaseDC(0, hdc);
	return isInstalled;
}

void Font::GetDefaultDialogFontInfo(Font::Info *pInfo)
{
	OSVERSIONINFO ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

	NONCLIENTMETRICS ncm = { 0 };
	ncm.cbSize = sizeof(ncm);
	if(ovi.dwMajorVersion < 6) // below Vista
		ncm.cbSize -= sizeof(ncm.iBorderWidth);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0); // default system font

	_LogfontToInfo(&ncm.lfMenuFont, pInfo);
}

void Font::_LogfontToInfo(const LOGFONT *lf, Font::Info *pInfo)
{
	lstrcpy(pInfo->name, lf->lfFaceName);
	pInfo->size = -(lf->lfHeight + 3);
	pInfo->bold = (lf->lfWeight == FW_BOLD);
	pInfo->italic = (lf->lfItalic == TRUE);
}

BOOL CALLBACK Font::_ApplyOnChild(HWND hWnd, LPARAM lp)
{
	SendMessage(hWnd, WM_SETFONT,
		(WPARAM)(HFONT)lp, MAKELPARAM(FALSE, 0)); // will run on each child
	return TRUE;
}

int CALLBACK Font::_EnumFontFamProc(ENUMLOGFONT *lpelf, NEWTEXTMETRIC *lpntm, DWORD FontType, LPARAM lp)
{
	*((bool*)lp) = true; // if we're here, font does exist
	return 0;
}