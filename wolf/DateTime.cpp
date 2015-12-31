/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "DateTime.h"
using namespace wolf;

DateTime::DateTime()
{
	this->setNow();
}

DateTime::DateTime(LONGLONG ms)
{
	this->setFromMs(ms);
}

DateTime::DateTime(const SYSTEMTIME& st)
{
	this->setFromSt(st);
}

DateTime::DateTime(const FILETIME& ft)
{
	this->setFromFt(ft);
}

const SYSTEMTIME& DateTime::get() const
{
	return this->_st;
}

DateTime& DateTime::setNow()
{
	SYSTEMTIME st1 = { 0 };
	GetSystemTime(&st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &this->_st);

	return *this;
}

DateTime& DateTime::setFromSt(const SYSTEMTIME& st)
{
	memcpy(&this->_st, &st, sizeof(SYSTEMTIME));
	return *this;
}

DateTime& DateTime::setFromFt(const FILETIME& ft)
{
	SYSTEMTIME st1 = { 0 };
	FileTimeToSystemTime(&ft, &st1);

	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);
	SystemTimeToTzSpecificLocalTime(&tzi, &st1, &this->_st);

	return *this;
}

DateTime& DateTime::setFromMs(LONGLONG ms)
{
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

LONGLONG DateTime::getTimestamp() const
{
	// http://www.frenk.com/2009/12/convert-filetime-to-unix-timestamp/
	LARGE_INTEGER date, adjust;
	_stToLi(this->_st, date);
	adjust.QuadPart = 11644473600000 * 10000; // 100-nanoseconds = milliseconds * 10000
	date.QuadPart -= adjust.QuadPart; // removes the diff between 1970 and 1601
	//return date.QuadPart / 10000000; // converts back from 100-nanoseconds to seconds
	return date.QuadPart / 10000; // to milliseconds; to printf use %I64u
}

LONGLONG DateTime::minus(const DateTime& other) const
{
	LARGE_INTEGER liUs, liThem;
	_stToLi(this->_st, liUs);
	_stToLi(other._st, liThem);
	return (liUs.QuadPart - liThem.QuadPart) / 10000; // 100-nanoseconds to milliseconds; to printf use %I64u
}

DateTime& DateTime::addMs(LONGLONG ms)
{
	LARGE_INTEGER li;
	_stToLi(this->_st, li);
	li.QuadPart += ms * 10000; // milliseconds to 100-nanoseconds
	_liToSt(li, this->_st);
	return *this;
}

DateTime& DateTime::addSec(LONGLONG sec)
{
	return this->addMs(sec * 1000);
}

DateTime& DateTime::addMin(LONGLONG min)
{
	return this->addSec(min * 60);
}

DateTime& DateTime::addHour(LONGLONG h)
{
	return this->addMin(h * 60);
}

DateTime& DateTime::addDay(LONGLONG d)
{
	return this->addHour(d * 24);
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