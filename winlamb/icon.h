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

	icon& load_from_shell(const wchar_t* fileExtension) {
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

	icon& load_from_resource(int iconId, SIZE resolution, HINSTANCE hInst = nullptr) {
		this->destroy();
		this->_hIcon = static_cast<HICON>(LoadImageW(hInst ? hInst : GetModuleHandleW(nullptr),
			MAKEINTRESOURCE(iconId), IMAGE_ICON,
			static_cast<int>(resolution.cx), static_cast<int>(resolution.cy),
			LR_DEFAULTCOLOR));
		return *this;
	}

	icon& load_from_resource(int iconId, SIZE resolution, HWND hParent) {
		return this->load_from_resource(iconId, resolution,
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)));
	}

	icon& icon_to_label(HWND hStatic) {
		// Loads an icon into a static control; the icon can be safely destroyed then.
		// On the resource editor, change "Type" property to "Icon".
		SendMessageW(hStatic, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(this->_hIcon));
		return *this;
	}
};

}//namespace wl