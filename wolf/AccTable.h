/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <vector>
#include <Windows.h>

namespace wolf {

class AccTable final {
public:
	enum class Mod : BYTE {
		NONE           = 0,
		CTRL           = FCONTROL,
		SHIFT          = FSHIFT,
		ALT            = FALT,
		CTRL_SHIFT_ALT = (FCONTROL | FSHIFT | FALT),
		CTRL_SHIFT     = (FCONTROL | FSHIFT),
		CTRL_ALT       = (FCONTROL | FALT),
		SHIFT_ALT      = (FSHIFT | FALT)
	};

private:
	HACCEL             _hAccel;
	std::vector<ACCEL> _entries;
public:
	~AccTable();
	AccTable();
	AccTable(HACCEL hAccel);
	AccTable(AccTable&& at);
	AccTable& operator=(HACCEL hAccel);
	AccTable& operator=(AccTable&& at);
	HACCEL    hAccel() const;
	void      destroy();
	AccTable& addChar(WORD commandId, char charKey, Mod modifier = Mod::NONE);
	AccTable& addKey(WORD commandId, WORD key, Mod modifier = Mod::NONE);
	bool      create(DWORD *lastError = nullptr);
	int       translate(HWND hWnd, MSG& msg);
};

}//namespace wolf