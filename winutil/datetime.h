/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <Windows.h>

namespace winutil {

class datetime final {
private:
	SYSTEMTIME _st;
public:
	datetime()                              { set_now(); }
	explicit datetime(LONGLONG ms)          { set_from_ms(ms); }
	explicit datetime(const SYSTEMTIME& st) { set_from_st(st); }
	explicit datetime(const FILETIME& ft)   { set_from_ft(ft); }

	datetime&         set_now();
	datetime&         set_from_st(const SYSTEMTIME& st);
	datetime&         set_from_ms(LONGLONG ms);
	datetime&         set_from_ft(const FILETIME& ft);
	const SYSTEMTIME& get() const          { return _st; }
	LONGLONG          get_timestamp() const;
	size_t            minus(const datetime& other) const;
	datetime&         add_ms(LONGLONG ms);
	datetime&         add_sec(LONGLONG sec) { return add_ms(sec * 1000); }
	datetime&         add_min(LONGLONG min) { return add_sec(min * 60); }
	datetime&         add_hour(LONGLONG h)  { return add_min(h * 60); }
	datetime&         add_day(LONGLONG d)   { return add_hour(d * 24); }
private:
	static void _st_to_li(const SYSTEMTIME& st, LARGE_INTEGER& li);
	static void _li_to_st(const LARGE_INTEGER& li, SYSTEMTIME& st);
};

}//namespace winutil