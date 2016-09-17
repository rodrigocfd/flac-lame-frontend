/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <algorithm>
#include "wnd_proc.h"
#include <Dbt.h>
#include <Ras.h>

/**
 * wnd <-- wnd_proc <-- wnd_msgs
 */

namespace winlamb {

template<typename traitsT>
class wnd_msgs : virtual public wnd_proc<traitsT> {
public:
	virtual ~wnd_msgs() = default;

	class par final {
	private:
		struct _parm_shim {
			UINT   message;
			WPARAM wParam;
			LPARAM lParam;
			_parm_shim(const params& p) : message(p.message), wParam(p.wParam), lParam(p.lParam) { }
		};

#define PARMDEC_BEGIN(ftype) \
		struct ftype final : public _parm_shim { \
			ftype(const params& p) : _parm_shim(p) { }
#define PARMDEC_END \
		};
#define PARMDEC(ftype) \
		PARMDEC_END \
		PARMDEC_BEGIN(ftype)

#pragma region Parameter declarations
	public:
		PARMDEC_BEGIN(command)
			WORD control_id() const          { return LOWORD(this->_parm_shim::wParam); }
			HWND control_hwnd() const        { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
			bool is_from_menu() const        { return HIWORD(this->_parm_shim::wParam) == 0; }
			bool is_from_accelerator() const { return HIWORD(this->_parm_shim::wParam) == 1; }
		PARMDEC(notify)
			NMHDR& nmhdr() const { return *reinterpret_cast<NMHDR*>(this->_parm_shim::lParam); }
		PARMDEC_END

		PARMDEC_BEGIN(activate)
			bool is_being_activated() const           { return this->_parm_shim::wParam != 0; }
			bool activated_not_by_mouse_click() const { return this->_parm_shim::wParam == 1; }
			bool activated_by_mouse_click() const     { return this->_parm_shim::wParam == 2; }
			HWND swapped_window() const               { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
		PARMDEC(activateapp)
			bool  is_being_activated() const { return this->_parm_shim::wParam != FALSE; }
			DWORD thread_id() const          { return reinterpret_cast<DWORD>(this->_parm_shim::lParam); }
		PARMDEC(askcbformatname)
			UINT   szbuffer() const { return static_cast<UINT>(this->_parm_shim::wParam); }
			TCHAR* buffer() const   { return reinterpret_cast<TCHAR*>(this->_parm_shim::lParam); }
		PARMDEC(cancelmode)
		PARMDEC(capturechanged)
			HWND window_gaining_mouse() const { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
		PARMDEC(changecbchain)
			HWND window_being_removed() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
			HWND next_window() const          { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
			bool is_last_window() const       { return this->next_window() == nullptr; }
		PARMDEC(char_)
			WORD char_code() const           { return static_cast<WORD>(this->_parm_shim::wParam); }
			WORD repeat_count() const        { return LOWORD(this->_parm_shim::lParam); }
			BYTE scan_code() const           { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool is_extended_key() const     { return (this->_parm_shim::lParam >> 24) & 1; }
			bool has_alt_key() const         { return (this->_parm_shim::lParam >> 29) & 1; }
			bool key_previously_down() const { return (this->_parm_shim::lParam >> 30) & 1; }
			bool key_being_released() const  { return (this->_parm_shim::lParam >> 31) & 1; }
		PARMDEC(chartoitem)
			WORD char_code() const         { return LOWORD(this->_parm_shim::wParam); }
			WORD current_caret_pos() const { return HIWORD(this->_parm_shim::wParam); }
			HWND hlistbox() const          { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
		PARMDEC(childactivate)
		PARMDEC(close)
		PARMDEC(compacting)
			UINT cpu_time_ratio() const { return static_cast<UINT>(this->_parm_shim::wParam); }
		PARMDEC(compareitem)
			WORD               control_id() const        { return static_cast<WORD>(this->_parm_shim::wParam); }
			COMPAREITEMSTRUCT& compareitemstruct() const { return *reinterpret_cast<COMPAREITEMSTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(contextmenu)
			HWND  target() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
			POINT pos() const    { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; }
		PARMDEC(copydata)
			COPYDATASTRUCT& copydatastruct() const { return *reinterpret_cast<COPYDATASTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(create)
			CREATESTRUCT& createstruct() const { return *reinterpret_cast<CREATESTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC_END

#define PARMDEC_CTLCOLOR(ctl) \
		PARMDEC_BEGIN(ctlcolor##ctl) \
			HDC  hdc_##ctl() const { return reinterpret_cast<HDC>(this->_parm_shim::wParam); } \
			HWND h##ctl() const    { return reinterpret_cast<HWND>(this->_parm_shim::lParam); } \
		PARMDEC_END
		PARMDEC_CTLCOLOR(btn);
		PARMDEC_CTLCOLOR(dlg);
		PARMDEC_CTLCOLOR(edit);
		PARMDEC_CTLCOLOR(listbox);
		PARMDEC_CTLCOLOR(scrollbar);
		PARMDEC_CTLCOLOR(static);

		PARMDEC_BEGIN(deadchar)
			WORD char_code() const           { return static_cast<WORD>(this->_parm_shim::wParam); }
			WORD repeat_count() const        { return LOWORD(this->_parm_shim::lParam); }
			BYTE scan_code() const           { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool is_extended_key() const     { return (this->_parm_shim::lParam >> 24) & 1; }
			bool has_alt_key() const         { return (this->_parm_shim::lParam >> 29) & 1; }
			bool key_previously_down() const { return (this->_parm_shim::lParam >> 30) & 1; }
			bool key_being_released() const  { return (this->_parm_shim::lParam >> 31) & 1; }
		PARMDEC(deleteitem)
			WORD              control_id() const       { return static_cast<WORD>(this->_parm_shim::wParam); }
			DELETEITEMSTRUCT& deleteitemstruct() const { return *reinterpret_cast<DELETEITEMSTRUCT>(this->_parm_shim::lParam); }
		PARMDEC(destroy)
		PARMDEC(destroyclipboard)
		PARMDEC(devmodechange)
			const TCHAR* device_name() const { return reinterpret_cast<const TCHAR*>(this->_parm_shim::lParam); }
		PARMDEC(devicechange)
			UINT                           event() const                         { return static_cast<UINT>(this->_parm_shim::wParam); }
			DEV_BROADCAST_HDR&             dev_broadcast_hdr() const             { return *reinterpret_cast<DEV_BROADCAST_HDR*>(this->_parm_shim::lParam); }
			DEV_BROADCAST_DEVICEINTERFACE& dev_broadcast_deviceinterface() const { return *reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(this->_parm_shim::lParam); }
			DEV_BROADCAST_HANDLE&          dev_broadcast_handle() const          { return *reinterpret_cast<DEV_BROADCAST_HANDLE*>(this->_parm_shim::lParam); }
			DEV_BROADCAST_OEM&             dev_broadcast_oem() const             { return *reinterpret_cast<DEV_BROADCAST_OEM*>(this->_parm_shim::lParam); }
			DEV_BROADCAST_PORT&            dev_broadcast_port() const            { return *reinterpret_cast<DEV_BROADCAST_PORT*>(this->_parm_shim::lParam); }
			DEV_BROADCAST_VOLUME&          dev_broadcast_volume() const          { return *reinterpret_cast<DEV_BROADCAST_VOLUME*>(this->_parm_shim::lParam); }
		PARMDEC(displaychange)
			UINT bits_per_pixel() const { return static_cast<UINT>(this->_parm_shim::wParam); }
			SIZE sz() const             { return { LOWORD(this->_parm_shim::lParam), HIWORD(this->_parm_shim::lParam) }; }
		PARMDEC(drawclipboard)
		PARMDEC(drawitem)
			WORD            control_id() const     { return static_cast<WORD>(this->_parm_shim::wParam); }
			bool            is_from_menu() const   { return this->control_id() == 0; }
			DRAWITEMSTRUCT& drawitemstruct() const { return *reinterpret_cast<DRAWITEMSTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(dropfiles)
			HDROP hdrop() const { return reinterpret_cast<HDROP>(this->_parm_shim::wParam); }
			UINT  count() const { return DragQueryFile(this->hdrop(), 0xFFFFFFFF, nullptr, 0); }
			std::vector<std::basic_string<TCHAR>> files() const {
				std::vector<std::basic_string<TCHAR>> files(this->count()); // alloc return vector
				for (size_t i = 0; i < files.size(); ++i) {
					files[i].resize(DragQueryFile(this->hdrop(),
						static_cast<UINT>(i), nullptr, 0) + 1, TEXT('\0')); // alloc path string
					DragQueryFile(this->hdrop(), static_cast<UINT>(i), &files[i][0],
						static_cast<UINT>(files[i].size()));
					files[i].resize(files[i].size() - 1); // trim null
				}
				DragFinish(this->hdrop());
				std::sort(files.begin(), files.end(),
					[](const std::basic_string<TCHAR>& a, const std::basic_string<TCHAR>& b)->bool {
					return lstrcmpi(a.c_str(), b.c_str()) < 0;
				});
				return files;
			}
			POINT pos() const {
				POINT pt = { 0 };
				DragQueryPoint(this->hdrop(), &pt);
				return pt;
			}
		PARMDEC(enable)
			bool has_been_enabled() const { return this->_parm_shim::wParam != FALSE; }
		PARMDEC(endsession)
			bool is_session_being_ended() const { return this->_parm_shim::wParam != FALSE; }
			bool is_system_issue() const        { return (this->_parm_shim::lParam & ENDSESSION_CLOSEAPP) != 0; }
			bool is_forced_critical() const     { return (this->_parm_shim::lParam & ENDSESSION_CRITICAL) != 0; }
			bool is_logoff() const              { return (this->_parm_shim::lParam & ENDESSION_LOGOFF) != 0; }
			bool is_shutdown() const            { return this->_parm_shim::lParam == 0; }
		PARMDEC(enteridle)
			bool is_menu_displayed() const { return this->_parm_shim::wParam == MSGF_MENU; }
			HWND hwindow() const           { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
		PARMDEC(entermenuloop)
			bool uses_trackpopupmenu() const { return this->_parm_shim::wParam != FALSE; }
		PARMDEC(entersizemove)
		PARMDEC(erasebkgnd)
			HDC hdc() const { return reinterpret_cast<HDC>(this->_parm_shim::wParam); }
		PARMDEC(exitmenuloop)
			bool is_shortcut_menu() const { return this->_parm_shim::wParam != FALSE; }
		PARMDEC(exitsizemove)
		PARMDEC(fontchange)
		PARMDEC(getdlgcode)
			WORD vkey_code() const { return static_cast<WORD>(this->_parm_shim::wParam); }
			MSG& msg() const       { return *reinterpret_cast<MSG*>(this->_parm_shim::lParam); }
		PARMDEC(getfont)
		PARMDEC(gethotkey)
		PARMDEC(geticon)
			bool is_big() const       { return this->_parm_shim::wParam == ICON_BIG; }
			bool is_small() const     { return this->_parm_shim::wParam == ICON_SMALL; }
			bool is_small_app() const { return this->_parm_shim::wParam == ICON_SMALL2; }
			UINT dpi() const          { return static_cast<UINT>(this->_parm_shim::lParam); }
		PARMDEC(getminmaxinfo)
			MINMAXINFO& minmaxinfo() const { return *reinterpret_cast<MINMAXINFO*>(this->_parm_shim::lParam); }
		PARMDEC(gettext)
			UINT   buffer_size() const { return static_cast<UINT>(this->_parm_shim::wParam); }
			TCHAR* buffer() const      { return reinterpret_cast<TCHAR*>(this->_parm_shim::lParam); }
		PARMDEC(gettextlength)
		PARMDEC(help)
			HELPINFO& helpinfo() const { return reinterpret_cast<HELPINFO*>(this->_parm_shim::lParam); }
		PARMDEC(hotkey)
			bool is_snap_desktop() const { return this->_parm_shim::wParam == IDHOT_SNAPDESKTOP; }
			bool is_snap_window() const  { return this->_parm_shim::wParam == IDHOT_SNAPWINDOW; }
			bool has_alt() const         { return (LOWORD(this->_parm_shim::lParam) & MOD_ALT) != 0; }
			bool has_ctrl() const        { return (LOWORD(this->_parm_shim::lParam) & MOD_CONTROL) != 0; }
			bool has_shift() const       { return (LOWORD(this->_parm_shim::lParam) & MOD_SHIFT) != 0; }
			bool has_win() const         { return (LOWORD(this->_parm_shim::lParam) & MOD_WIN) != 0; }
			WORD vkey_code() const       { return HIWORD(this->_parm_shim::lParam); }
		PARMDEC_END

#define PARMDEC_SCROLL(direc) \
		PARMDEC_BEGIN(direc##scroll) \
			WORD scroll_request() const { return LOWORD(this->_parm_shim::wParam); } \
			WORD scroll_pos() const     { return HIWORD(this->_parm_shim::wParam); } \
			HWND scrollbar() const      { return reinterpret_cast<HWND>(this->_parm_shim::lParam); } \
		PARMDEC_END
		PARMDEC_SCROLL(h);
		PARMDEC_SCROLL(v);

#define PARMDEC_SCROLLCLIPBOARD(direc) \
		PARMDEC_BEGIN(direc##scrollclipboard) \
			HWND clipboard_viewer() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); } \
			WORD scroll_event() const     { return LOWORD(this->_parm_shim::lParam); } \
			WORD scroll_pos() const       { return HIWORD(this->_parm_shim::lParam); } \
		PARMDEC_END
		PARMDEC_SCROLLCLIPBOARD(h)
		PARMDEC_SCROLLCLIPBOARD(v)

		PARMDEC_BEGIN(iconerasebkgnd)
			HDC hdc() const { return reinterpret_cast<HDC>(this->_parm_shim::wParam); }
		PARMDEC(initdialog)
			HWND focused_ctrl() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
		PARMDEC(initmenu)
			HMENU hmenu() const { return reinterpret_cast<HMENU>(this->_parm_shim::wParam); }
		PARMDEC(initmenupopup)
			HMENU hmenu() const          { return reinterpret_cast<HMENU>(this->_parm_shim::wParam); }
			short relative_pos() const   { return LOWORD(this->_parm_shim::lParam); }
			bool  is_window_menu() const { return HIWORD(this->_parm_shim::lParam) != FALSE; }
		PARMDEC(inputlangchange)
			DWORD new_charset() const     { return static_cast<DWORD>(this->_parm_shim::wParam); }
			HKL   keyboard_layout() const { return reinterpret_cast<HKL>(this->_parm_shim::lParam); }
		PARMDEC(inputlangchangerequest)
			bool previous_chosen() const      { return (this->_param_shim::wParam & INPUTLANGCHANGE_BACKWARD) != 0; }
			bool next_chosen() const          { return (this->_param_shim::wParam & INPUTLANGCHANGE_FORWARD) != 0; }
			bool can_be_used_with_sys() const { return (this->_param_shim::wParam & INPUTLANGCHANGE_SYSCHARSET) != 0; }
			HKL  keyboard_layout() const      { return reinterpret_cast<HKL>(this->_parm_shim::lParam); }
		PARMDEC(keydown)
			WORD vkey_code() const           { return static_cast<WORD>(this->_parm_shim::wParam); }
			WORD repeat_count() const        { return LOWORD(this->_parm_shim::lParam); }
			BYTE scan_code() const           { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool is_extended_key() const     { return (this->_parm_shim::lParam >> 24) & 1; }
			bool key_previously_down() const { return (this->_parm_shim::lParam >> 30) & 1; }
		PARMDEC(keyup)
			WORD vkey_code() const       { return static_cast<WORD>(this->_parm_shim::wParam); }
			BYTE scan_code() const       { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool is_extended_key() const { return (this->_parm_shim::lParam >> 24) & 1; }
		PARMDEC(killfocus)
			HWND focused_window() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
		PARMDEC_END

#define PARMDEC_MOUSE(btnmsg) \
		PARMDEC_BEGIN(btnmsg) \
			bool  has_ctrl() const       { return (this->_parm_shim::wParam & MK_CONTROL) != 0; } \
			bool  has_left_btn() const   { return (this->_parm_shim::wParam & MK_LBUTTON) != 0; } \
			bool  has_middle_btn() const { return (this->_parm_shim::wParam & MK_MBUTTON) != 0; } \
			bool  has_right_btn() const  { return (this->_parm_shim::wParam & MK_RBUTTON) != 0; } \
			bool  has_shift() const      { return (this->_parm_shim::wParam & MK_SHIFT) != 0; } \
			bool  has_xbtn1() const      { return (this->_parm_shim::wParam & MK_XBUTTON1) != 0; } \
			bool  has_xbtn2() const      { return (this->_parm_shim::wParam & MK_XBUTTON2) != 0; } \
			POINT pos() const            { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; } \
		PARMDEC_END
		PARMDEC_MOUSE(lbuttondblclk)
		PARMDEC_MOUSE(lbuttondown)
		PARMDEC_MOUSE(lbuttonup)
		PARMDEC_MOUSE(mbuttondblclk)
		PARMDEC_MOUSE(mbuttondown)
		PARMDEC_MOUSE(mbuttonup)
		PARMDEC_MOUSE(mousehover)
		PARMDEC_MOUSE(mousemove)
		PARMDEC_MOUSE(rbuttondblclk)
		PARMDEC_MOUSE(rbuttondown)
		PARMDEC_MOUSE(rbuttonup)

		PARMDEC_BEGIN(mdiactivate)
			HWND activated_child() const   { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
			HWND deactivated_child() const { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
		PARMDEC(measureitem)
			MEASUREITEMSTRUCT& measureitemstruct() const { return *reinterpret_cast<MEASUREITEMSTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(menuchar)
			WORD  char_code() const      { return LOWORD(this->_parm_shim::wParam); }
			bool  is_window_menu() const { return HIWORD(this->_parm_shim::wParam) == MF_SYSMENU; }
			HMENU hmenu() const          { return reinterpret_cast<HMENU>(this->_parm_shim::lParam); }
		PARMDEC(menudrag)
			UINT  initial_pos() const { static_cast<UINT>(this->_parm_shim::wParam); }
			HMENU hmenu() const       { return reinterpret_cast<HMENU>(this->_parm_shim::lParam); }
		PARMDEC(menugetobject)
			MENUGETOBJECTINFO& menugetobjectinfo() const { return *reinterpret_cast<MENUGETOBJECTINFO*>(this->_parm_shim::lParam); }
		PARMDEC(menurbuttonup)
			UINT  index() const { return static_cast<UINT>(this->_parm_shim::wParam); }
			HMENU hmenu() const { return reinterpret_cast<HMENU>(this->_parm_shim::lParam); }
		PARMDEC(menuselect)
			WORD  item() const              { return LOWORD(this->_parm_shim::wParam); }
			bool  has_bitmap() const        { return (HIWORD(this->_parm_shim::wParam) & MF_BITMAP) != 0; }
			bool  is_checked() const        { return (HIWORD(this->_parm_shim::wParam) & MF_CHECKED) != 0; }
			bool  is_disabled() const       { return (HIWORD(this->_parm_shim::wParam) & MF_DISABLED) != 0; }
			bool  is_grayed() const         { return (HIWORD(this->_parm_shim::wParam) & MF_GRAYED) != 0; }
			bool  is_highlighted() const    { return (HIWORD(this->_parm_shim::wParam) & MF_HILITE) != 0; }
			bool  mouse_selected() const    { return (HIWORD(this->_parm_shim::wParam) & MF_MOUSESELECT) != 0; }
			bool  is_owner_draw() const     { return (HIWORD(this->_parm_shim::wParam) & MF_OWNERDRAW) != 0; }
			bool  opens_popup() const       { return (HIWORD(this->_parm_shim::wParam) & MF_POPUP) != 0; }
			bool  is_sysmenu() const        { return (HIWORD(this->_parm_shim::wParam) & MF_SYSMENU) != 0; }
			bool  system_has_closed() const { return HIWORD(this->_parm_shim::wParam) == 0xFFFF && !this->_parm_shim::lParam; }
			HMENU hmenu() const             { return (this->opens_popup() || this->is_sysmenu()) ? reinterpret_cast<HMENU>(this->_parm_shim::lParam) : nullptr; }
		PARMDEC(mouseactivate)
			short hit_test_code() const { return static_cast<short>(LOWORD(this->_parm_shim::lParam)); }
			WORD  mouse_msg_id() const  { return HIWORD(this->_parm_shim::lParam); }
		PARMDEC(mouseleave)
		PARMDEC(mousewheel)
			short wheel_delta() const    { return GET_WHEEL_DELTA_WPARAM(this->_parm_shim::wParam); }
			bool  has_ctrl() const       { return (LOWORD(this->_parm_shim::wParam) & MK_CONTROL) != 0; }
			bool  has_left_btn() const   { return (LOWORD(this->_parm_shim::wParam) & MK_LBUTTON) != 0; }
			bool  has_middle_btn() const { return (LOWORD(this->_parm_shim::wParam) & MK_MBUTTON) != 0; }
			bool  has_right_btn() const  { return (LOWORD(this->_parm_shim::wParam) & MK_RBUTTON) != 0; }
			bool  has_shift() const      { return (LOWORD(this->_parm_shim::wParam) & MK_SHIFT) != 0; }
			bool  has_xbtn1() const      { return (LOWORD(this->_parm_shim::wParam) & MK_XBUTTON1) != 0; }
			bool  has_xbtn2() const      { return (LOWORD(this->_parm_shim::wParam) & MK_XBUTTON2) != 0; }
			POINT pos() const            { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; }
		PARMDEC(move)
			POINT pos() const { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; }
		PARMDEC(moving)
			RECT& screen_coords() const { return *reinterpret_cast<RECT*>(this->_parm_shim::lParam); }
		PARMDEC(ncactivate)
			bool is_active() const { return this->_parm_shim::wParam == TRUE; }
		PARMDEC(nccalcsize)
			bool               is_nccalcsize() const     { return this->_parm_shim::wParam == TRUE; }
			bool               is_rect() const           { return this->_parm_shim::wParam == FALSE; }
			NCCALCSIZE_PARAMS& nccalcsize_params() const { return *reinterpret_cast<NCCALCSIZE_PARAMS*>(this->_parm_shim::lParam); }
			RECT&              rect() const              { return *reinterpret_cast<RECT*>(this->_parm_shim::lParam); }
		PARMDEC(nccreate)
			CREATESTRUCT& createstruct() const { return *reinterpret_cast<CREATESTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(ncdestroy)
		PARMDEC(nchittest)
			POINT pos() const { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; }
		PARMDEC_END

#define PARMDEC_NCBUTTON(ncbtnmsg) \
		PARMDEC_BEGIN(ncbtnmsg) \
			short hit_test_code() const { return static_cast<short>(this->_parm_shim::wParam); } \
			POINT pos() const           { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; } \
		PARMDEC_END
		PARMDEC_NCBUTTON(nclbuttondblclk);
		PARMDEC_NCBUTTON(nclbuttondown);
		PARMDEC_NCBUTTON(nclbuttonup);
		PARMDEC_NCBUTTON(ncmbuttondblclk);
		PARMDEC_NCBUTTON(ncmbuttondown);
		PARMDEC_NCBUTTON(ncmbuttonup);
		PARMDEC_NCBUTTON(ncmousemove);
		PARMDEC_NCBUTTON(ncrbuttondblclk);
		PARMDEC_NCBUTTON(ncrbuttondown);
		PARMDEC_NCBUTTON(ncrbuttonup);

		PARMDEC_BEGIN(ncpaint)
			HRGN hrgn() const { return reinterpret_cast<HRGN>(this->_parm_shim::wParam); }
		PARMDEC(nextdlgctl)
			bool has_ctrl_receiving_focus() const { return LOWORD(this->_parm_shim::lParam) != FALSE; }
			HWND ctrl_receiving_focus() const     { return LOWORD(this->_parm_shim::lParam) ? reinterpret_cast<HWND>(this->_parm_shim::wParam) : nullptr; }
			bool focus_next() const               { return this->_parm_shim::wParam == 0; }
		PARMDEC(nextmenu)
			WORD         vkey_code() const   { return static_cast<WORD>(this->_parm_shim::wParam); }
			MDINEXTMENU& mdinextmenu() const { return *reinterpret_cast<MDINEXTMENU*>(this->_parm_shim::lParam); }
		PARMDEC(notifyformat)
			HWND window_from() const           { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
			bool is_query_from_control() const { return this->_parm_shim::lParam == NF_QUERY; }
			bool is_requery_to_control() const { return this->_parm_shim::lParam == NF_REQUERY; }
		PARMDEC(paint)
		PARMDEC(paintclipboard)
			HWND               clipboard_viewer() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
			const PAINTSTRUCT& paintstruct() const      { return *reinterpret_cast<const PAINTSTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(palettechanged)
			HWND window_origin() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
		PARMDEC(paletteischanging)
			HWND window_origin() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
		PARMDEC(parentnotify)
			UINT  event_message() const { return static_cast<UINT>(LOWORD(this->_parm_shim::wParam)); }
			WORD  child_id() const      { return HIWORD(this->_parm_shim::wParam); }
			HWND  child_hwnd() const    { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
			POINT pos() const           { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; }
			bool  is_xbutton1() const   { return HIWORD(this->_parm_shim::wParam) == XBUTTON1; }
			WORD  pointer_flag() const  { return HIWORD(this->_parm_shim::wParam); }
		PARMDEC(powerbroadcast)
			bool                    is_power_status_change() const  { return this->_parm_shim::wParam == PBT_APMPOWERSTATUSCHANGE; }
			bool                    is_resuming() const             { return this->_parm_shim::wParam == PBT_APMRESUMEAUTOMATIC; }
			bool                    is_suspending() const           { return this->_parm_shim::wParam == PBT_APMSUSPEND; }
			bool                    is_power_setting_change() const { return this->_parm_shim::wParam == PBT_POWERSETTINGCHANGE; }
			POWERBROADCAST_SETTING& power_setting() const           { return *reinterpret_cast<POWERBROADCAST_SETTING*>(this->_parm_shim::lParam); }
		PARMDEC(print)
			HDC  hdc() const   { return reinterpret_cast<HDC>(this->_parm_shim::wParam); }
			UINT flags() const { return static_cast<UINT>(this->_parm_shim::lParam); }
		PARMDEC(printclient)
			HDC  hdc() const   { return reinterpret_cast<HDC>(this->_parm_shim::wParam); }
			UINT flags() const { return static_cast<UINT>(this->_parm_shim::lParam); }
		PARMDEC(querydragicon)
		PARMDEC(queryendsession)
			bool is_system_issue() const    { return (this->_parm_shim::lParam & ENDSESSION_CLOSEAPP) != 0; }
			bool is_forced_critical() const { return (this->_parm_shim::lParam & ENDSESSION_CRITICAL) != 0; }
			bool is_logoff() const          { return (this->_parm_shim::lParam & ENDSESSION_LOGOFF) != 0; }
			bool is_shutdown() const        { return this->_parm_shim::lParam == 0; }
		PARMDEC(querynewpalette)
		PARMDEC(queryopen)
		PARMDEC(rasdialevent)
			RASCONNSTATE rasconnstate() const { return reinterpret_cast<RASCONNSTATE>(this->_parm_shim::wParam); }
			DWORD        error() const        { return static_cast<DWORD>(this->_parm_shim::lParam); }
		PARMDEC(renderallformats)
		PARMDEC(renderformat)
			WORD clipboard_format() const { return static_cast<WORD>(this->_parm_shim::wParam); }
		PARMDEC(setcursor)
			HWND  cursor_owner() const  { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
			short hit_test_code() const { return static_cast<short>(LOWORD(this->_parm_shim::wParam)); }
			WORD  mouse_msg_id() const  { return HIWORD(this->_parm_shim::wParam); }
		PARMDEC(setfocus)
			HWND unfocused_window() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
		PARMDEC(setfont)
			HFONT hfont() const         { return reinterpret_cast<HFONT>(this->_parm_shim::wParam); }
			bool  should_redraw() const { return LOWORD(this->_parm_shim::lParam) != FALSE; }
		PARMDEC(sethotkey)
			WORD vkey_code() const    { return LOWORD(this->_parm_shim::wParam); }
			bool has_alt() const      { return (HIWORD(this->_parm_shim::wParam) & HOTKEYF_ALT) != 0; }
			bool has_ctrl() const     { return (HIWORD(this->_parm_shim::wParam) & HOTKEYF_CONTROL) != 0; }
			bool has_extended() const { return (HIWORD(this->_parm_shim::wParam) & HOTKEYF_EXT) != 0; }
			bool has_shift() const    { return (HIWORD(this->_parm_shim::wParam) & HOTKEYF_SHIFT) != 0; }
		PARMDEC(seticon)
			bool  is_small() const   { return this->_parm_shim::wParam == ICON_SMALL; }
			HICON hicon() const      { return reinterpret_cast<HICON>(this->_parm_shim::lParam); }
			bool  is_removed() const { return this->hicon() == nullptr; }
		PARMDEC(setredraw)
			bool can_redraw() const { return this->_parm_shim::wParam != FALSE; }
		PARMDEC(settext)
			const TCHAR* text() const { return reinterpret_cast<const TCHAR*>(this->_parm_shim::lParam); }
		PARMDEC(settingchange)
			const TCHAR* string_id() const           { return reinterpret_cast<const TCHAR*>(this->_parm_shim::lParam); }
			bool         is_policy() const           { return !lstrcmp(this->string_id(), TEXT("Policy")); }
			bool         is_locale() const           { return !lstrcmp(this->string_id(), TEXT("intl")); }
			bool         is_environment_vars() const { return !lstrcmp(this->string_id(), TEXT("Environment")); }
		PARMDEC(showwindow)
			bool is_being_shown() const           { return this->_parm_shim::wParam != FALSE; }
			bool is_other_away() const            { return this->_parm_shim::lParam == SW_OTHERUNZOOM; }
			bool is_other_over() const            { return this->_parm_shim::lParam == SW_OTHERZOOM; }
			bool is_owner_being_minimized() const { return this->_parm_shim::lParam == SW_PARENTCLOSING; }
			bool is_owner_being_restored() const  { return this->_parm_shim::lParam == SW_PARENTOPENING; }
		PARMDEC(size)
			bool is_other_maximized() const { return this->_parm_shim::wParam == 4; }
			bool is_maximized() const       { return this->_parm_shim::wParam == 2; }
			bool is_other_restored() const  { return this->_parm_shim::wParam == 3; }
			bool is_minimized() const       { return this->_parm_shim::wParam == 1; }
			bool is_restored() const        { return this->_parm_shim::wParam == 0; }
			SIZE sz() const                 { return { LOWORD(this->_parm_shim::lParam), HIWORD(this->_parm_shim::lParam) }; }
		PARMDEC(sizeclipboard)
			HWND        clipboard_viewer() const { return reinterpret_cast<HWND>(this->_parm_shim::wParam); }
			const RECT& clipboard_rect() const   { return *reinterpret_cast<const RECT*>(this->_parm_shim::lParam); }
		PARMDEC(sizing)
			WORD  edge() const          { return static_cast<WORD>(this->_parm_shim::wParam); }
			RECT& screen_coords() const { *reinterpret_cast<RECT*>(this->_parm_shim::lParam); }
		PARMDEC(spoolerstatus)
			UINT status_flag() const    { return static_cast<UINT>(this->_parm_shim::wParam); }
			WORD remaining_jobs() const { return LOWORD(this->_parm_shim::lParam); }
		PARMDEC(stylechanged)
			bool               is_style() const    { return (this->_parm_shim::wParam & GWL_STYLE) != 0; }
			bool               is_ex_style() const { return (this->_parm_shim::wParam & GWL_EXSTYLE) != 0; }
			const STYLESTRUCT& stylestruct() const { return *reinterpret_cast<const STYLESTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(stylechanging)
			bool               is_style() const    { return (this->_parm_shim::wParam & GWL_STYLE) != 0; }
			bool               is_ex_style() const { return (this->_parm_shim::wParam & GWL_EXSTYLE) != 0; }
			const STYLESTRUCT& stylestruct() const { return *reinterpret_cast<const STYLESTRUCT*>(this->_parm_shim::lParam); }
		PARMDEC(syschar)
			WORD char_code() const           { return static_cast<WORD>(this->_parm_shim::wParam); }
			WORD repeat_count() const        { return LOWORD(this->_parm_shim::lParam); }
			BYTE scan_code() const           { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool is_extended_key() const     { return (this->_parm_shim::lParam >> 24) & 1; }
			bool has_alt_key() const         { return (this->_parm_shim::lParam >> 29) & 1; }
			bool key_previously_down() const { return (this->_parm_shim::lParam >> 30) & 1; }
			bool key_being_released() const  { return (this->_parm_shim::lParam >> 31) & 1; }
		PARMDEC(syscolorchange)
		PARMDEC(syscommand)
			WORD  command_type() const { return static_cast<WORD>(this->_parm_shim::wParam); }
			POINT pos() const          { return { GET_X_LPARAM(this->_parm_shim::lParam), GET_Y_LPARAM(this->_parm_shim::lParam) }; }
		PARMDEC(sysdeadchar)
			WORD char_code() const           { return static_cast<WORD>(this->_parm_shim::wParam); }
			WORD repeat_count() const        { return LOWORD(this->_parm_shim::lParam); }
			BYTE scan_code() const           { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool is_extended_key() const     { return (this->_parm_shim::lParam >> 24) & 1; }
			bool has_alt_key() const         { return (this->_parm_shim::lParam >> 29) & 1; }
			bool key_previously_down() const { return (this->_parm_shim::lParam >> 30) & 1; }
			bool key_being_released() const  { return (this->_parm_shim::lParam >> 31) & 1; }
		PARMDEC(syskeydown)
			WORD vkey_code() const           { return static_cast<WORD>(this->_parm_shim::wParam); }
			WORD repeat_count() const        { return LOWORD(this->_parm_shim::lParam); }
			BYTE scan_code() const           { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool is_extended_key() const     { return (this->_parm_shim::lParam >> 24) & 1; }
			bool has_alt_key() const         { return (this->_parm_shim::lParam >> 29) & 1; }
			bool key_previously_down() const { return (this->_parm_shim::lParam >> 30) & 1; }
		PARMDEC(syskeyup)
			WORD vkey_code() const    { return static_cast<WORD>(this->_parm_shim::wParam); }
			WORD repeat_count() const { return LOWORD(this->_parm_shim::lParam); }
			BYTE scan_code() const    { return LOBYTE(HIWORD(this->_parm_shim::lParam)); }
			bool has_alt_key() const  { return (this->_parm_shim::lParam >> 29) & 1; }
		PARMDEC(tcard)
			UINT action_id() const   { return static_cast<UINT>(this->_parm_shim::wParam); }
			long action_data() const { return static_cast<long>(this->_parm_shim::lParam); }
		PARMDEC(timechange)
		PARMDEC(timer)
			UINT_PTR  timer_id() const { return static_cast<UINT_PTR>(this->_parm_shim::wParam); }
			TIMERPROC callback() const { return static_cast<TIMERPROC>(this->_parm_shim::lParam); }
		PARMDEC(uninitmenupopup)
			HMENU hmenu() const   { return reinterpret_cast<HMENU>(this->_parm_shim::wParam); }
			WORD  menu_id() const { return HIWORD(this->_parm_shim::lParam); }
		PARMDEC(userchanged)
		PARMDEC(vkeytoitem)
			WORD vkey_code() const         { return LOWORD(this->_parm_shim::wParam); }
			WORD current_caret_pos() const { return HIWORD(this->_parm_shim::wParam); }
			HWND hlistbox() const          { return reinterpret_cast<HWND>(this->_parm_shim::lParam); }
		PARMDEC(windowposchanged)
			WINDOWPOS& windowpos() const { return *reinterpret_cast<WINDOWPOS*>(this->_parm_shim::lParam); }
		PARMDEC(windowposchanging)
			WINDOWPOS& windowpos() const { return *reinterpret_cast<WINDOWPOS*>(this->_parm_shim::lParam); }
		PARMDEC_END
#pragma endregion
	};

	class handlers final {
	private:
		wnd_msgs& _owner;
	public:
		handlers(wnd_msgs& owner) : _owner(owner) { }

#pragma region WM_COMMAND handler
	private:
		callbacks<WORD, typename par::command, traitsT> _callbacksCommand;
		void _register_command() {
			if (this->_callbacksCommand.empty()) {
				this->_owner.on_message(WM_COMMAND, [this](params p)->typename traitsT::ret_type {
					par::command pcmd{p};
					return this->_callbacksCommand.process(this->_owner.hwnd(),
						WM_COMMAND, pcmd.control_id(), pcmd);
				});
			}
		}
	public:
		using callback_command_type = std::function<typename traitsT::ret_type(typename par::command)>;
		void COMMAND(WORD commandId, callback_command_type callback) {
			this->_register_command();
			this->_callbacksCommand.add(commandId, std::move(callback));
		}
		void COMMAND(std::initializer_list<WORD> commandIds, callback_command_type callback) {
			this->_register_command();
			this->_callbacksCommand.add(commandIds, std::move(callback));
		}
#pragma endregion

#pragma region WM_NOTIFY handler
	private:
		callbacks<std::pair<UINT_PTR, UINT>, typename par::notify, traitsT> _callbacksNotify;
		void _register_notify() {
			if (this->_callbacksNotify.empty()) {
				this->_owner.on_message(WM_NOTIFY, [this](params p)->typename traitsT::ret_type {
					par::notify pnot{p};
					return this->_callbacksNotify.process(this->_owner.hwnd(), WM_NOTIFY,
						{ pnot.nmhdr().idFrom, pnot.nmhdr().code }, pnot);
				});
			}
		}
	public:
		using callback_notify_type = std::function<typename traitsT::ret_type(typename par::notify)>;
		void NOTIFY(UINT_PTR idFrom, UINT code, callback_notify_type callback) {
			this->_register_notify();
			this->_callbacksNotify.add(std::make_pair(idFrom, code), std::move(callback));
		}
		void NOTIFY(std::initializer_list<std::pair<UINT_PTR, UINT>> idFromAndCodes, callback_notify_type callback) {
			this->_register_notify();
			this->_callbacksNotify.add(idFromAndCodes, std::move(callback));
		}
#pragma endregion

#define HANDL(fname, ftype) \
		void fname(std::function<typename traitsT::ret_type(typename par::ftype)> callback) { \
			this->_owner._on_message_proxy(WM_##fname, \
				[cbfunc = std::move(callback)](params p)->typename traitsT::ret_type { \
					return cbfunc(par::ftype{p}); \
				}); \
		}

#pragma region Ordinary handlers
	public:
		HANDL(ACTIVATE, activate)
		HANDL(ACTIVATEAPP, activateapp)
		HANDL(ASKCBFORMATNAME, askcbformatname)
		HANDL(CANCELMODE, cancelmode)
		HANDL(CAPTURECHANGED, capturechanged)
		HANDL(CHANGECBCHAIN, changecbchain)
		HANDL(CHAR, char_)
		HANDL(CHARTOITEM, chartoitem)
		HANDL(CHILDACTIVATE, childactivate)
		HANDL(CLOSE, close)
		HANDL(COMPACTING, compacting)
		HANDL(COMPAREITEM, compareitem)
		HANDL(CONTEXTMENU, contextmenu)
		HANDL(COPYDATA, copydata)
		HANDL(CREATE, create)
		HANDL(CTLCOLORBTN, ctlcolorbtn)
		HANDL(CTLCOLORDLG, ctlcolordlg)
		HANDL(CTLCOLOREDIT, ctlcoloredit)
		HANDL(CTLCOLORLISTBOX, ctlcolorlistbox)
		HANDL(CTLCOLORSCROLLBAR, ctlcolorscrollbar)
		HANDL(CTLCOLORSTATIC, ctlcolorstatic)
		HANDL(DEADCHAR, deadchar)
		HANDL(DELETEITEM, deleteitem)
		HANDL(DESTROY, destroy)
		HANDL(DESTROYCLIPBOARD, destroyclipboard)
		HANDL(DEVICECHANGE, devicechange)
		HANDL(DEVMODECHANGE, devmodechange)
		HANDL(DISPLAYCHANGE, displaychange)
		HANDL(DRAWCLIPBOARD, drawclipboard)
		HANDL(DRAWITEM, drawitem)
		HANDL(DROPFILES, dropfiles)
		HANDL(ENABLE, enable)
		HANDL(ENDSESSION, endsession)
		HANDL(ENTERIDLE, enteridle)
		HANDL(ENTERMENULOOP, entermenuloop)
		HANDL(ENTERSIZEMOVE, entersizemove)
		HANDL(ERASEBKGND, erasebkgnd)
		HANDL(EXITMENULOOP, exitmenuloop)
		HANDL(EXITSIZEMOVE, exitsizemove)
		HANDL(FONTCHANGE, fontchange)
		HANDL(GETDLGCODE, getdlgcode)
		HANDL(GETFONT, getfont)
		HANDL(GETHOTKEY, gethotkey)
		HANDL(GETICON, geticon)
		HANDL(GETMINMAXINFO, getminmaxinfo)
		HANDL(GETTEXT, gettext);
		HANDL(GETTEXTLENGTH, gettextlength)
		HANDL(HELP, help)
		HANDL(HOTKEY, hotkey)
		HANDL(HSCROLL, hscroll)
		HANDL(HSCROLLCLIPBOARD, hscrollclipboard)
		HANDL(ICONERASEBKGND, iconerasebkgnd)
		HANDL(INITDIALOG, initdialog)
		HANDL(INITMENU, initmenu)
		HANDL(INITMENUPOPUP, initmenupopup)
		HANDL(INPUTLANGCHANGE, inputlangchange)
		HANDL(INPUTLANGCHANGEREQUEST, inputlangchangerequest)
		HANDL(KEYDOWN, keydown)
		HANDL(KEYUP, keyup)
		HANDL(KILLFOCUS, killfocus)
		HANDL(LBUTTONDBLCLK, lbuttondblclk)
		HANDL(LBUTTONDOWN, lbuttondown)
		HANDL(LBUTTONUP, lbuttonup)
		HANDL(MBUTTONDBLCLK, mbuttondblclk)
		HANDL(MBUTTONDOWN, mbuttondown)
		HANDL(MBUTTONUP, mbuttonup)
		HANDL(MDIACTIVATE, mdiactivate)
		HANDL(MEASUREITEM, measureitem)
		HANDL(MENUCHAR, menuchar)
		HANDL(MENUDRAG, menudrag)
		HANDL(MENUGETOBJECT, menugetobject)
		HANDL(MENURBUTTONUP, menurbuttonup)
		HANDL(MENUSELECT, menuselect)
		HANDL(MOUSEACTIVATE, mouseactivate)
		HANDL(MOUSEHOVER, mousehover)
		HANDL(MOUSEMOVE, mousemove)
		HANDL(MOUSELEAVE, mouseleave)
		HANDL(MOUSEWHEEL, mousewheel)
		HANDL(MOVE, move)
		HANDL(MOVING, moving)
		HANDL(NCACTIVATE, ncactivate)
		HANDL(NCCALCSIZE, nccalcsize)
		HANDL(NCCREATE, nccreate)
		HANDL(NCDESTROY, ncdestroy)
		HANDL(NCHITTEST, nchittest)
		HANDL(NCLBUTTONDBLCLK, nclbuttondblclk)
		HANDL(NCLBUTTONDOWN, nclbuttondown)
		HANDL(NCLBUTTONUP, nclbuttonup)
		HANDL(NCMBUTTONDBLCLK, ncmbuttondblclk)
		HANDL(NCMBUTTONDOWN, ncmbuttondown)
		HANDL(NCMBUTTONUP, ncmbuttonup)
		HANDL(NCMOUSEMOVE, ncmousemove)
		HANDL(NCPAINT, ncpaint)
		HANDL(NCRBUTTONDBLCLK, ncrbuttondblclk)
		HANDL(NCRBUTTONDOWN, ncrbuttondown)
		HANDL(NCRBUTTONUP, ncrbuttonup)
		HANDL(NEXTDLGCTL, nextdlgctl)
		HANDL(NEXTMENU, nextmenu)
		HANDL(NOTIFYFORMAT, notifyformat)
		HANDL(PAINT, paint)
		HANDL(PAINTCLIPBOARD, paintclipboard)
		HANDL(PALETTECHANGED, palettechanged)
		HANDL(PALETTEISCHANGING, paletteischanging)
		HANDL(PARENTNOTIFY, parentnotify)
		HANDL(POWERBROADCAST, powerbroadcast)
		HANDL(PRINT, print)
		HANDL(PRINTCLIENT, printclient)
		HANDL(QUERYDRAGICON, querydragicon)
		HANDL(QUERYENDSESSION, queryendsession)
		HANDL(QUERYNEWPALETTE, querynewpalette)
		HANDL(QUERYOPEN, queryopen)
#define WM_RASDIALEVENT_ WM_RASDIALEVENT
		HANDL(RASDIALEVENT_, rasdialevent)
		HANDL(RBUTTONDBLCLK, rbuttondblclk)
		HANDL(RBUTTONDOWN, rbuttondown)
		HANDL(RBUTTONUP, rbuttonup)
		HANDL(RENDERALLFORMATS, renderallformats)
		HANDL(RENDERFORMAT, renderformat)
		HANDL(SETCURSOR, setcursor)
		HANDL(SETFOCUS, setfocus)
		HANDL(SETFONT, setfont)
		HANDL(SETHOTKEY, sethotkey)
		HANDL(SETICON, seticon)
		HANDL(SETREDRAW, setredraw)
		HANDL(SETTEXT, settext)
		HANDL(SETTINGCHANGE, settingchange)
		HANDL(SHOWWINDOW, showwindow)
		HANDL(SIZE, size)
		HANDL(SIZECLIPBOARD, sizeclipboard)
		HANDL(SIZING, sizing)
		HANDL(SPOOLERSTATUS, spoolerstatus)
		HANDL(STYLECHANGED, stylechanged)
		HANDL(STYLECHANGING, stylechanging)
		HANDL(SYSCHAR, syschar)
		HANDL(SYSCOLORCHANGE, syscolorchange)
		HANDL(SYSCOMMAND, syscommand)
		HANDL(SYSDEADCHAR, sysdeadchar)
		HANDL(SYSKEYDOWN, syskeydown)
		HANDL(SYSKEYUP, syskeyup)
		HANDL(TCARD, tcard)
		HANDL(TIMECHANGE, timechange)
		HANDL(TIMER, timer)
		HANDL(UNINITMENUPOPUP, uninitmenupopup)
		HANDL(USERCHANGED, userchanged)
		HANDL(VKEYTOITEM, vkeytoitem)
		HANDL(VSCROLL, vscroll)
		HANDL(VSCROLLCLIPBOARD, vscrollclipboard)
		HANDL(WINDOWPOSCHANGED, windowposchanged)
		HANDL(WINDOWPOSCHANGING, windowposchanging)
#pragma endregion
	};

private:
	friend handlers;
	void _on_message_proxy(UINT msg, typename wnd_proc::callback_message_type callback) {
		this->wnd_proc::on_message(msg, std::move(callback));
	}
	
protected:
	handlers on;
	wnd_msgs() : on(*this) { }
};

}//namespace winlamb