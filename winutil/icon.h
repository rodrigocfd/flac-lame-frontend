/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <Windows.h>

namespace winutil {

class icon final {
private:
	HICON _hIcon;
public:
	~icon() { release(); }
	icon();
	icon(icon&& i);
	icon& operator=(icon&& i);

	HICON hicon() const { return _hIcon; }
	icon& release();
	icon& get_from_explorer(const wchar_t *fileExtension);
	icon& load_resource(int iconId, int size, HINSTANCE hInst = nullptr);

	static void icon_to_label(HWND hStatic, int idIconRes, BYTE size);
};

}//namespace winutil