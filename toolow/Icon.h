//
// HICON automation.
// Night of Friday, November 9, 2012.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include <Windows.h>

class Icon {
public:
	Icon()  : _hIcon(0) { }
	~Icon() { this->free(); }

	HICON hIcon() const          { return _hIcon; }
	Icon& free()                 { if(_hIcon) ::DestroyIcon(_hIcon); return *this; }
	Icon& operator=(HICON hIcon) { _hIcon = hIcon; return *this; }
	
	Icon& getFromExplorer(const wchar_t *fileExtension) {
		this->free();
		wchar_t extens[10];
		::lstrcpy(extens, L"*.");
		::lstrcat(extens, fileExtension); // prefix extension; caller must pass just the 3 letters (or 4, whathever)
		SHFILEINFO shfi = { 0 };
		::SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
			SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		::SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
			SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
		_hIcon = shfi.hIcon;
		return *this;
	}

	Icon& getFromResource(int iconId, int size, HINSTANCE hInst=0) {
		// The size should be 16, 32, 48 or any other size the icon eventually has.
		_hIcon = (HICON)::LoadImage(hInst ? hInst : ::GetModuleHandle(0),
			MAKEINTRESOURCE(iconId), IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
		return *this;
	}

	static void IconToLabel(HWND hStatic, int idIconRes, BYTE size) {
		// Loads an icon resource into a static control placed on a dialog.
		// On the resource editor, change "Type" property to "Icon".
		// The size should be 16, 32, 48 or any other size the icon eventually has.
		::SendMessage(hStatic, STM_SETIMAGE, IMAGE_ICON,
			(LPARAM)(HICON)::LoadImage(
				(HINSTANCE)::GetWindowLongPtr(::GetParent(hStatic), GWLP_HINSTANCE),
				MAKEINTRESOURCE(idIconRes), IMAGE_ICON, size, size, LR_DEFAULTCOLOR) );
	}

private:
	HICON _hIcon;
};