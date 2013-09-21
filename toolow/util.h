//
// Miscellaneous shorthand functions that didn't
// justify a whole class to themselves.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Ptr.h"
#include "String.h"

inline bool hasCtrl()                    { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
inline bool hasShift()                   { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
inline bool within(int x, int a, int b)  { return x >= a && x <= b; }
inline bool between(int x, int a, int b) { return x > a && x < b; }

void        debug(const wchar_t *fmt, ...);
Ptr<String> fmt(const wchar_t *fmt, ...);
int         round(float x);
int         round(double x);
int         ceilmult(int n, int multiple);
DWORD       exec(const wchar_t *cmdLine);
DWORD       shellOpen(const wchar_t *file);
void        popMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo);
void        explodeMultiStr(const wchar_t *multiStr, Array<String> *pBuf);
void        explodeQuotedStr(const wchar_t *quotedStr, Array<String> *pBuf);
inline void CaesarCipher(String *s, int shift) { for(int i = 0, len = s->len(); i < len; ++i) (*s)[i] += shift; }

namespace GetPathTo {
	String*        Exe(String *pBuf);
	String*        _Folder(String *pBuf, BYTE cslid_id);
	inline String* Desktop(String *pBuf) { return _Folder(pBuf, 0); }
	inline String* MyDocs(String *pBuf)  { return _Folder(pBuf, 1); }
	inline String* Roaming(String *pBuf) { return _Folder(pBuf, 2); }
}

struct SysColor {
	enum Value { BUTTON=COLOR_BTNFACE, DESKTOP=COLOR_DESKTOP,
		BLACK=COLOR_BTNTEXT, WHITE=COLOR_WINDOW, GREY=COLOR_APPWORKSPACE };
};