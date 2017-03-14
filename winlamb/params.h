/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <CommCtrl.h>

namespace wl {

// Simple struct with message, WPARAM and LPARAM.
struct params {
	UINT   message;
	WPARAM wParam;
	LPARAM lParam;
};


#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp)) // from windowsx.h
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#define EMPTYWM(sname) \
	struct sname final : public params { \
		sname(const params& p) : params(p) { } \
	}


struct wm final {
	struct command final : public params {
		command(const params& p) : params(p) { }
		WORD control_id() const          { return LOWORD(this->wParam); }
		HWND control_hwnd() const        { return reinterpret_cast<HWND>(this->lParam); }
		bool is_from_menu() const        { return HIWORD(this->wParam) == 0; }
		bool is_from_accelerator() const { return HIWORD(this->wParam) == 1; }
	};
	struct activate final : public params {
		activate(const params& p) : params(p) { }
		bool is_being_activated() const           { return this->wParam != WA_INACTIVE; }
		bool activated_not_by_mouse_click() const { return this->wParam == WA_ACTIVE; }
		bool activated_by_mouse_click() const     { return this->wParam == WA_CLICKACTIVE; }
		HWND swapped_window() const               { return reinterpret_cast<HWND>(this->lParam); }
	};
	struct activateapp final : public params {
		activateapp(const params& p) : params(p) { }
		bool  is_being_activated() const { return this->wParam != FALSE; }
		DWORD thread_id() const          { return static_cast<DWORD>(this->lParam); }
	};
	struct askcbformatname final : public params {
		askcbformatname(const params& p) : params(p) { }
		UINT   szbuffer() const { return static_cast<UINT>(this->wParam); }
		TCHAR* buffer() const   { return reinterpret_cast<TCHAR*>(this->lParam); }
	};
	EMPTYWM(cancelmodel);
	struct capturechanged final : public params {
		capturechanged(const params& p) : params(p) { }
		HWND window_gaining_mouse() const { return reinterpret_cast<HWND>(this->lParam); }
	};
	struct changecbchain final : public params {
		changecbchain(const params& p) : params(p) { }
		HWND window_being_removed() const { return reinterpret_cast<HWND>(this->wParam); }
		HWND next_window() const          { return reinterpret_cast<HWND>(this->lParam); }
		bool is_last_window() const       { return this->next_window() == nullptr; }
	};
	struct char_ final : public params {
		char_(const params& p) : params(p) { }
		WORD char_code() const           { return static_cast<WORD>(this->wParam); }
		WORD repeat_count() const        { return LOWORD(this->lParam); }
		BYTE scan_code() const           { return LOBYTE(HIWORD(this->lParam)); }
		bool is_extended_key() const     { return (this->lParam >> 24) & 1; }
		bool has_alt_key() const         { return (this->lParam >> 29) & 1; }
		bool key_previously_down() const { return (this->lParam >> 30) & 1; }
		bool key_being_released() const  { return (this->lParam >> 31) & 1; }
	};
	struct chartoitem final : public params {
		chartoitem(const params& p) : params(p) { }
		WORD char_code() const         { return LOWORD(this->wParam); }
		WORD current_caret_pos() const { return HIWORD(this->wParam); }
		HWND hlistbox() const          { return reinterpret_cast<HWND>(this->lParam); }
	};
	EMPTYWM(childactivate);
	EMPTYWM(close);
	struct compacting final : public params {
		compacting(const params& p) : params(p) { }
		UINT cpu_time_ratio() const { return static_cast<UINT>(this->wParam); }
	};
	struct compareitem final : public params {
		compareitem(const params& p) : params(p) { }
		WORD               control_id() const        { return static_cast<WORD>(this->wParam); }
		COMPAREITEMSTRUCT& compareitemstruct() const { return *reinterpret_cast<COMPAREITEMSTRUCT*>(this->lParam); }
	};
	struct contextmenu final : public params {
		contextmenu(const params& p) : params(p) { }
		HWND  target() const { return reinterpret_cast<HWND>(this->wParam); }
		POINT pos() const    { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
	};
	struct copydata final : public params {
		copydata(const params& p) : params(p) { }
		COPYDATASTRUCT& copydatastruct() const { return *reinterpret_cast<COPYDATASTRUCT*>(this->lParam); }
	};
	struct create final : public params {
		create(const params& p) : params(p) { }
		CREATESTRUCT& createstruct() const { return *reinterpret_cast<CREATESTRUCT*>(this->lParam); }
	};

	struct ctlcolorbtn : public params {
		ctlcolorbtn(const params& p) : params(p) { }
		HDC  hdc() const  { return reinterpret_cast<HDC>(this->wParam); }
		HWND hctl() const { return reinterpret_cast<HWND>(this->lParam); }
	};
	struct ctlcolordlg final       : public ctlcolorbtn { ctlcolordlg(const params& p)       : ctlcolorbtn(p) { } };
	struct ctlcoloredit final      : public ctlcolorbtn { ctlcoloredit(const params& p)      : ctlcolorbtn(p) { } };
	struct ctlcolorlistbox final   : public ctlcolorbtn { ctlcolorlistbox(const params& p)   : ctlcolorbtn(p) { } };
	struct ctlcolorscrollbar final : public ctlcolorbtn { ctlcolorscrollbar(const params& p) : ctlcolorbtn(p) { } };
	struct ctlcolorstatic final    : public ctlcolorbtn { ctlcolorstatic(const params& p)    : ctlcolorbtn(p) { } };

