/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "file_map.h"
#include "str.h"
using namespace winutil;
using std::vector;
using std::wstring;

file_map::file_map()
	: _hMap(nullptr), _pMem(nullptr), _size(0)
{
}

file_map::file_map(file_map&& fm)
	: _file(std::move(fm._file)), _hMap(fm._hMap), _pMem(fm._pMem), _size(fm._size)
{
	fm._hMap = nullptr;
	fm._pMem = nullptr;
	fm._size = 0;
}

file_map& file_map::operator=(file_map&& fm)
{
	std::swap(_file, fm._file);
	std::swap(_hMap, fm._hMap);
	std::swap(_pMem, fm._pMem);
	std::swap(_size, fm._size);
	return *this;
}

void file_map::close()
{
	if (_pMem) {
		UnmapViewOfFile(_pMem);
		_pMem = nullptr;
	}
	if (_hMap) {
		CloseHandle(_hMap);
		_hMap = nullptr;
	}
	_file.close();
	_size = 0;
}

bool file_map::open(const wstring& filePath, file::access accessType, wstring* pErr)
{
	close();

	// Open file.
	if (!_file.open(filePath, accessType, pErr)) {
		close();
		return false;
	}

	// Mapping into memory.
	_hMap = CreateFileMapping(_file.hfile(), nullptr,
		(accessType == file::access::READWRITE) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if (!_hMap) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = str::format(L"CreateFileMapping() failed to create file mapping, error code %u.", err);
		return false;
	}

	// Get pointer to data block.
	_pMem = MapViewOfFile(_hMap,
		(accessType == file::access::READWRITE) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	if (!_pMem) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = str::format(L"MapViewOfFile() failed to map view of file, error code %u.", err);
		return false;
	}

	_size = _file.size(); // keep file size
	if (pErr) pErr->clear();
	return true;
}

bool file_map::set_new_size(size_t newSize, wstring* pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// It will probably fail if file was opened as read-only.

	if (!_hMap || !_pMem || !_file.hfile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	}

	// Unmap file, but keep it open.
	UnmapViewOfFile(_pMem);
	CloseHandle(_hMap);

	// Truncate/expand file.
	if (!_file.set_new_size(newSize, pErr)) {
		close();
		return false;
	}

	// Remap into memory.
	if (!( _hMap = CreateFileMapping(_file.hfile(), 0, PAGE_READWRITE, 0, 0, nullptr) )) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = str::format(L"CreateFileMapping() failed to recreate file mapping, error code %u.", err);
		return false;
	}

	// Get new pointer to data block, old one just became invalid!
	if (!( _pMem = MapViewOfFile(_hMap, FILE_MAP_WRITE, 0, 0, 0) )) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = str::format(L"MapViewOfFile() failed to remap view of file, error code %u.", err);
		return false;
	}

	_size = _file.size(); // keep new file size
	if (pErr) pErr->clear();
	return true;
}

bool file_map::get_content(vector<BYTE>& buf, size_t offset, size_t numBytes, wstring* pErr) const
{
	if (!_hMap || !_pMem || !_file.hfile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	} else if (offset >= _size) {
		if (pErr) *pErr = L"Offset is beyond end of file.";
		return false;
	} else if (numBytes == -1 || offset + numBytes > _size) {
		numBytes = _size - offset; // avoid reading beyond EOF
	}

	buf.resize(numBytes);
	memcpy(&buf[0], p_mem() + offset, numBytes * sizeof(BYTE));

	if (pErr) pErr->clear();
	return true;
}