/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <string>
#include <Windows.h>

namespace winlamb {

class font final {
private:
	HFONT _hFont;
public:
	~font()
	{
		release();
	}

	font() : _hFont(nullptr)
	{
	}

	font(font&& f) : _hFont(f._hFont)
	{
		f._hFont = nullptr;
	}

	font& operator=(font&& f)
	{
		std::swap(_hFont, f._hFont);
		return *this;
	}

	font& release()
	{
		if (_hFont) {
			DeleteObject(_hFont);
			_hFont = nullptr;
		}
		return *this;
	}
	
	HFONT hfont() const
	{
		return _hFont;
	}

	font& create(const LOGFONT& lf)
	{
		release();
		_hFont = CreateFontIndirect(&lf);
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
		return create(ncm.lfMenuFont); // Tahoma/Segoe
	}

	static void set_ui_on_children(HWND hParent)
	{
		// Helper function to set UI font on all chidren of a window.

		static font font; // keep one single font instance for all windows
		if (!font._hFont) font.create_ui();

		SendMessage(hParent, WM_SETFONT,
			reinterpret_cast<WPARAM>(font._hFont),
			MAKELPARAM(FALSE, 0));
		EnumChildWindows(hParent, [](HWND hWnd, LPARAM lp)->BOOL {
			SendMessage(hWnd, WM_SETFONT,
				reinterpret_cast<WPARAM>(reinterpret_cast<HFONT>(lp)),
				MAKELPARAM(FALSE, 0)); // will run on each child
			return TRUE;
		}, reinterpret_cast<LPARAM>(font._hFont));
	}
};

}//namespace winlamb