	struct deadchar final : public params {
		deadchar(const params& p) : params(p) { }
		WORD char_code() const           { return static_cast<WORD>(this->wParam); }
		WORD repeat_count() const        { return LOWORD(this->lParam); }
		BYTE scan_code() const           { return LOBYTE(HIWORD(this->lParam)); }
		bool is_extended_key() const     { return (this->lParam >> 24) & 1; }
		bool has_alt_key() const         { return (this->lParam >> 29) & 1; }
		bool key_previously_down() const { return (this->lParam >> 30) & 1; }
		bool key_being_released() const  { return (this->lParam >> 31) & 1; }
	};
	struct deleteitem final : public params {
		deleteitem(const params& p) : params(p) { }
		WORD              control_id() const       { return static_cast<WORD>(this->wParam); }
		DELETEITEMSTRUCT& deleteitemstruct() const { return *reinterpret_cast<DELETEITEMSTRUCT*>(this->lParam); }
	};
	EMPTYWM(destroy);
	EMPTYWM(destroyclipboard);
	struct devmodechange final : public params {
		devmodechange(const params& p) : params(p) { }
		const TCHAR* device_name() const { return reinterpret_cast<const TCHAR*>(this->lParam); }
	};

#ifdef _DBT_H // Ras.h
	struct devicechange final : public params {
		devicechange(const params& p) : params(p) { }
		UINT                           event() const                         { return static_cast<UINT>(this->wParam); }
		DEV_BROADCAST_HDR&             dev_broadcast_hdr() const             { return *reinterpret_cast<DEV_BROADCAST_HDR*>(this->lParam); }
		DEV_BROADCAST_DEVICEINTERFACE& dev_broadcast_deviceinterface() const { return *reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(this->lParam); }
		DEV_BROADCAST_HANDLE&          dev_broadcast_handle() const          { return *reinterpret_cast<DEV_BROADCAST_HANDLE*>(this->lParam); }
		DEV_BROADCAST_OEM&             dev_broadcast_oem() const             { return *reinterpret_cast<DEV_BROADCAST_OEM*>(this->lParam); }
		DEV_BROADCAST_PORT&            dev_broadcast_port() const            { return *reinterpret_cast<DEV_BROADCAST_PORT*>(this->lParam); }
		DEV_BROADCAST_VOLUME&          dev_broadcast_volume() const          { return *reinterpret_cast<DEV_BROADCAST_VOLUME*>(this->lParam); }
	};
#endif

