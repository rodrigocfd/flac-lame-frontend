/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "Font.h"
using namespace wolf;

Font::Info::Info()
	: size(0), bold(false), italic(false)
{
	*this->name = L'\0';
}

Font::Info::Info(const Info& other)
{
	this->operator=(other);
}

Font::Info& Font::Info::operator=(const Info& other)
{
	this->size = other.size;
	this->bold = other.bold;
	this->italic = other.italic;
	lstrcpy(this->name, other.name);
	return *this;
}


Font::~Font()
{
	this->release();
}

Font::Font()
	: _hFont(nullptr)
{
}

Font::Font(Font&& f)
	: _hFont(f._hFont)
{
	f._hFont = nullptr;
}

Font& Font::operator=(HFONT hfont)
{
	this->release();
	this->_hFont = hfont;
	return *this;
}

Font& Font::operator=(Font&& f)
{
	std::swap(this->_hFont, f._hFont);
	return *this;
}

HFONT Font::hFont() const
{
	return this->_hFont;
}

void Font::release()
{
	if (this->_hFont) {
		DeleteObject(this->_hFont);
		this->_hFont = nullptr;
	}
}

Font& Font::create(const wchar_t *name, int size, bool bold, bool italic)
{
	this->release();

	LOGFONT lf = { 0 };
	lstrcpy(lf.lfFaceName, name);
	lf.lfHeight = -(size + 3);
	lf.lfWeight = bold ? FW_BOLD : FW_DONTCARE;
	lf.lfItalic = static_cast<BYTE>(italic);
	this->_hFont = CreateFontIndirect(&lf);
	return *this;
}

Font& Font::create(const Info& info)
{
	return this->create(info.name, info.size, info.bold, info.italic);
}

Font& Font::cloneFrom(const Font& font)
{
	this->release();

	LOGFONT lf = { 0 };
	GetObject(font._hFont, sizeof(LOGFONT), &lf);
	this->_hFont = CreateFontIndirect(&lf);
	return *this;
}

Font::Info Font::getInfo() const
{
	LOGFONT lf = { 0 };
	GetObject(this->_hFont, sizeof(LOGFONT), &lf);

	Info info;
	_logfontToInfo(lf, info);
	return info;
}

Font& Font::apply(HWND hWnd)
{
	if (this->_hFont) {
		SendMessage(hWnd, WM_SETFONT,
			reinterpret_cast<WPARAM>(this->_hFont),
			MAKELPARAM(FALSE, 0)); // to window itself only
	}
	return *this;
}

Font& Font::applyOnChildren(HWND hWnd)
{
	if (this->_hFont) {
		EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lp)->BOOL { // propagate to children
			SendMessage(hWnd, WM_SETFONT,
				reinterpret_cast<WPARAM>(reinterpret_cast<HFONT>(lp)),
				MAKELPARAM(FALSE, 0)); // will run on each child
			return TRUE;
		}, reinterpret_cast<LPARAM>(this->_hFont));
	}
	return *this;
}

bool Font::exists(const wchar_t *name)
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

Font::Info Font::getDefaultDialogFontInfo()
{
	OSVERSIONINFO ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof(ovi);

#pragma warning (disable: 4996)
	// http://www.codeproject.com/Articles/678606/Part-Overcoming-Windows-s-deprecation-of-GetVe
	GetVersionEx(&ovi);
#pragma warning (default: 4996)

	NONCLIENTMETRICS ncm = { 0 };
	ncm.cbSize = sizeof(ncm);
	if (ovi.dwMajorVersion < 6) { // below Vista
		ncm.cbSize -= sizeof(ncm.iBorderWidth);
	}
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0); // default system font

	Info info;
	_logfontToInfo(ncm.lfMenuFont, info);
	return info;
}

void Font::_logfontToInfo(const LOGFONT& lf, Info& info)
{
	lstrcpy(info.name, lf.lfFaceName);
	info.size = -(lf.lfHeight + 3);
	info.bold = (lf.lfWeight == FW_BOLD);
	info.italic = (lf.lfItalic == TRUE);
}