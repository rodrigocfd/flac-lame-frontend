/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace wolf {

class WindowDialogTemplate final {
public:
	class Item final {
	public:
		DWORD        style;
		DWORD        exStyle;
		POINT        pos;
		SIZE         size;
		DWORD        id;
		std::wstring windowClass;
		std::wstring title;

		explicit Item(BYTE *pData, BYTE **pPast);
	};

	DWORD             style;
	DWORD             exStyle;
	POINT             pos;
	SIZE              size;
	std::wstring      title;
	std::wstring      fontFace;
	WORD              fontPointSize;
	WORD              fontWeight;
	bool              fontItalic;
	BYTE              fontCharset;
	std::vector<Item> children;

	bool load(HINSTANCE hInst, int dialogId, std::wstring *pErr = nullptr);
};

}//namespace wolf