	struct displaychange final : public params {
		displaychange(const params& p) : params(p) { }
		UINT bits_per_pixel() const { return static_cast<UINT>(this->wParam); }
		SIZE sz() const             { return { LOWORD(this->lParam), HIWORD(this->lParam) }; }
	};
	EMPTYWM(drawclipboard);
	struct drawitem final : public params {
		drawitem(const params& p) : params(p) { }
		WORD            control_id() const     { return static_cast<WORD>(this->wParam); }
		bool            is_from_menu() const   { return this->control_id() == 0; }
		DRAWITEMSTRUCT& drawitemstruct() const { return *reinterpret_cast<DRAWITEMSTRUCT*>(this->lParam); }
	};
	struct dropfiles final : public params {
		dropfiles(const params& p) : params(p) { }
		HDROP hdrop() const { return reinterpret_cast<HDROP>(this->wParam); }
		UINT  count() const { return DragQueryFileW(this->hdrop(), 0xFFFFFFFF, nullptr, 0); }
		std::vector<std::wstring> files() const {
			std::vector<std::wstring> files(this->count()); // alloc return vector
			for (size_t i = 0; i < files.size(); ++i) {
				files[i].resize(DragQueryFileW(this->hdrop(),
					static_cast<UINT>(i), nullptr, 0) + 1, L'\0'); // alloc path string
				DragQueryFileW(this->hdrop(), static_cast<UINT>(i), &files[i][0],
					static_cast<UINT>(files[i].size()));
				files[i].resize(files[i].size() - 1); // trim null
			}
			DragFinish(this->hdrop()); // files are not necessarily sorted
			return files;
		}
		POINT pos() const {
			POINT pt = { 0 };
			DragQueryPoint(this->hdrop(), &pt);
			return pt;
		}
	};
	struct enable final : public params {
		enable(const params& p) : params(p) { }
		bool has_been_enabled() const { return this->wParam != FALSE; }
	};
	struct endsession final : public params {
		endsession(const params& p) : params(p) { }
		bool is_session_being_ended() const { return this->wParam != FALSE; }
		bool is_system_issue() const        { return (this->lParam & ENDSESSION_CLOSEAPP) != 0; }
		bool is_forced_critical() const     { return (this->lParam & ENDSESSION_CRITICAL) != 0; }
		bool is_logoff() const              { return (this->lParam & ENDSESSION_LOGOFF) != 0; }
		bool is_shutdown() const            { return this->lParam == 0; }
	};
	struct enteridle final : public params {
		enteridle(const params& p) : params(p) { }
		bool is_menu_displayed() const { return this->wParam == MSGF_MENU; }
		HWND hwindow() const           { return reinterpret_cast<HWND>(this->lParam); }
	};
	struct entermenuloop final : public params {
		entermenuloop(const params& p) : params(p) { }
		bool uses_trackpopupmenu() const { return this->wParam != FALSE; }
	};
	EMPTYWM(entersizemove);
	struct erasebkgnd final : public params {
		erasebkgnd(const params& p) : params(p) { }
		HDC hdc() const { return reinterpret_cast<HDC>(this->wParam); }
	};
	struct exitmenuloop final : public params {
		exitmenuloop(const params& p) : params(p) { }
		bool is_shortcut_menu() const { return this->wParam != FALSE; }
	};
	EMPTYWM(exitsizemove);
	EMPTYWM(fontchange);
	struct getdlgcode final : public params {
		getdlgcode(const params& p) : params(p) { }
		BYTE vkey_code() const { return static_cast<BYTE>(this->wParam); }
		MSG& msg() const       { return *reinterpret_cast<MSG*>(this->lParam); }
	};
	EMPTYWM(getfont);
	EMPTYWM(gethotkey);
	struct geticon final : public params {
		geticon(const params& p) : params(p) { }
		bool is_big() const       { return this->wParam == ICON_BIG; }
		bool is_small() const     { return this->wParam == ICON_SMALL; }
		bool is_small_app() const { return this->wParam == ICON_SMALL2; }
		UINT dpi() const          { return static_cast<UINT>(this->lParam); }
	};
	struct getminmaxinfo final : public params {
		getminmaxinfo(const params& p) : params(p) { }
		MINMAXINFO& minmaxinfo() const { return *reinterpret_cast<MINMAXINFO*>(this->lParam); }
	};
	struct gettext final : public params {
		gettext(const params& p) : params(p) { }
		UINT   buffer_size() const { return static_cast<UINT>(this->wParam); }
		TCHAR* buffer() const      { return reinterpret_cast<TCHAR*>(this->lParam); }
	};
	EMPTYWM(gettextlength);
	struct help final : public params {
		help(const params& p) : params(p) { }
		HELPINFO& helpinfo() const { return *reinterpret_cast<HELPINFO*>(this->lParam); }
	};
	struct hotkey final : public params {
		hotkey(const params& p) : params(p) { }
		bool is_snap_desktop() const { return this->wParam == IDHOT_SNAPDESKTOP; }
		bool is_snap_window() const  { return this->wParam == IDHOT_SNAPWINDOW; }
		bool has_alt() const         { return (LOWORD(this->lParam) & MOD_ALT) != 0; }
		bool has_ctrl() const        { return (LOWORD(this->lParam) & MOD_CONTROL) != 0; }
		bool has_shift() const       { return (LOWORD(this->lParam) & MOD_SHIFT) != 0; }
		bool has_win() const         { return (LOWORD(this->lParam) & MOD_WIN) != 0; }
		BYTE vkey_code() const       { return static_cast<BYTE>(HIWORD(this->lParam)); }
	};

	struct hscroll : public params {
		hscroll(const params& p) : params(p) { }
		WORD scroll_request() const { return LOWORD(this->wParam); }
		WORD scroll_pos() const     { return HIWORD(this->wParam); }
		HWND scrollbar() const      { return reinterpret_cast<HWND>(this->lParam); }
	};
	struct vscroll final : public hscroll { vscroll(const params& p) : hscroll(p) { } };

	struct hscrollclipboard : public params {
		hscrollclipboard(const params& p) : params(p) { }
		HWND clipboard_viewer() const { return reinterpret_cast<HWND>(this->wParam); }
		WORD scroll_event() const     { return LOWORD(this->lParam); }
		WORD scroll_pos() const       { return HIWORD(this->lParam); }
	};
	struct vscrollclipboard final : public hscrollclipboard { vscrollclipboard(const params& p) : hscrollclipboard(p) { } };

