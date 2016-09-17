/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>

namespace winlamb {

class font final {
private:
	HFONT _hFont;
public:
	~font()        { this->release(); }
	font()         : _hFont(nullptr)  { }
	font(font&& f) : _hFont(f._hFont) { f._hFont = nullptr; }
	
	HFONT hfont() const { return this->_hFont; }

	font& operator=(font&& f)
	{
		std::swap(this->_hFont, f._hFont);
		return *this;
	}

	font& release()
	{
		if (this->_hFont) {
			DeleteObject(this->_hFont);
			this->_hFont = nullptr;
		}
		return *this;
	}
	
	font& create(const LOGFONT& lf)
	{
		this->release();
		this->_hFont = CreateFontIndirect(&lf);
		return *this;
	}

	font& create_ui()
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
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
		return this->create(ncm.lfMenuFont); // Tahoma/Segoe
	}

	static void set_ui_on_children(HWND hParent)
	{
		static font oneFont; // keep one single font instance for all windows
		if (!oneFont._hFont) oneFont.create_ui();

		SendMessage(hParent, WM_SETFONT,
			reinterpret_cast<WPARAM>(oneFont._hFont),
			MAKELPARAM(FALSE, 0));
		EnumChildWindows(hParent, [](HWND hWnd, LPARAM lp)->BOOL {
			SendMessage(hWnd, WM_SETFONT,
				reinterpret_cast<WPARAM>(reinterpret_cast<HFONT>(lp)),
				MAKELPARAM(FALSE, 0)); // will run on each child
			return TRUE;
		}, reinterpret_cast<LPARAM>(oneFont._hFont));
	}
};

}//namespace winlamb