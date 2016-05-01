
#include <utility>
#include "Icon.h"

Icon::Icon() :
	_hIcon(nullptr)
{
}

Icon::Icon(Icon&& i)
	: _hIcon(i._hIcon)
{
	i._hIcon = nullptr;
}

Icon& Icon::operator=(Icon&& i)
{
	std::swap(_hIcon, i._hIcon);
	return *this;
}

Icon& Icon::release()
{
	if (_hIcon) {
		DestroyIcon(_hIcon);
		_hIcon = nullptr;
	}
	return *this;
}

Icon& Icon::getFromExplorer(const wchar_t *fileExtension)
{
	release();
	
	wchar_t extens[10] = { L'\0' };
	lstrcpy(extens, L"*.");
	lstrcat(extens, fileExtension); // prefix extension; caller must pass just the 3 letters (or 4, whathever)
	
	SHFILEINFO shfi = { 0 };
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
	_hIcon = shfi.hIcon;
	return *this;
}

Icon& Icon::loadResource(int iconId, int size, HINSTANCE hInst)
{
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	release();
	_hIcon = static_cast<HICON>(LoadImage(hInst ? hInst : GetModuleHandle(nullptr),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, size, size, LR_DEFAULTCOLOR));
	return *this;
}

void Icon::iconToLabel(HWND hStatic, int idIconRes, BYTE size)
{
	// Loads an icon resource into a static control placed on a dialog.
	// On the resource editor, change "Type" property to "Icon".
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	SendMessage(hStatic, STM_SETIMAGE, IMAGE_ICON,
		reinterpret_cast<LPARAM>(static_cast<HICON>(LoadImage(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(GetParent(hStatic), GWLP_HINSTANCE)),
			MAKEINTRESOURCE(idIconRes), IMAGE_ICON, size, size, LR_DEFAULTCOLOR))));
}