
#pragma once
#include <Windows.h>

class DateTime final {
private:
	SYSTEMTIME _st;
public:
	DateTime() = default;
	explicit DateTime(LONGLONG ms)          { setFromMs(ms); }
	explicit DateTime(const SYSTEMTIME& st) { setFromSt(st); }
	explicit DateTime(const FILETIME& ft)   { setFromFt(ft); }

	DateTime&         setNow();
	DateTime&         setFromSt(const SYSTEMTIME& st);
	DateTime&         setFromMs(LONGLONG ms);
	DateTime&         setFromFt(const FILETIME& ft);
	const SYSTEMTIME& get() const          { return _st; }
	LONGLONG          getTimestamp() const;
	LONGLONG          minus(const DateTime& other) const;
	DateTime&         addMs(LONGLONG ms);
	DateTime&         addSec(LONGLONG sec) { return addMs(sec * 1000); }
	DateTime&         addMin(LONGLONG min) { return addSec(min * 60); }
	DateTime&         addHour(LONGLONG h)  { return addMin(h * 60); }
	DateTime&         addDay(LONGLONG d)   { return addHour(d * 24); }
private:
	static void _stToLi(const SYSTEMTIME& st, LARGE_INTEGER& li);
	static void _liToSt(const LARGE_INTEGER& li, SYSTEMTIME& st);
};