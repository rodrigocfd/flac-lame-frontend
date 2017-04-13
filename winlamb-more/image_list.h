/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "icon.h"
#include <CommCtrl.h>

namespace wl {

// Wrapper to image list object from Common Controls library.
class image_list final {
private:
	HIMAGELIST _hImgList;

public:
	image_list()                    : _hImgList(nullptr) { }
	image_list(HIMAGELIST hImgList) : _hImgList(hImgList) { }

	image_list& operator=(const image_list& il) { this->_hImgList = il._hImgList; }
	image_list& operator=(HIMAGELIST hImgList)  { this->_hImgList = hImgList; }

	HIMAGELIST himagelist() const { return this->_hImgList; }

	SIZE resolution() const {
		SIZE buf = {0, 0};
		if (this->_hImgList) {
			ImageList_GetIconSize(this->_hImgList,
				reinterpret_cast<int*>(&buf.cx), reinterpret_cast<int*>(&buf.cy));
		}
		return buf;
	}

	size_t size() const {
		return this->_hImgList ? ImageList_GetImageCount(this->_hImgList) : 0;
	}

	image_list& destroy() {
		if (this->_hImgList) {
			ImageList_Destroy(this->_hImgList);
			this->_hImgList = nullptr;
		}
		return *this;
	}

	bool create(SIZE resolution, UINT flags = ILC_COLOR32, WORD szInitial = 1, WORD szGrow = 1) {
		this->destroy();
		this->_hImgList = ImageList_Create(resolution.cx, resolution.cy, flags,
			static_cast<int>(szInitial), static_cast<int>(szGrow));
		return this->_hImgList != nullptr;
	}

	image_list& load(HICON hIcon) {
		if (this->_hImgList) {
			ImageList_AddIcon(this->_hImgList, hIcon);
		} else {
			OutputDebugStringW(L"ERROR: can't add icon before create image list.\n");
		}
		return *this;
	}

	image_list& load(const icon& ico) {
		return this->load(ico.hicon());
	}

	image_list& load_from_resource(int iconId, HINSTANCE hInst = nullptr) {
		icon tmpIco;
		tmpIco.load_from_resource(iconId, this->resolution(), hInst);
		this->load(tmpIco);
		tmpIco.destroy();
		return *this;
	}

	image_list& load_from_resource(int iconId, HWND hParent) {
		return this->load_from_resource(iconId,
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)));
	}

	image_list& load_from_shell(const wchar_t* fileExtension) {
		if (this->_hImgList) {
			icon::res iRes = icon::resolution_resolve_type(this->resolution());
			if (iRes != icon::res::OTHER) { // system shell allowed resolutions only
				icon tmpIco;
				tmpIco.load_from_shell(fileExtension, iRes);
				this->load(tmpIco);
				tmpIco.destroy();			
			}
		}
		return *this;
	}
};

}//namespace wl