	struct iconerasebkgnd final : public params {
		iconerasebkgnd(const params& p) : params(p) { }
		HDC hdc() const { return reinterpret_cast<HDC>(this->wParam); }
	};
	struct initdialog final : public params {
		initdialog(const params& p) : params(p) { }
		HWND focused_ctrl() const { return reinterpret_cast<HWND>(this->wParam); }
	};
	struct initmenu final : public params {
		initmenu(const params& p) : params(p) { }
		HMENU hmenu() const { return reinterpret_cast<HMENU>(this->wParam); }
	};
	struct initmenupopup final : public params {
		initmenupopup(const params& p) : params(p) { }
		HMENU hmenu() const          { return reinterpret_cast<HMENU>(this->wParam); }
		short relative_pos() const   { return LOWORD(this->lParam); }
		bool  is_window_menu() const { return HIWORD(this->lParam) != FALSE; }
	};
	struct inputlangchange final : public params {
		inputlangchange(const params& p) : params(p) { }
		DWORD new_charset() const     { return static_cast<DWORD>(this->wParam); }
		HKL   keyboard_layout() const { return reinterpret_cast<HKL>(this->lParam); }
	};
	struct inputlangchangerequest final : public params {
		inputlangchangerequest(const params& p) : params(p) { }
		bool previous_chosen() const      { return (this->wParam & INPUTLANGCHANGE_BACKWARD) != 0; }
		bool next_chosen() const          { return (this->wParam & INPUTLANGCHANGE_FORWARD) != 0; }
		bool can_be_used_with_sys() const { return (this->wParam & INPUTLANGCHANGE_SYSCHARSET) != 0; }
		HKL  keyboard_layout() const      { return reinterpret_cast<HKL>(this->lParam); }
	};
	struct keydown final : public params {
		keydown(const params& p) : params(p) { }
		BYTE vkey_code() const           { return static_cast<BYTE>(this->wParam); }
		WORD repeat_count() const        { return LOWORD(this->lParam); }
		BYTE scan_code() const           { return LOBYTE(HIWORD(this->lParam)); }
		bool is_extended_key() const     { return (this->lParam >> 24) & 1; }
		bool key_previously_down() const { return (this->lParam >> 30) & 1; }
	};
	struct keyup final : public params {
		keyup(const params& p) : params(p) { }
		BYTE vkey_code() const       { return static_cast<BYTE>(this->wParam); }
		BYTE scan_code() const       { return LOBYTE(HIWORD(this->lParam)); }
		bool is_extended_key() const { return (this->lParam >> 24) & 1; }
	};
	struct killfocus final : public params {
		killfocus(const params& p) : params(p) { }
		HWND focused_window() const { return reinterpret_cast<HWND>(this->wParam); }
	};

	struct lbuttondblclk : public params {
		lbuttondblclk(const params& p) : params(p) { }
		bool  has_ctrl() const       { return (this->wParam & MK_CONTROL) != 0; }
		bool  has_left_btn() const   { return (this->wParam & MK_LBUTTON) != 0; }
		bool  has_middle_btn() const { return (this->wParam & MK_MBUTTON) != 0; }
		bool  has_right_btn() const  { return (this->wParam & MK_RBUTTON) != 0; }
		bool  has_shift() const      { return (this->wParam & MK_SHIFT) != 0; }
		bool  has_xbtn1() const      { return (this->wParam & MK_XBUTTON1) != 0; }
		bool  has_xbtn2() const      { return (this->wParam & MK_XBUTTON2) != 0; }
		POINT pos() const            { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
	};
	struct lbuttondown final   : public lbuttondblclk { lbuttondown(const params& p)   : lbuttondblclk(p) { } };
	struct lbuttonup final     : public lbuttondblclk { lbuttonup(const params& p)     : lbuttondblclk(p) { } };
	struct mbuttondblclk final : public lbuttondblclk { mbuttondblclk(const params& p) : lbuttondblclk(p) { } };
	struct mbuttondown final   : public lbuttondblclk { mbuttondown(const params& p)   : lbuttondblclk(p) { } };
	struct mbuttonup final     : public lbuttondblclk { mbuttonup(const params& p)     : lbuttondblclk(p) { } };
	struct mousehover final    : public lbuttondblclk { mousehover(const params& p)    : lbuttondblclk(p) { } };
	struct mousemove final     : public lbuttondblclk { mousemove(const params& p)     : lbuttondblclk(p) { } };
	struct rbuttondblclk final : public lbuttondblclk { rbuttondblclk(const params& p) : lbuttondblclk(p) { } };
	struct rbuttondown final   : public lbuttondblclk { rbuttondown(const params& p)   : lbuttondblclk(p) { } };
	struct rbuttonup final     : public lbuttondblclk { rbuttonup(const params& p)     : lbuttondblclk(p) { } };

