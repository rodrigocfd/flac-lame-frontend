/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowDialogTemplate.h"
#include "Str.h"
using namespace wolf;
using std::wstring;
#define DWORD_ALIGN(x) x = ((x + 3) & ~3)

namespace wolf {

struct DLGTEMPLATEEX final {
	WORD      dlgVer;
	WORD      signature;
	DWORD     helpID;
	DWORD     exStyle;
	DWORD     style;
	WORD      cDlgItems;
	short     x;
	short     y;
	short     cx;
	short     cy;
	/*sz_Or_Ord menu;
	sz_Or_Ord windowClass;
	WCHAR     title[titleLen];
	WORD      pointsize;
	WORD      weight;
	BYTE      italic;
	BYTE      charset;
	WCHAR     typeface[stringLen];*/
};

struct DLGITEMTEMPLATEEX final {
	DWORD     helpID;
	DWORD     exStyle;
	DWORD     style;
	short     x;
	short     y;
	short     cx;
	short     cy;
	DWORD     id;
	/*sz_Or_Ord windowClass;
	sz_Or_Ord title;
	WORD      extraCount;*/
};

}//namespace wolf


static void _dluToPx(HINSTANCE hInst, DLGTEMPLATEEX *pTpl, WindowDialogTemplate* pObj)
{
	static bool firstPass = true; // ratios may change after main window is created
	static double dlgX = 0, dlgY = 0; // dialog window ratio
	static double itemX = 0, itemY = 0; // control item ratio

	if (!itemX) {
		HWND hdlg = CreateDialogIndirect(hInst, reinterpret_cast<DLGTEMPLATE*>(pTpl), nullptr, // ghost window
			[](HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)->INT_PTR { return FALSE; });
		RECT rc = { 0 };
		GetWindowRect(hdlg, &rc);
		dlgX = static_cast<double>(rc.right - rc.left) / pTpl->cx;
		dlgY = static_cast<double>(rc.bottom - rc.top) / pTpl->cy;

		HWND hchild = nullptr; // first child with non-zero ID
		for (const auto& child : pObj->children) {
			if (child.id) {
				hchild = GetDlgItem(hdlg, child.id);
				break;
			}
		}
		if (hchild) {
			GetWindowRect(hchild, &rc);
			ScreenToClient(hdlg, reinterpret_cast<POINT*>(&rc));
			ScreenToClient(hdlg, reinterpret_cast<POINT*>(&rc.right));
			RECT rcOrig = rc;
			MapDialogRect(hdlg, &rc);
			itemX = static_cast<double>(rc.right) / rcOrig.right;
			itemY = static_cast<double>(rc.bottom) / rcOrig.bottom;
		}
		DestroyWindow(hdlg);
	}

	pObj->pos  = { static_cast<LONG>(pTpl->x * dlgX), static_cast<LONG>(pTpl->y * dlgY) };
	pObj->size = { static_cast<LONG>(pTpl->cx * dlgX), static_cast<LONG>(pTpl->cy * dlgY) };

	for (auto& child : pObj->children) {
		child.pos  = { static_cast<LONG>(child.pos.x * itemX), static_cast<LONG>(child.pos.y * itemY) };
		child.size = { static_cast<LONG>(child.size.cx * itemX), static_cast<LONG>(child.size.cy * itemY) };
	}

	if (firstPass) {
		dlgX = dlgY = itemX = itemY = 0; // so they'll be calculated once again
		firstPass = false;
	}
}

WindowDialogTemplate::Item::Item(BYTE *pData, BYTE **pPast)
{
	BYTE *pRun = pData;
	DLGITEMTEMPLATEEX *pTemplate = reinterpret_cast<DLGITEMTEMPLATEEX*>(pRun);

	this->style   = pTemplate->style;
	this->exStyle = pTemplate->exStyle;
	this->pos     = { pTemplate->x, pTemplate->y };
	this->size    = { pTemplate->cx, pTemplate->cy };
	this->id      = pTemplate->id;

	pRun += sizeof(DLGITEMTEMPLATEEX);
	WORD *wPtr = reinterpret_cast<WORD*>(pRun);

	if (*wPtr == 0x0000) { // window class
		++wPtr;
	} else if (*wPtr == 0xFFFF) {
		++wPtr;
		switch (*wPtr) {
			case 0x0080: this->windowClass = L"Button"; break;
			case 0x0081: this->windowClass = L"Edit"; break;
			case 0x0082: this->windowClass = L"Static"; break;
			case 0x0083: this->windowClass = L"ListBox"; break;
			case 0x0084: this->windowClass = L"Scrollbar"; break;
			case 0x0085: this->windowClass = L"ComboBox"; break;
			default:     this->windowClass = L"";
		}
		++wPtr;
	} else {
		this->windowClass = reinterpret_cast<const wchar_t*>(wPtr);
		while ((*wPtr++) != 0x0000) ;
	}

	if (*wPtr == 0x0000) { // title
		++wPtr;
	} else {
		this->title = reinterpret_cast<const wchar_t*>(wPtr);
		while ((*wPtr++) != 0x0000) ;
	}

	WORD wCADataSize = 0; // creation array
	if (*wPtr == 0x0000) {
		wCADataSize = sizeof(WORD);
	} else {
		wCADataSize = *wPtr;
		CREATESTRUCT *lpCreateStruct = reinterpret_cast<CREATESTRUCT*>(wPtr);
	}

	UINT_PTR dwItemSize = reinterpret_cast<BYTE*>(wPtr) - reinterpret_cast<BYTE*>(pData) + wCADataSize;
	DWORD_ALIGN(dwItemSize);
	*pPast = pData + dwItemSize;
}

