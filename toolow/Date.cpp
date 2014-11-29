//
// Automation for date and time.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "Date.h"

Date& Date::setNow()
{
	SYSTEMTIME st1 = { 0 };
	GetSystemTime(&st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &_st);

	return *this;
}

Date& Date::setFromFt(const FILETIME *ft)
{
	SYSTEMTIME st1 = { 0 };
	FileTimeToSystemTime(ft, &st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &_st);

	return *this;
}

Date& Date::setFromMs(LONGLONG ms)
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

LONGLONG Date::getTimestamp() const
{
	// http://www.frenk.com/2009/12/convert-filetime-to-unix-timestamp/
	LARGE_INTEGER date, adjust;
	_StToLi(&_st, &date);
	adjust.QuadPart = 11644473600000 * 10000; // 100-nanoseconds = milliseconds * 10000
	date.QuadPart -= adjust.QuadPart; // removes the diff between 1970 and 1601
	//return date.QuadPart / 10000000; // converts back from 100-nanoseconds to seconds
	return date.QuadPart / 10000; // to milliseconds; to printf use %I64u
}

LONGLONG Date::minus(const Date &other) const
{
	LARGE_INTEGER liUs, liThem;
	_StToLi(&_st, &liUs);
	_StToLi(&other._st, &liThem);
	return (liUs.QuadPart - liThem.QuadPart) / 10000; // 100-nanoseconds to milliseconds; to printf use %I64u
}

Date& Date::addMs(LONGLONG ms)
{
	LARGE_INTEGER li;
	_StToLi(&_st, &li);
	li.QuadPart += ms * 10000; // milliseconds to 100-nanoseconds
	_LiToSt(&li, &_st);
	return *this;
}

void Date::_StToLi(const SYSTEMTIME *pst, LARGE_INTEGER *pli)
{
	FILETIME ft = { 0 };
	SystemTimeToFileTime(pst, &ft);

	pli->HighPart = ft.dwHighDateTime;
	pli->LowPart = ft.dwLowDateTime;
}

void Date::_LiToSt(const LARGE_INTEGER *pli, SYSTEMTIME *pst)
{
	FILETIME ft = { 0 };
	ft.dwHighDateTime = pli->HighPart;
	ft.dwLowDateTime = pli->LowPart;

	FileTimeToSystemTime(&ft, pst);
}