	struct mdiactivate final : public params {
		mdiactivate(const params& p) : params(p) { }
		HWND activated_child() const   { return reinterpret_cast<HWND>(this->wParam); }
		HWND deactivated_child() const { return reinterpret_cast<HWND>(this->lParam); }
	};
	struct measureitem final : public params {
		measureitem(const params& p) : params(p) { }
		MEASUREITEMSTRUCT& measureitemstruct() const { return *reinterpret_cast<MEASUREITEMSTRUCT*>(this->lParam); }
	};
	struct menuchar final : public params {
		menuchar(const params& p) : params(p) { }
		WORD  char_code() const      { return LOWORD(this->wParam); }
		bool  is_window_menu() const { return HIWORD(this->wParam) == MF_SYSMENU; }
		HMENU hmenu() const          { return reinterpret_cast<HMENU>(this->lParam); }
	};
	struct menudrag final : public params {
		menudrag(const params& p) : params(p) { }
		UINT  initial_pos() const { static_cast<UINT>(this->wParam); }
		HMENU hmenu() const       { return reinterpret_cast<HMENU>(this->lParam); }
	};
	struct menugetobject final : public params {
		menugetobject(const params& p) : params(p) { }
		MENUGETOBJECTINFO& menugetobjectinfo() const { return *reinterpret_cast<MENUGETOBJECTINFO*>(this->lParam); }
	};
	struct menurbuttonup final : public params {
		menurbuttonup(const params& p) : params(p) { }
		UINT  index() const { return static_cast<UINT>(this->wParam); }
		HMENU hmenu() const { return reinterpret_cast<HMENU>(this->lParam); }
	};
	struct menuselect final : public params {
		menuselect(const params& p) : params(p) { }
		WORD  item() const              { return LOWORD(this->wParam); }
		bool  has_bitmap() const        { return (HIWORD(this->wParam) & MF_BITMAP) != 0; }
		bool  is_checked() const        { return (HIWORD(this->wParam) & MF_CHECKED) != 0; }
		bool  is_disabled() const       { return (HIWORD(this->wParam) & MF_DISABLED) != 0; }
		bool  is_grayed() const         { return (HIWORD(this->wParam) & MF_GRAYED) != 0; }
		bool  is_highlighted() const    { return (HIWORD(this->wParam) & MF_HILITE) != 0; }
		bool  mouse_selected() const    { return (HIWORD(this->wParam) & MF_MOUSESELECT) != 0; }
		bool  is_owner_draw() const     { return (HIWORD(this->wParam) & MF_OWNERDRAW) != 0; }
		bool  opens_popup() const       { return (HIWORD(this->wParam) & MF_POPUP) != 0; }
		bool  is_sysmenu() const        { return (HIWORD(this->wParam) & MF_SYSMENU) != 0; }
		bool  system_has_closed() const { return HIWORD(this->wParam) == 0xFFFF && !this->lParam; }
		HMENU hmenu() const             { return (this->opens_popup() || this->is_sysmenu()) ? reinterpret_cast<HMENU>(this->lParam) : nullptr; }
	};
	struct mouseactivate final : public params {
		mouseactivate(const params& p) : params(p) { }
		short hit_test_code() const { return static_cast<short>(LOWORD(this->lParam)); }
		WORD  mouse_msg_id() const  { return HIWORD(this->lParam); }
	};
	EMPTYWM(mouseleave);
	struct mousewheel final : public params {
		mousewheel(const params& p) : params(p) { }
		short wheel_delta() const    { return GET_WHEEL_DELTA_WPARAM(this->wParam); }
		bool  has_ctrl() const       { return (LOWORD(this->wParam) & MK_CONTROL) != 0; }
		bool  has_left_btn() const   { return (LOWORD(this->wParam) & MK_LBUTTON) != 0; }
		bool  has_middle_btn() const { return (LOWORD(this->wParam) & MK_MBUTTON) != 0; }
		bool  has_right_btn() const  { return (LOWORD(this->wParam) & MK_RBUTTON) != 0; }
		bool  has_shift() const      { return (LOWORD(this->wParam) & MK_SHIFT) != 0; }
		bool  has_xbtn1() const      { return (LOWORD(this->wParam) & MK_XBUTTON1) != 0; }
		bool  has_xbtn2() const      { return (LOWORD(this->wParam) & MK_XBUTTON2) != 0; }
		POINT pos() const            { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
	};
	struct move final : public params {
		move(const params& p) : params(p) { }
		POINT pos() const { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
	};
	struct moving final : public params {
		moving(const params& p) : params(p) { }
		RECT& screen_coords() const { return *reinterpret_cast<RECT*>(this->lParam); }
	};
	struct ncactivate final : public params {
		ncactivate(const params& p) : params(p) { }
		bool is_active() const { return this->wParam == TRUE; }
	};
	struct nccalcsize final : public params {
		nccalcsize(const params& p) : params(p) { }
		bool               is_nccalcsize() const     { return this->wParam == TRUE; }
		bool               is_rect() const           { return this->wParam == FALSE; }
		NCCALCSIZE_PARAMS& nccalcsize_params() const { return *reinterpret_cast<NCCALCSIZE_PARAMS*>(this->lParam); }
		RECT&              rect() const              { return *reinterpret_cast<RECT*>(this->lParam); }
	};
	struct nccreate final : public params {
		nccreate(const params& p) : params(p) { }
		CREATESTRUCT& createstruct() const { return *reinterpret_cast<CREATESTRUCT*>(this->lParam); }
	};
	EMPTYWM(ncdestroy);
	struct nchittest final : public params {
		nchittest(const params& p) : params(p) { }
		POINT pos() const { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
	};

