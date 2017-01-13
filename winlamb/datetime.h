/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>

namespace wl {

class datetime final {
private:
	SYSTEMTIME _st;

public:
	datetime()                              { this->set_now(); }
	explicit datetime(LONGLONG ms)          { this->set_from_ms(ms); }
	explicit datetime(const SYSTEMTIME& st) { this->set_from_systemtime(st); }
	explicit datetime(const FILETIME& ft)   { this->set_from_filetime(ft); }
	const SYSTEMTIME& systemtime() const    { return this->_st; }

	datetime& set_now() {
		SYSTEMTIME st1 = { 0 };
		GetSystemTime(&st1);

		TIME_ZONE_INFORMATION tzi = { 0 };
		GetTimeZoneInformation(&tzi);
		SystemTimeToTzSpecificLocalTime(&tzi, &st1, &this->_st);
		return *this;
	}

	datetime& set_from_ms(LONGLONG ms) {
		SecureZeroMemory(&this->_st, sizeof(SYSTEMTIME));

		this->_st.wMilliseconds = ms % 1000;
		ms = (ms - this->_st.wMilliseconds) / 1000; // now in seconds
		this->_st.wSecond = ms % 60;
		ms = (ms - this->_st.wSecond) / 60; // now in minutes
		this->_st.wMinute = ms % 60;
		ms = (ms - this->_st.wMinute) / 60; // now in hours
		this->_st.wHour = ms % 24;
		ms = (ms - this->_st.wHour) / 24; // now in days
		return *this;
	}

	datetime& set_from_systemtime(const SYSTEMTIME& st) {
		memcpy(&this->_st, &st, sizeof(SYSTEMTIME));
		return *this;
	}

	datetime& set_from_filetime(const FILETIME& ft) {
		SYSTEMTIME st1 = { 0 };
		FileTimeToSystemTime(&ft, &st1);

		TIME_ZONE_INFORMATION tzi = { 0 };
		GetTimeZoneInformation(&tzi);
		SystemTimeToTzSpecificLocalTime(&tzi, &st1, &this->_st);

		return *this;
	}

	LONGLONG timestamp() const {
		// http://www.frenk.com/2009/12/convert-filetime-to-unix-timestamp/
		LARGE_INTEGER date, adjust;
		_st_to_li(this->_st, date);
		adjust.QuadPart = 11644473600000 * 10000; // 100-nanoseconds = milliseconds * 10000
		date.QuadPart -= adjust.QuadPart; // removes the diff between 1970 and 1601
										  //return date.QuadPart / 10000000; // converts back from 100-nanoseconds to seconds
		return date.QuadPart / 10000; // to milliseconds; to printf use %I64u
	}

	size_t minus(const datetime& other) const {
		LARGE_INTEGER liUs, liThem;
		_st_to_li(this->_st, liUs);
		_st_to_li(other._st, liThem);

		// 100-nanoseconds to milliseconds; to printf a LARGE_INTEGER use %I64u.
		// To int32, max is 1,193 hours; to int64, a shitload of hours.
		return static_cast<size_t>((liUs.QuadPart - liThem.QuadPart) / 10000);
	}

	datetime& add_ms(LONGLONG ms) {
		LARGE_INTEGER li;
		_st_to_li(this->_st, li);
		li.QuadPart += ms * 10000; // milliseconds to 100-nanoseconds
		_li_to_st(li, this->_st);
		return *this;
	}

	datetime& add_sec(LONGLONG sec) { return this->add_ms(sec * 1000); }
	datetime& add_min(LONGLONG min) { return this->add_sec(min * 60); }
	datetime& add_hour(LONGLONG h)  { return this->add_min(h * 60); }
	datetime& add_day(LONGLONG d)   { return this->add_hour(d * 24); }

private:
	static void _st_to_li(const SYSTEMTIME& st, LARGE_INTEGER& li) {
		FILETIME ft = { 0 };
		SystemTimeToFileTime(&st, &ft);

		li.HighPart = ft.dwHighDateTime;
		li.LowPart = ft.dwLowDateTime;
	}

	static void _li_to_st(const LARGE_INTEGER& li, SYSTEMTIME& st) {
		FILETIME ft = { 0 };
		ft.dwHighDateTime = li.HighPart;
		ft.dwLowDateTime = li.LowPart;

		FileTimeToSystemTime(&ft, &st);
	}
};

}//namespace wl