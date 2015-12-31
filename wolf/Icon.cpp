/**
* Part of WOLF - WinAPI Object Lambda Framework
* @author Rodrigo Cesar de Freitas Dias
* @see https://github.com/rodrigocfd/wolf
*/

#include <utility>
#include "Icon.h"
using namespace wolf;

Icon::~Icon()
{
	this->release();
}

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
	std::swap(this->_hIcon, i._hIcon);
	return *this;
}

HICON Icon::hIcon() const
{
	return this->_hIcon;
}

Icon& Icon::release()
{
	if (this->_hIcon) {
		DestroyIcon(this->_hIcon);
		this->_hIcon = nullptr;
	}
	return *this;
}

Icon& Icon::getFromExplorer(const wchar_t *fileExtension)
{
	this->release();
	
	wchar_t extens[10] = { L'\0' };
	lstrcpy(extens, L"*.");
	lstrcat(extens, fileExtension); // prefix extension; caller must pass just the 3 letters (or 4, whathever)
	
	SHFILEINFO shfi = { 0 };
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
	this->_hIcon = shfi.hIcon;
	return *this;
}

Icon& Icon::loadResource(int iconId, int size, HINSTANCE hInst)
{
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	this->release();
	this->_hIcon = static_cast<HICON>(LoadImage(hInst ? hInst : GetModuleHandle(nullptr),
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