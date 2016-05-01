
#pragma once
#include <Windows.h>

class Icon final {
private:
	HICON _hIcon;
public:
	~Icon() { release(); }
	Icon();
	Icon(Icon&& i);
	Icon& operator=(Icon&& i);

	HICON hIcon() const { return _hIcon; }
	Icon& release();
	Icon& getFromExplorer(const wchar_t *fileExtension);
	Icon& loadResource(int iconId, int size, HINSTANCE hInst = nullptr);

	static void iconToLabel(HWND hStatic, int idIconRes, BYTE size);
};