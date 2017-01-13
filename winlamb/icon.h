/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <utility>
#include <Windows.h>

namespace wl {

class icon final {
private:
	HICON _hIcon;

public:
	~icon()        { this->destroy(); }
	icon()         : _hIcon(nullptr) { }
	icon(icon&& i) : _hIcon(i._hIcon) { i._hIcon = nullptr; }

	HICON hicon() const { return this->_hIcon; }

	icon& operator=(icon&& i) {
		std::swap(this->_hIcon, i._hIcon);
		return *this;
	}

	icon& destroy() {
		if (this->_hIcon) {
			DestroyIcon(this->_hIcon);
			this->_hIcon = nullptr;
		}
		return *this;
	}

	icon& get_from_explorer(const wchar_t* fileExtension) {
		this->destroy();
		wchar_t extens[10] = { L'\0' };
		lstrcpyW(extens, L"*.");
		lstrcatW(extens, fileExtension); // prefix extension; caller must pass just the 3 letters (or 4, whathever)

		SHFILEINFO shfi = { 0 };
		SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
			SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
			SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
		this->_hIcon = shfi.hIcon;
		return *this;
	}

	icon& load_resource(int iconId, int size, HINSTANCE hInst = nullptr) {
		// The size should be 16, 32, 48 or any other size the icon eventually has.
		this->destroy();
		this->_hIcon = static_cast<HICON>(LoadImageW(hInst ? hInst : GetModuleHandleW(nullptr),
			MAKEINTRESOURCE(iconId), IMAGE_ICON, size, size, LR_DEFAULTCOLOR));
		return *this;
	}

	void icon_to_label(HWND hStatic, int idIconRes, BYTE size) const {
		// Loads an icon resource into a static control placed on a dialog.
		// On the resource editor, change "Type" property to "Icon".
		// The size should be 16, 32, 48 or any other size the icon eventually has.
		SendMessageW(hStatic, STM_SETIMAGE, IMAGE_ICON,
			reinterpret_cast<LPARAM>(static_cast<HICON>(LoadImageW(
				reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(GetParent(hStatic), GWLP_HINSTANCE)),
				MAKEINTRESOURCE(idIconRes), IMAGE_ICON, size, size, LR_DEFAULTCOLOR))));
	}
};

}//namespace wl