	struct nclbuttondblclk : public params {
		nclbuttondblclk(const params& p) : params(p) { }
		short hit_test_code() const { return static_cast<short>(this->wParam); }
		POINT pos() const           { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
	};
	struct nclbuttondown final   : public nclbuttondblclk { nclbuttondown(const params& p)   : nclbuttondblclk(p) { } };
	struct nclbuttonup final     : public nclbuttondblclk { nclbuttonup(const params& p)     : nclbuttondblclk(p) { } };
	struct ncmbuttondblclk final : public nclbuttondblclk { ncmbuttondblclk(const params& p) : nclbuttondblclk(p) { } };
	struct ncmbuttondown final   : public nclbuttondblclk { ncmbuttondown(const params& p)   : nclbuttondblclk(p) { } };
	struct ncmbuttonup final     : public nclbuttondblclk { ncmbuttonup(const params& p)     : nclbuttondblclk(p) { } };
	struct ncmousemove final     : public nclbuttondblclk { ncmousemove(const params& p)     : nclbuttondblclk(p) { } };
	struct ncrbuttondblclk final : public nclbuttondblclk { ncrbuttondblclk(const params& p) : nclbuttondblclk(p) { } };
	struct ncrbuttondown final   : public nclbuttondblclk { ncrbuttondown(const params& p)   : nclbuttondblclk(p) { } };
	struct ncrbuttonup final     : public nclbuttondblclk { ncrbuttonup(const params& p)     : nclbuttondblclk(p) { } };

	struct ncpaint final : public params {
		ncpaint(const params& p) : params(p) { }
		HRGN hrgn() const { return reinterpret_cast<HRGN>(this->wParam); }
	};
	struct nextdlgctl final : public params {
		nextdlgctl(const params& p) : params(p) { }
		bool has_ctrl_receiving_focus() const { return LOWORD(this->lParam) != FALSE; }
		HWND ctrl_receiving_focus() const     { return LOWORD(this->lParam) ? reinterpret_cast<HWND>(this->wParam) : nullptr; }
		bool focus_next() const               { return this->wParam == 0; }
	};
	struct nextmenu final : public params {
		nextmenu(const params& p) : params(p) { }
		BYTE         vkey_code() const   { return static_cast<BYTE>(this->wParam); }
		MDINEXTMENU& mdinextmenu() const { return *reinterpret_cast<MDINEXTMENU*>(this->lParam); }
	};
	struct notifyformat final : public params {
		notifyformat(const params& p) : params(p) { }
		HWND window_from() const           { return reinterpret_cast<HWND>(this->wParam); }
		bool is_query_from_control() const { return this->lParam == NF_QUERY; }
		bool is_requery_to_control() const { return this->lParam == NF_REQUERY; }
	};
	EMPTYWM(paint);
	struct paintclipboard final : public params {
		paintclipboard(const params& p) : params(p) { }
		HWND               clipboard_viewer() const { return reinterpret_cast<HWND>(this->wParam); }
		const PAINTSTRUCT& paintstruct() const      { return *reinterpret_cast<const PAINTSTRUCT*>(this->lParam); }
	};
	struct palettechanged final : public params {
		palettechanged(const params& p) : params(p) { }
		HWND window_origin() const { return reinterpret_cast<HWND>(this->wParam); }
	};
	struct paletteischanging final : public params {
		paletteischanging(const params& p) : params(p) { }
		HWND window_origin() const { return reinterpret_cast<HWND>(this->wParam); }
	};
	struct parentnotify final : public params {
		parentnotify(const params& p) : params(p) { }
		UINT  event_message() const { return static_cast<UINT>(LOWORD(this->wParam)); }
		WORD  child_id() const      { return HIWORD(this->wParam); }
		HWND  child_hwnd() const    { return reinterpret_cast<HWND>(this->lParam); }
		POINT pos() const           { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
		bool  is_xbutton1() const   { return HIWORD(this->wParam) == XBUTTON1; }
		WORD  pointer_flag() const  { return HIWORD(this->wParam); }
	};
	struct powerbroadcast final : public params {
		powerbroadcast(const params& p) : params(p) { }
		bool                    is_power_status_change() const  { return this->wParam == PBT_APMPOWERSTATUSCHANGE; }
		bool                    is_resuming() const             { return this->wParam == PBT_APMRESUMEAUTOMATIC; }
		bool                    is_suspending() const           { return this->wParam == PBT_APMSUSPEND; }
		bool                    is_power_setting_change() const { return this->wParam == PBT_POWERSETTINGCHANGE; }
		POWERBROADCAST_SETTING& power_setting() const           { return *reinterpret_cast<POWERBROADCAST_SETTING*>(this->lParam); }
	};
	struct print final : public params {
		print(const params& p) : params(p) { }
		HDC  hdc() const   { return reinterpret_cast<HDC>(this->wParam); }
		UINT flags() const { return static_cast<UINT>(this->lParam); }
	};
	struct printclient final : public params {
		printclient(const params& p) : params(p) { }
		HDC  hdc() const   { return reinterpret_cast<HDC>(this->wParam); }
		UINT flags() const { return static_cast<UINT>(this->lParam); }
	};
	EMPTYWM(querydragicon);
	struct queryendsession final : public params {
		queryendsession(const params& p) : params(p) { }
		bool is_system_issue() const    { return (this->lParam & ENDSESSION_CLOSEAPP) != 0; }
		bool is_forced_critical() const { return (this->lParam & ENDSESSION_CRITICAL) != 0; }
		bool is_logoff() const          { return (this->lParam & ENDSESSION_LOGOFF) != 0; }
		bool is_shutdown() const        { return this->lParam == 0; }
	};
	EMPTYWM(querynewpalette);
	EMPTYWM(queryopen);

#ifdef _RAS_H_ // Ras.h
	struct rasdialevent final : public params {
		rasdialevent(const params& p) : params(p) { }
		RASCONNSTATE rasconnstate() const { return static_cast<RASCONNSTATE>(this->wParam); }
		DWORD        error() const        { return static_cast<DWORD>(this->lParam); }
	};
#endif

