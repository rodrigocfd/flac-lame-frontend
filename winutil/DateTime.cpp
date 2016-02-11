
#include "DateTime.h"

DateTime::DateTime()
{
	setNow();
}

DateTime::DateTime(LONGLONG ms)
{
	setFromMs(ms);
}

DateTime::DateTime(const SYSTEMTIME& st)
{
	setFromSt(st);
}

DateTime::DateTime(const FILETIME& ft)
{
	setFromFt(ft);
}

const SYSTEMTIME& DateTime::get() const
{
	return _st;
}

DateTime& DateTime::setNow()
{
	SYSTEMTIME st1 = { 0 };
	GetSystemTime(&st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &_st);

	return *this;
}

DateTime& DateTime::setFromSt(const SYSTEMTIME& st)
{
	memcpy(&_st, &st, sizeof(SYSTEMTIME));
	return *this;
}

DateTime& DateTime::setFromFt(const FILETIME& ft)
{
	SYSTEMTIME st1 = { 0 };
	FileTimeToSystemTime(&ft, &st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &_st);

	return *this;
}

DateTime& DateTime::setFromMs(LONGLONG ms)
{
	SecureZeroMemory(&_st, sizeof(SYSTEMTIME));

	_st.wMilliseconds = ms % 1000;
	ms = (ms - _st.wMilliseconds) / 1000; // now in seconds
	_st.wSecond = ms % 60;
	ms = (ms - _st.wSecond) / 60; // now in minutes
	_st.wMinute = ms % 60;
	ms = (ms - _st.wMinute) / 60; // now in hours
	_st.wHour = ms % 24;
	ms = (ms - _st.wHour) / 24; // now in days

	return *this;
}

LONGLONG DateTime::getTimestamp() const
{
	// http://www.frenk.com/2009/12/convert-filetime-to-unix-timestamp/
	LARGE_INTEGER date, adjust;
	_stToLi(_st, date);
	adjust.QuadPart = 11644473600000 * 10000; // 100-nanoseconds = milliseconds * 10000
	date.QuadPart -= adjust.QuadPart; // removes the diff between 1970 and 1601
	//return date.QuadPart / 10000000; // converts back from 100-nanoseconds to seconds
	return date.QuadPart / 10000; // to milliseconds; to printf use %I64u
}

LONGLONG DateTime::minus(const DateTime& other) const
{
	LARGE_INTEGER liUs, liThem;
	_stToLi(_st, liUs);
	_stToLi(other._st, liThem);
	return (liUs.QuadPart - liThem.QuadPart) / 10000; // 100-nanoseconds to milliseconds; to printf use %I64u
}

DateTime& DateTime::addMs(LONGLONG ms)
{
	LARGE_INTEGER li;
	_stToLi(_st, li);
	li.QuadPart += ms * 10000; // milliseconds to 100-nanoseconds
	_liToSt(li, _st);
	return *this;
}

DateTime& DateTime::addSec(LONGLONG sec)
{
	return addMs(sec * 1000);
}

DateTime& DateTime::addMin(LONGLONG min)
{
	return addSec(min * 60);
}

DateTime& DateTime::addHour(LONGLONG h)
{
	return addMin(h * 60);
}

DateTime& DateTime::addDay(LONGLONG d)
{
	return addHour(d * 24);
}

void DateTime::_stToLi(const SYSTEMTIME& st, LARGE_INTEGER& li)
{
	FILETIME ft = { 0 };
	SystemTimeToFileTime(&st, &ft);

	li.HighPart = ft.dwHighDateTime;
	li.LowPart = ft.dwLowDateTime;
}

void DateTime::_liToSt(const LARGE_INTEGER& li, SYSTEMTIME& st)
{
	FILETIME ft = { 0 };
	ft.dwHighDateTime = li.HighPart;
	ft.dwLowDateTime = li.LowPart;

	FileTimeToSystemTime(&ft, &st);
}