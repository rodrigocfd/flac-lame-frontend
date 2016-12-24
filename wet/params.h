/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "drop_files.h"
#include "menu.h"
#include <CommCtrl.h>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp)) // from windowsx.h
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

namespace wet {

struct params {
	UINT   message;
	WPARAM wParam;
	LPARAM lParam;

	struct activate;
	struct char_;
	struct command;
	struct deadchar;
	struct dropfiles;
	struct getdlgcode;
	struct hscroll;
	struct initmenupopup;
	struct keydown;
	struct keyup;
	struct lbuttondblclk;
	struct lbuttondown;
	struct lbuttonup;
	struct mbuttondblclk;
	struct mbuttondown;
	struct mbuttonup;
	struct mousewheel;
	struct nccalcsize;
	struct nchittest;
	struct notify;
	struct queryendsession;
	struct rbuttondblclk;
	struct rbuttondown;
	struct rbuttonup;
	struct showwindow;
	struct size;
	struct sizing;
	struct timer;
	struct vscroll;
};

struct params::activate final : public params {
	activate(const params& p) : params(p) { }
	bool is_being_activated() const           { return this->params::wParam != WA_INACTIVE; }
	bool activated_not_by_mouse_click() const { return this->params::wParam == WA_ACTIVE; }
	bool activated_by_mouse_click() const     { return this->params::wParam == WA_CLICKACTIVE; }
	HWND swapped_window() const               { return reinterpret_cast<HWND>(this->params::lParam); }
};

struct params::char_ : public params {
	char_(const params& p) : params(p) { }
	WORD char_code() const           { return static_cast<WORD>(this->params::wParam); }
	WORD repeat_count() const        { return LOWORD(this->params::lParam); }
	BYTE scan_code() const           { return LOBYTE(HIWORD(this->params::lParam)); }
	bool is_extended_key() const     { return (this->params::lParam >> 24) & 1; }
	bool has_alt_key() const         { return (this->params::lParam >> 29) & 1; }
	bool key_previously_down() const { return (this->params::lParam >> 30) & 1; }
	bool key_being_released() const  { return (this->params::lParam >> 31) & 1; }
};

struct params::command final : public params {
	command(const params& p) : params(p) { }
	WORD control_id() const          { return LOWORD(this->params::wParam); }
	HWND control_hwnd() const        { return reinterpret_cast<HWND>(this->params::lParam); }
	bool is_from_menu() const        { return HIWORD(this->params::wParam) == 0; }
	bool is_from_accelerator() const { return HIWORD(this->params::wParam) == 1; }
};

struct params::deadchar final : public params::char_ {
	deadchar(const params& p) : char_(p) { }
};

struct params::dropfiles final : public params {
	dropfiles(const params& p) : params(p) { }
	drop_files drop() const { return drop_files(reinterpret_cast<HDROP>(this->params::wParam)); }
};

struct params::getdlgcode final : public params {
	getdlgcode(const params& p) : params(p) { }
	WORD vkey() const { return static_cast<WORD>(this->params::wParam); }
	MSG& msg() const  { return *reinterpret_cast<MSG*>(this->params::lParam); }
};

struct params::hscroll : public params {
	hscroll(const params& p) : params(p) { }
	WORD scroll_request() const { return LOWORD(this->params::wParam); } 
	WORD scroll_pos() const     { return HIWORD(this->params::wParam); }
	HWND scrollbar() const      { return reinterpret_cast<HWND>(this->params::lParam); }
};

struct params::initmenupopup final : public params {
	initmenupopup(const params& p) : params(p) { }
	menu menup() const          { return menu(reinterpret_cast<HMENU>(this->params::wParam)); }
	WORD item_rel_pos() const   { return LOWORD(this->params::lParam); }
	bool is_window_menu() const { return HIWORD(this->params::lParam) == TRUE; }
};

struct params::keydown final : public params {
	keydown(const params& p) : params(p) { }
	WORD vkey() const                { return static_cast<WORD>(this->params::wParam); }
	WORD repeat_count() const        { return LOWORD(this->params::lParam); }
	BYTE scan_code() const           { return LOBYTE(HIWORD(this->params::lParam)); }
	bool is_extended_key() const     { return (this->params::lParam >> 24) & 1; }
	bool key_previously_down() const { return (this->params::lParam >> 30) & 1; }
};

struct params::keyup final : public params {
	keyup(const params& p) : params(p) { }
	WORD vkey() const            { return static_cast<WORD>(this->params::wParam); }
	BYTE scan_code() const       { return LOBYTE(HIWORD(this->params::lParam)); }
	bool is_extended_key() const { return (this->params::lParam >> 24) & 1; }
};

struct params::lbuttondblclk : public params {
	lbuttondblclk(const params& p) : params(p) { }
	bool  has_ctrl() const       { return (this->params::wParam & MK_CONTROL) != 0; }
	bool  has_left_btn() const   { return (this->params::wParam & MK_LBUTTON) != 0; }
	bool  has_middle_btn() const { return (this->params::wParam & MK_MBUTTON) != 0; }
	bool  has_right_btn() const  { return (this->params::wParam & MK_RBUTTON) != 0; }
	bool  has_shift() const      { return (this->params::wParam & MK_SHIFT) != 0; }
	bool  has_xbtn1() const      { return (this->params::wParam & MK_XBUTTON1) != 0; }
	bool  has_xbtn2() const      { return (this->params::wParam & MK_XBUTTON2) != 0; }
	POINT pos() const            { return { GET_X_LPARAM(this->params::lParam), GET_Y_LPARAM(this->params::lParam) }; }
};

struct params::lbuttondown final : public params::lbuttondblclk {
	lbuttondown(const params& p) : lbuttondblclk(p) { }
};

struct params::lbuttonup final : public params::lbuttondblclk {
	lbuttonup(const params& p) : lbuttondblclk(p) { }
};

struct params::mbuttondblclk final : public params::lbuttondblclk {
	mbuttondblclk(const params& p) : lbuttondblclk(p) { }
};

struct params::mbuttondown final : public params::lbuttondblclk {
	mbuttondown(const params& p) : lbuttondblclk(p) { }
};

struct params::mbuttonup final : public params::lbuttondblclk {
	mbuttonup(const params& p) : lbuttondblclk(p) { }
};

struct params::mousewheel final : public params {
	mousewheel(const params& p) : params(p) { }
	short wheel_delta() const    { return GET_WHEEL_DELTA_WPARAM(this->params::wParam); }
	bool  has_ctrl() const       { return (LOWORD(this->params::wParam) & MK_CONTROL) != 0; }
	bool  has_left_btn() const   { return (LOWORD(this->params::wParam) & MK_LBUTTON) != 0; }
	bool  has_middle_btn() const { return (LOWORD(this->params::wParam) & MK_MBUTTON) != 0; }
	bool  has_right_btn() const  { return (LOWORD(this->params::wParam) & MK_RBUTTON) != 0; }
	bool  has_shift() const      { return (LOWORD(this->params::wParam) & MK_SHIFT) != 0; }
	bool  has_xbtn1() const      { return (LOWORD(this->params::wParam) & MK_XBUTTON1) != 0; }
	bool  has_xbtn2() const      { return (LOWORD(this->params::wParam) & MK_XBUTTON2) != 0; }
	POINT pos() const            { return { GET_X_LPARAM(this->params::lParam), GET_Y_LPARAM(this->params::lParam) }; }
};

struct params::nccalcsize final : public params {
	nccalcsize(const params& p) : params(p) { }
	bool               is_nccalcsize() const     { return this->params::wParam == TRUE; }
	bool               is_rect() const           { return this->params::wParam == FALSE; }
	NCCALCSIZE_PARAMS& nccalcsize_params() const { return *reinterpret_cast<NCCALCSIZE_PARAMS*>(this->params::lParam); }
	RECT&              rect() const              { return *reinterpret_cast<RECT*>(this->params::lParam); }
};

struct params::nchittest final : public params {
	nchittest(const params& p) : params(p) { }
	POINT pos() const { return { GET_X_LPARAM(this->params::lParam), GET_Y_LPARAM(this->params::lParam) }; }
};

struct params::notify final : public params {
	notify(const params& p) : params(p) { }
	NMHDR&       nmhdr() const       { return *reinterpret_cast<NMHDR*>(this->params::lParam); }
	NMLVKEYDOWN& nmlvkeydown() const { return *reinterpret_cast<NMLVKEYDOWN*>(this->params::lParam); }
};

struct params::queryendsession final : public params {
	queryendsession(const params& p) : params(p) { }
	bool is_system_issue() const    { return (this->params::lParam & ENDSESSION_CLOSEAPP) != 0; }
	bool is_forced_critical() const { return (this->params::lParam & ENDSESSION_CRITICAL) != 0; }
	bool is_logoff() const          { return (this->params::lParam & ENDSESSION_LOGOFF) != 0; }
	bool is_shutdown() const        { return this->params::lParam == 0; }
};

struct params::rbuttondblclk final : public params::lbuttondblclk {
	rbuttondblclk(const params& p) : lbuttondblclk(p) { }
};

struct params::rbuttondown final : public params::lbuttondblclk {
	rbuttondown(const params& p) : lbuttondblclk(p) { }
};

struct params::rbuttonup final : public params::lbuttondblclk {
	rbuttonup(const params& p) : lbuttondblclk(p) { }
};

struct params::showwindow final : public params {
	showwindow(const params& p) : params(p) { }
	bool is_being_shown() const           { return this->params::wParam != FALSE; }
	bool is_other_away() const            { return this->params::lParam == SW_OTHERUNZOOM; }
	bool is_other_over() const            { return this->params::lParam == SW_OTHERZOOM; }
	bool is_owner_being_minimized() const { return this->params::lParam == SW_PARENTCLOSING; }
	bool is_owner_being_restored() const  { return this->params::lParam == SW_PARENTOPENING; }
};

struct params::size final : public params {
	size(const params& p) : params(p) { }
	bool is_other_maximized() const { return this->params::wParam == SIZE_MAXHIDE; }
	bool is_maximized() const       { return this->params::wParam == SIZE_MAXIMIZED; }
	bool is_other_restored() const  { return this->params::wParam == SIZE_MAXSHOW; }
	bool is_minimized() const       { return this->params::wParam == SIZE_MINIMIZED; }
	bool is_restored() const        { return this->params::wParam == SIZE_RESTORED; }
	SIZE sz() const                 { return { LOWORD(this->params::lParam), HIWORD(this->params::lParam) }; }
};

struct params::sizing final : public params {
	sizing(const params& p) : params(p) { }
	WORD  edge() const          { return static_cast<WORD>(this->params::wParam); }
	RECT& screen_coords() const { *reinterpret_cast<RECT*>(this->params::lParam); }
};

struct params::timer final : public params {
	timer(const params& p) : params(p) { }
	UINT_PTR timer_id() const { return static_cast<UINT_PTR>(this->params::wParam); }
};

struct params::vscroll final : public params::hscroll {
	vscroll(const params& p) : hscroll(p) { }
};

}//namspace wet