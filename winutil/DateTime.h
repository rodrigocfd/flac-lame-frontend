
#pragma once
#include <Windows.h>

class DateTime final {
private:
	SYSTEMTIME _st;
public:
	DateTime();
	explicit DateTime(LONGLONG ms);
	explicit DateTime(const SYSTEMTIME& st);
	explicit DateTime(const FILETIME& ft);

	DateTime&         setNow();
	DateTime&         setFromSt(const SYSTEMTIME& st);
	DateTime&         setFromMs(LONGLONG ms);
	DateTime&         setFromFt(const FILETIME& ft);
	const SYSTEMTIME& get() const;
	LONGLONG          getTimestamp() const;
	LONGLONG          minus(const DateTime& other) const;
	DateTime&         addMs(LONGLONG ms);
	DateTime&         addSec(LONGLONG sec);
	DateTime&         addMin(LONGLONG min);
	DateTime&         addHour(LONGLONG h);
	DateTime&         addDay(LONGLONG d);
private:
	static void _stToLi(const SYSTEMTIME& st, LARGE_INTEGER& li);
	static void _liToSt(const LARGE_INTEGER& li, SYSTEMTIME& st);
};