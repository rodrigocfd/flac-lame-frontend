
#include "FileMap.h"
#include "Str.h"
using std::vector;
using std::wstring;

FileMap::~FileMap()
{
	close();
}

FileMap::FileMap()
	: _hMap(nullptr), _pMem(nullptr), _size(0)
{
}

FileMap::FileMap(FileMap&& fm)
	: _file(std::move(fm._file)), _hMap(fm._hMap), _pMem(fm._pMem), _size(fm._size)
{
	fm._hMap = nullptr;
	fm._pMem = nullptr;
	fm._size = 0;
}

FileMap& FileMap::operator=(FileMap&& fm)
{
	std::swap(_file, fm._file);
	std::swap(_hMap, fm._hMap);
	std::swap(_pMem, fm._pMem);
	std::swap(_size, fm._size);
	return *this;
}

File::Access FileMap::getAccess() const
{
	return _file.getAccess();
}

void FileMap::close()
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

bool FileMap::open(const wchar_t *path, File::Access access, wstring *pErr)
{
	close();

	// Open file.
	if (!_file.open(path, access, pErr)) {
		close();
		return false;
	}

	// Mapping into memory.
	_hMap = CreateFileMapping(_file.hFile(), nullptr,
		(access == File::Access::READWRITE) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if (!_hMap) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = Str::format(L"CreateFileMapping() failed to create file mapping, error code %d.", err);
		return false;
	}

	// Get pointer to data block.
	_pMem = MapViewOfFile(_hMap,
		(access == File::Access::READWRITE) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	if (!_pMem) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = Str::format(L"MapViewOfFile() failed to map view of file, error code %d.", err);
		return false;
	}

	_size = _file.size(); // keep file size
	if (pErr) pErr->clear();
	return true;
}

bool FileMap::open(const wstring& path, File::Access access, wstring *pErr)
{
	return open(path.c_str(), access, pErr);
}

size_t FileMap::size() const
{
	return _size;
}

BYTE* FileMap::pMem() const
{
	return reinterpret_cast<BYTE*>(_pMem);
}

BYTE* FileMap::pPastMem() const
{
	return pMem() + size();
}

bool FileMap::setNewSize(size_t newSize, wstring *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// It will probably fail if file was opened as read-only.

	if (!_hMap || !_pMem || !_file.hFile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	}

	// Unmap file, but keep it open.
	UnmapViewOfFile(_pMem);
	CloseHandle(_hMap);

	// Truncate/expand file.
	if (!_file.setNewSize(newSize, pErr)) {
		close();
		return false;
	}

	// Remap into memory.
	if (!( _hMap = CreateFileMapping(_file.hFile(), 0, PAGE_READWRITE, 0, 0, nullptr) )) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = Str::format(L"CreateFileMapping() failed to recreate file mapping, error code %d.", err);
		return false;
	}

	// Get new pointer to data block, old one just became invalid!
	if (!( _pMem = MapViewOfFile(_hMap, FILE_MAP_WRITE, 0, 0, 0) )) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = Str::format(L"MapViewOfFile() failed to remap view of file, error code %d.", err);
		return false;
	}

	_size = _file.size(); // keep new file size
	if (pErr) pErr->clear();
	return true;
}

bool FileMap::getContent(vector<BYTE>& buf, int offset, int numBytes, wstring *pErr) const
{
	if (!_hMap || !_pMem || !_file.hFile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	} else if (offset >= static_cast<int>(_size)) {
		if (pErr) *pErr = L"Offset is beyond end of file.";
		return false;
	} else if (numBytes == -1 || offset + numBytes > static_cast<int>(_size)) {
		numBytes = static_cast<int>(_size) - offset; // avoid reading beyond EOF
	}

	buf.resize(numBytes);
	memcpy(&buf[0], pMem(), numBytes * sizeof(BYTE));

	if (pErr) pErr->clear();
	return true;
}

bool FileMap::getAnsiContent(wstring& buf, int offset, int numChars, wstring *pErr) const
{
	vector<BYTE> byteBuf;
	if (!getContent(byteBuf, offset, numChars, pErr)) {
		return false;
	}

	buf.reserve(byteBuf.size());
	for (const auto& ch : byteBuf) {
		buf.append(1, static_cast<wchar_t>(ch)); // raw conversion
	}

	if (pErr) pErr->clear();
	return true;
}