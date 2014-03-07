//
// Miscellaneous shorthand functions that didn't
// justify a whole class to themselves.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "String.h"

inline bool hasCtrl()                    { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
inline bool hasShift()                   { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
inline bool within(int x, int a, int b)  { return x >= a && x <= b; }
inline bool between(int x, int a, int b) { return x > a && x < b; }

int         rounds(float x);
int         rounds(double x);
int         ceilmult(int n, int multiple);
int         indexOfBin(const BYTE *pData, int dataLen, const wchar_t *what, bool asWideChar);
DWORD       exec(const wchar_t *cmdLine);
DWORD       shellOpen(const wchar_t *file);
void        popMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo);
inline void CaesarCipher(String *s, int shift) { for(int i = 0, len = s->len(); i < len; ++i) (*s)[i] += shift; }

enum class SysColor { BUTTON=COLOR_BTNFACE, DESKTOP=COLOR_DESKTOP,
	BLACK=COLOR_BTNTEXT, WHITE=COLOR_WINDOW, GREY=COLOR_APPWORKSPACE };