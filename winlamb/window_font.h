/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <Windows.h>

namespace winlamb {

class window_font final {
private:
	HFONT _hFont;
public:
	~window_font()
	{
		release();
	}

	window_font() : _hFont(nullptr)
	{
	}

	window_font(window_font&& f) : _hFont(f._hFont)
	{
		f._hFont = nullptr;
	}

	window_font& operator=(window_font&& f)
	{
		std::swap(_hFont, f._hFont);
		return *this;
	}

	window_font& release()
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

	window_font& create(const LOGFONT& lf)
	{
		release();
		_hFont = CreateFontIndirect(&lf);
		return *this;
	}

	window_font& create_ui()
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

		static window_font font; // keep one single font instance for all windows
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