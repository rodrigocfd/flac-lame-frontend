/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <utility>
#include <Windows.h>
#include <commoncontrols.h>

namespace wl {

// Wrapper to HICON.
class icon final {
private:
	HICON _hIcon;

public:
	enum class res : BYTE {
		SMALL16      = SHIL_SMALL,
		LARGE32      = SHIL_LARGE,
		EXTRALARGE48 = SHIL_EXTRALARGE,
		JUMBO256     = SHIL_JUMBO,
		OTHER        = SHIL_LAST + 1
	};

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

	icon& load_from_shell(const wchar_t* fileExtension, res resolution) {
		this->destroy();
		wchar_t extens[16] = { L'\0' };
		lstrcpyW(extens, (fileExtension[0] == L'.') ? L"*" : L"*.");
		lstrcatW(extens, fileExtension);

		CoInitialize(nullptr);
		SHFILEINFO shfi = { 0 };

		if (resolution == res::SMALL16 || resolution == res::LARGE32) { // http://stackoverflow.com/a/28015423
			SHGetFileInfoW(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
				SHGFI_USEFILEATTRIBUTES | SHGFI_ICON |
				(resolution == res::SMALL16 ? SHGFI_SMALLICON : SHGFI_LARGEICON));
			this->_hIcon = shfi.hIcon;
		} else if (resolution != res::OTHER) {
			IImageList* pImgList = nullptr; // http://stackoverflow.com/a/30496252
			if (SUCCEEDED(SHGetImageList(static_cast<int>(resolution), IID_IImageList,
				reinterpret_cast<void**>(&pImgList))))
			{
				SHGetFileInfoW(extens,
					FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
					SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
				this->_hIcon = ImageList_GetIcon(reinterpret_cast<HIMAGELIST>(pImgList),
					shfi.iIcon, ILD_NORMAL);
			}
		}

		CoUninitialize();
		return *this;
	}

	icon& icon_to_label(HWND hStatic) {
		// Loads an icon into a static control; the icon can be safely destroyed then.
		// On the resource editor, change "Type" property to "Icon".
		SendMessageW(hStatic, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(this->_hIcon));
		return *this;
	}

	SIZE resolution() const {
		SIZE sz = { 0 }; // http://stackoverflow.com/a/13923853
		if (this->_hIcon) {
			BITMAP bmp = { 0 };
			ICONINFO nfo = { 0 };
			GetIconInfo(this->_hIcon, &nfo);
			
			if (nfo.hbmColor) {
				int nWrittenBytes = GetObjectW(nfo.hbmColor, sizeof(bmp), &bmp);
				if (nWrittenBytes > 0) {
					sz.cx = bmp.bmWidth;
					sz.cy = bmp.bmHeight;
					//myinfo.nBitsPerPixel = bmp.bmBitsPixel;
				}
			} else if (nfo.hbmMask) {
				int nWrittenBytes = GetObjectW(nfo.hbmMask, sizeof(bmp), &bmp);
				if (nWrittenBytes > 0) {
					sz.cx = bmp.bmWidth;
					sz.cy = bmp.bmHeight / 2;
					//myinfo.nBitsPerPixel = 1;
				}
			}

			if (nfo.hbmColor) DeleteObject(nfo.hbmColor);
			if (nfo.hbmMask) DeleteObject(nfo.hbmMask);
		}
		return sz;
	}

	res resolution_type() const {
		return resolution_resolve_type(this->resolution());
	}

	static res resolution_resolve_type(SIZE sz) {
		if (sz.cx == 16 && sz.cy == 16) return res::SMALL16;
		else if (sz.cx == 32 && sz.cy == 32) return res::LARGE32;
		else if (sz.cx == 48 && sz.cy == 48) return res::EXTRALARGE48;
		else if (sz.cx == 256 && sz.cy == 256) return res::JUMBO256;
		return res::OTHER;
	}
};

}//namespace wl