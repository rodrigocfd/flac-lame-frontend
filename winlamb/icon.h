/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <utility>
#include <Windows.h>

namespace wl {

// Wrapper to HICON.
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

	icon& load_resource(int iconId, BYTE widthOrHeight, HINSTANCE hInst = nullptr) {
		// The widthOrHeight should be 16, 32, 48 or any other size the icon eventually has.
		this->destroy();
		this->_hIcon = static_cast<HICON>(LoadImageW(hInst ? hInst : GetModuleHandleW(nullptr),
			MAKEINTRESOURCE(iconId), IMAGE_ICON,
			static_cast<int>(widthOrHeight), static_cast<int>(widthOrHeight),
			LR_DEFAULTCOLOR));
		return *this;
	}

	icon& load_resource(int iconId, BYTE widthOrHeight, HWND hParent) {
		// The widthOrHeight should be 16, 32, 48 or any other size the icon eventually has.
		return this->load_resource(iconId, widthOrHeight,
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)));
	}

	icon& icon_to_label(HWND hStatic, int idIconRes, BYTE widthOrHeight) {
		// Loads an icon resource into a static control placed on a dialog.
		// On the resource editor, change "Type" property to "Icon".
		// The widthOrHeight should be 16, 32, 48 or any other size the icon eventually has.
		SendMessageW(hStatic, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(this->_hIcon));
		return *this;
	}
};

}//namespace wl