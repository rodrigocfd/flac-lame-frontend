/**
* Part of WOLF - WinAPI Object Lambda Framework
* @author Rodrigo Cesar de Freitas Dias
* @see https://github.com/rodrigocfd/wolf
*/

#pragma once
#include <Windows.h>

namespace wolf {

class Icon final {
private:
	HICON _hIcon;
public:
	~Icon();
	Icon();
	Icon(Icon&& i);
	Icon& operator=(Icon&& i);
	HICON hIcon() const;
	Icon& release();
	Icon& getFromExplorer(const wchar_t *fileExtension);
	Icon& loadResource(int iconId, int size, HINSTANCE hInst = nullptr);

	static void iconToLabel(HWND hStatic, int idIconRes, BYTE size);
};

}//namespace wolf