bool WindowDialogTemplate::load(HINSTANCE hInst, int dialogId, wstring *pErr)
{
	// http://www.codeguru.com/cpp/w-d/dislog/miscellaneous/article.php/c1943/Enumerating-Controls-of-a-Dialog-Resource-at-Runtime.htm
	HRSRC hrDlg = FindResource(hInst, MAKEINTRESOURCE(dialogId), RT_DIALOG);
	if (!hrDlg) {
		if (pErr) *pErr = Str::format(L"WindowDialogTemplate::load, FindResource() failed, error code %d.", GetLastError());
		return false;
	}

	HGLOBAL hgResource = LoadResource(hInst, hrDlg);
	if (!hgResource) {
		if (pErr) *pErr = Str::format(L"WindowDialogTemplate::load, LoadResource() failed, error code %d.", GetLastError());
		return false;
	}

	LPVOID lpResource = LockResource(hgResource);
	if (!lpResource) {
		if (pErr) *pErr = Str::format(L"WindowDialogTemplate::load, LockResource() failed, error code %d.", GetLastError());
		FreeResource(hgResource);
		return false;
	}

	BYTE *pGlobalData = reinterpret_cast<BYTE*>(lpResource);
	DLGTEMPLATEEX *pTemplate = reinterpret_cast<DLGTEMPLATEEX*>(pGlobalData);

	if (pTemplate->dlgVer != 0x0001) {
		if (pErr) *pErr = Str::format(L"WindowDialogTemplate::load, dialog version is not 0x0001.");
		return false;
	} else if (pTemplate->signature != 0xFFFF) {
		if (pErr) *pErr = Str::format(L"WindowDialogTemplate::load, dialog signature is not 0xFFFF.");
		return false;
	}

	this->style   = pTemplate->style;
	this->exStyle = pTemplate->exStyle;
	this->pos     = { pTemplate->x, pTemplate->y };
	this->size    = { pTemplate->cx, pTemplate->cy };

	pGlobalData += sizeof(DLGTEMPLATEEX);
	WORD *wPtr = reinterpret_cast<WORD*>(pGlobalData);

	if (*wPtr == 0x0000) { // menu
		++wPtr;
	} else if (*wPtr == 0xFFFF) {
		++wPtr;
		WORD menuOrd = *wPtr;
		++wPtr;
	} else {
		const wchar_t *strMenuRes = reinterpret_cast<const wchar_t*>(wPtr);
		while ((*wPtr++) != 0x0000) ;
	}

	/*if (*wPtr == 0x0000) { // window class
		++wPtr;
	} else if (*wPtr == 0xFFFF) {
		++wPtr;
		WORD wClassOrd = *wPtr;
		++wPtr;
	} else {
		const wchar_t *strClassRes = reinterpret_cast<const wchar_t*>(wPtr);
		while ((*wPtr++) != 0x0000) ;
	}*/

	if (*wPtr == 0x0000) { // title
		++wPtr;
	} else {
		this->title = reinterpret_cast<const wchar_t*>(wPtr);
		while ((*wPtr++) != 0x0000) ;
	}

	if ((pTemplate->style & DS_SETFONT) || (pTemplate->style & DS_SHELLFONT)) {
		this->fontPointSize = *wPtr;
		++wPtr;

		this->fontWeight = *wPtr;
		++wPtr;

		this->fontItalic = LOBYTE(*wPtr) != FALSE;
		this->fontCharset = HIBYTE(*wPtr);
		++wPtr;

		this->fontFace = reinterpret_cast<const wchar_t*>(wPtr);
		while ((*wPtr++) != 0x0000) ;
	}

	UINT_PTR dataPos = reinterpret_cast<UINT_PTR>(wPtr) - reinterpret_cast<UINT_PTR>(pGlobalData);
	DWORD_ALIGN(dataPos);
	pGlobalData += dataPos;

	//pGlobalData = reinterpret_cast<BYTE*>(wPtr);
	this->children.clear();
	for (WORD i = 0; i < pTemplate->cDlgItems; ++i) {
		this->children.emplace_back(Item(pGlobalData, &pGlobalData));
	}

	_dluToPx(hInst, pTemplate, this);
	UnlockResource(lpResource);
	FreeResource(hgResource);
	if (pErr) pErr->clear();
	return true;
}