	EMPTYWM(renderallformats);
	struct renderformat final : public params {
		renderformat(const params& p) : params(p) { }
		WORD clipboard_format() const { return static_cast<WORD>(this->wParam); }
	};
	struct setcursor final : public params {
		setcursor(const params& p) : params(p) { }
		HWND  cursor_owner() const  { return reinterpret_cast<HWND>(this->wParam); }
		short hit_test_code() const { return static_cast<short>(LOWORD(this->wParam)); }
		WORD  mouse_msg_id() const  { return HIWORD(this->wParam); }
	};
	struct setfocus final : public params {
		setfocus(const params& p) : params(p) { }
		HWND unfocused_window() const { return reinterpret_cast<HWND>(this->wParam); }
	};
	struct setfont final : public params {
		setfont(const params& p) : params(p) { }
		HFONT hfont() const         { return reinterpret_cast<HFONT>(this->wParam); }
		bool  should_redraw() const { return LOWORD(this->lParam) != FALSE; }
	};
	struct sethotkey final : public params {
		sethotkey(const params& p) : params(p) { }
		BYTE vkey_code() const    { return static_cast<BYTE>(LOWORD(this->wParam)); }
		bool has_alt() const      { return (HIWORD(this->wParam) & 0x04) != 0; }
		bool has_ctrl() const     { return (HIWORD(this->wParam) & 0x02) != 0; }
		bool has_extended() const { return (HIWORD(this->wParam) & 0x08) != 0; }
		bool has_shift() const    { return (HIWORD(this->wParam) & 0x01) != 0; }
	};
	struct seticon final : public params {
		seticon(const params& p) : params(p) { }
		bool  is_small() const   { return this->wParam == ICON_SMALL; }
		HICON hicon() const      { return reinterpret_cast<HICON>(this->lParam); }
		bool  is_removed() const { return this->hicon() == nullptr; }
	};
	struct setredraw final : public params {
		setredraw(const params& p) : params(p) { }
		bool can_redraw() const { return this->wParam != FALSE; }
	};
	struct settext final : public params {
		settext(const params& p) : params(p) { }
		const TCHAR* text() const { return reinterpret_cast<const TCHAR*>(this->lParam); }
	};
	struct settingchange final : public params {
		settingchange(const params& p) : params(p) { }
		const TCHAR* string_id() const           { return reinterpret_cast<const TCHAR*>(this->lParam); }
		bool         is_policy() const           { return !lstrcmp(this->string_id(), TEXT("Policy")); }
		bool         is_locale() const           { return !lstrcmp(this->string_id(), TEXT("intl")); }
		bool         is_environment_vars() const { return !lstrcmp(this->string_id(), TEXT("Environment")); }
	};
	struct showwindow final : public params {
		showwindow(const params& p) : params(p) { }
		bool is_being_shown() const           { return this->wParam != FALSE; }
		bool is_other_away() const            { return this->lParam == SW_OTHERUNZOOM; }
		bool is_other_over() const            { return this->lParam == SW_OTHERZOOM; }
		bool is_owner_being_minimized() const { return this->lParam == SW_PARENTCLOSING; }
		bool is_owner_being_restored() const  { return this->lParam == SW_PARENTOPENING; }
	};
	struct size final : public params {
		size(const params& p) : params(p) { }
		bool is_other_maximized() const { return this->wParam == 4; }
		bool is_maximized() const       { return this->wParam == 2; }
		bool is_other_restored() const  { return this->wParam == 3; }
		bool is_minimized() const       { return this->wParam == 1; }
		bool is_restored() const        { return this->wParam == 0; }
		SIZE sz() const                 { return { LOWORD(this->lParam), HIWORD(this->lParam) }; }
	};
	struct sizeclipboard final : public params {
		sizeclipboard(const params& p) : params(p) { }
		HWND        clipboard_viewer() const { return reinterpret_cast<HWND>(this->wParam); }
		const RECT& clipboard_rect() const   { return *reinterpret_cast<const RECT*>(this->lParam); }
	};
	struct sizing final : public params {
		sizing(const params& p) : params(p) { }
		WORD  edge() const          { return static_cast<WORD>(this->wParam); }
		RECT& screen_coords() const { *reinterpret_cast<RECT*>(this->lParam); }
	};
	struct spoolerstatus final : public params {
		spoolerstatus(const params& p) : params(p) { }
		UINT status_flag() const    { return static_cast<UINT>(this->wParam); }
		WORD remaining_jobs() const { return LOWORD(this->lParam); }
	};
	struct stylechanged final : public params {
		stylechanged(const params& p) : params(p) { }
		bool               is_style() const    { return (this->wParam & GWL_STYLE) != 0; }
		bool               is_ex_style() const { return (this->wParam & GWL_EXSTYLE) != 0; }
		const STYLESTRUCT& stylestruct() const { return *reinterpret_cast<const STYLESTRUCT*>(this->lParam); }
	};
	struct stylechanging final : public params {
		stylechanging(const params& p) : params(p) { }
		bool               is_style() const    { return (this->wParam & GWL_STYLE) != 0; }
		bool               is_ex_style() const { return (this->wParam & GWL_EXSTYLE) != 0; }
		const STYLESTRUCT& stylestruct() const { return *reinterpret_cast<const STYLESTRUCT*>(this->lParam); }
	};
	struct syschar final : public params {
		syschar(const params& p) : params(p) { }
		WORD char_code() const           { return static_cast<WORD>(this->wParam); }
		WORD repeat_count() const        { return LOWORD(this->lParam); }
		BYTE scan_code() const           { return LOBYTE(HIWORD(this->lParam)); }
		bool is_extended_key() const     { return (this->lParam >> 24) & 1; }
		bool has_alt_key() const         { return (this->lParam >> 29) & 1; }
		bool key_previously_down() const { return (this->lParam >> 30) & 1; }
		bool key_being_released() const  { return (this->lParam >> 31) & 1; }
	};
	struct syscommand final : public params {
		syscommand(const params& p) : params(p) { }
		WORD  command_type() const { return static_cast<WORD>(this->wParam); }
		POINT pos() const          { return { GET_X_LPARAM(this->lParam), GET_Y_LPARAM(this->lParam) }; }
	};
	struct sysdeadchar final : public params {
		sysdeadchar(const params& p) : params(p) { }
		WORD char_code() const           { return static_cast<WORD>(this->wParam); }
		WORD repeat_count() const        { return LOWORD(this->lParam); }
		BYTE scan_code() const           { return LOBYTE(HIWORD(this->lParam)); }
		bool is_extended_key() const     { return (this->lParam >> 24) & 1; }
		bool has_alt_key() const         { return (this->lParam >> 29) & 1; }
		bool key_previously_down() const { return (this->lParam >> 30) & 1; }
		bool key_being_released() const  { return (this->lParam >> 31) & 1; }
	};
	struct syskeydown final : public params {
		syskeydown(const params& p) : params(p) { }
		BYTE vkey_code() const           { return static_cast<BYTE>(this->wParam); }
		WORD repeat_count() const        { return LOWORD(this->lParam); }
		BYTE scan_code() const           { return LOBYTE(HIWORD(this->lParam)); }
		bool is_extended_key() const     { return (this->lParam >> 24) & 1; }
		bool has_alt_key() const         { return (this->lParam >> 29) & 1; }
		bool key_previously_down() const { return (this->lParam >> 30) & 1; }
	};
	struct syskeyup final : public params {
		syskeyup(const params& p) : params(p) { }
		BYTE vkey_code() const    { return static_cast<BYTE>(this->wParam); }
		WORD repeat_count() const { return LOWORD(this->lParam); }
		BYTE scan_code() const    { return LOBYTE(HIWORD(this->lParam)); }
		bool has_alt_key() const  { return (this->lParam >> 29) & 1; }
	};
	struct tcard final : public params {
		tcard(const params& p) : params(p) { }
		UINT action_id() const   { return static_cast<UINT>(this->wParam); }
		long action_data() const { return static_cast<long>(this->lParam); }
	};
	EMPTYWM(timechange);
	struct timer final : public params {
		timer(const params& p) : params(p) { }
		UINT_PTR  timer_id() const { return static_cast<UINT_PTR>(this->wParam); }
		TIMERPROC callback() const { return reinterpret_cast<TIMERPROC>(this->lParam); }
	};
	struct uninitmenupopup final : public params {
		uninitmenupopup(const params& p) : params(p) { }
		HMENU hmenu() const   { return reinterpret_cast<HMENU>(this->wParam); }
		WORD  menu_id() const { return HIWORD(this->lParam); }
	};
	EMPTYWM(userchanged);
	struct vkeytoitem final : public params {
		vkeytoitem(const params& p) : params(p) { }
		BYTE vkey_code() const         { return static_cast<BYTE>(LOWORD(this->wParam)); }
		WORD current_caret_pos() const { return HIWORD(this->wParam); }
		HWND hlistbox() const          { return reinterpret_cast<HWND>(this->lParam); }
	};
	struct windowposchanged final : public params {
		windowposchanged(const params& p) : params(p) { }
		WINDOWPOS& windowpos() const { return *reinterpret_cast<WINDOWPOS*>(this->lParam); }
	};
	struct windowposchanging final : public params {
		windowposchanging(const params& p) : params(p) { }
		WINDOWPOS& windowpos() const { return *reinterpret_cast<WINDOWPOS*>(this->lParam); }
	};
protected:
	wm() = default;
};


#define NFYDEC(fname, sname) \
	struct fname : public params { \
		fname(const params& p) : params(p) { } \
		sname* operator->() { return reinterpret_cast<sname*>(this->lParam); } \
	};

}//namespace wl