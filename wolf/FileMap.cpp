/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "FileMap.h"
#include "Str.h"
using namespace wolf;
using std::vector;
using std::wstring;

FileMap::~FileMap()
{
	this->close();
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
	std::swap(this->_file, fm._file);
	std::swap(this->_hMap, fm._hMap);
	std::swap(this->_pMem, fm._pMem);
	std::swap(this->_size, fm._size);
	return *this;
}

File::Access FileMap::getAccess() const
{
	return this->_file.getAccess();
}

void FileMap::close()
{
	if (this->_pMem) {
		UnmapViewOfFile(this->_pMem);
		this->_pMem = nullptr;
	}
	if (this->_hMap) {
		CloseHandle(this->_hMap);
		this->_hMap = nullptr;
	}
	this->_file.close();
	this->_size = 0;
}

bool FileMap::open(const wchar_t *path, File::Access access, wstring *pErr)
{
	this->close();

	// Open file.
	if (!this->_file.open(path, access, pErr)) {
		this->close();
		return false;
	}

	// Mapping into memory.
	this->_hMap = CreateFileMapping(this->_file.hFile(), nullptr,
		(access == File::Access::READWRITE) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if (!this->_hMap) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = Str::format(L"CreateFileMapping() failed to create file mapping, error code %d.", err);
		return false;
	}

	// Get pointer to data block.
	this->_pMem = MapViewOfFile(this->_hMap,
		(access == File::Access::READWRITE) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	if (!this->_pMem) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = Str::format(L"MapViewOfFile() failed to map view of file, error code %d.", err);
		return false;
	}

	this->_size = this->_file.size(); // keep file size
	if (pErr) pErr->clear();
	return true;
}

bool FileMap::open(const wstring& path, File::Access access, wstring *pErr)
{
	return this->open(path.c_str(), access, pErr);
}

size_t FileMap::size() const
{
	return this->_size;
}

BYTE* FileMap::pMem() const
{
	return reinterpret_cast<BYTE*>(this->_pMem);
}

BYTE* FileMap::pPastMem() const
{
	return this->pMem() + this->size();
}

bool FileMap::setNewSize(size_t newSize, wstring *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// It will probably fail if file was opened as read-only.

	if (!this->_hMap || !this->_pMem || !this->_file.hFile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	}

	// Unmap file, but keep it open.
	UnmapViewOfFile(this->_pMem);
	CloseHandle(this->_hMap);

	// Truncate/expand file.
	if (!this->_file.setNewSize(newSize, pErr)) {
		this->close();
		return false;
	}

	// Remap into memory.
	if (!( this->_hMap = CreateFileMapping(this->_file.hFile(), 0, PAGE_READWRITE, 0, 0, nullptr) )) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = Str::format(L"CreateFileMapping() failed to recreate file mapping, error code %d.", err);
		return false;
	}

	// Get new pointer to data block, old one just became invalid!
	if (!( this->_pMem = MapViewOfFile(this->_hMap, FILE_MAP_WRITE, 0, 0, 0) )) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = Str::format(L"MapViewOfFile() failed to remap view of file, error code %d.", err);
		return false;
	}

	this->_size = this->_file.size(); // keep new file size
	if (pErr) pErr->clear();
	return true;
}

bool FileMap::getContent(vector<BYTE>& buf, int offset, int numBytes, wstring *pErr) const
{
	if (!this->_hMap || !this->_pMem || !this->_file.hFile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	} else if (offset >= static_cast<int>(this->_size)) {
		if (pErr) *pErr = L"Offset is beyond end of file.";
		return false;
	} else if (numBytes == -1 || offset + numBytes > static_cast<int>(this->_size)) {
		numBytes = static_cast<int>(this->_size) - offset; // avoid reading beyond EOF
	}

	buf.resize(numBytes);
	memcpy(&buf[0], this->pMem(), numBytes * sizeof(BYTE));

	if (pErr) pErr->clear();
	return true;
}

bool FileMap::getContent(wstring& buf, int offset, int numChars, wstring *pErr) const
{
	vector<BYTE> byteBuf;
	if (!this->getContent(byteBuf, offset, numChars, pErr)) {
		return false;
	}

	buf.resize(byteBuf.size());
	for (size_t i = 0; i < byteBuf.size(); ++i) {
		buf[i] = static_cast<wchar_t>(byteBuf[i]); // raw conversion
	}

	if (pErr) pErr->clear();
	return true;
}