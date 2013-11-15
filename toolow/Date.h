//
// Automation for date and time.
// Morning of Saturday, November 3, 2012.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include <Windows.h>

class Date {
public:
	Date()                              { setNow(); }
	explicit Date(LONGLONG ms)          { setFromMs(ms); }
	explicit Date(const SYSTEMTIME *st) { setFromSt(st); }
	explicit Date(const FILETIME *ft)   { setFromFt(ft); }

	Date&             setNow();
	Date&             setFromSt(const SYSTEMTIME *st) { ::memcpy(&_st, st, sizeof(SYSTEMTIME)); return *this; }
	Date&             setFromMs(LONGLONG ms);
	Date&             setFromFt(const FILETIME *ft);
	const SYSTEMTIME& get() const                     { return _st; }
	LONGLONG          getTimestamp() const;
	LONGLONG          minus(const Date& other) const;
	Date&             addMs(LONGLONG ms);
	Date&             addSec(LONGLONG sec)            { return addMs(sec * 1000); }
	Date&             addMin(LONGLONG min)            { return addSec(min * 60); }
	Date&             addHour(LONGLONG h)             { return addMin(h * 60); }
	Date&             addDay(LONGLONG d)              { return addHour(d * 24); }

private:
	static void _StToLi(const SYSTEMTIME *pst, LARGE_INTEGER *pli);
	static void _LiToSt(const LARGE_INTEGER *pli, SYSTEMTIME *pst);

	SYSTEMTIME _st;
};