
#include "File.h"
#include "util.h"

void FileMap::close()
{
	if(_pMem) { ::UnmapViewOfFile(_pMem); _pMem = 0; }
	if(_hMap) { ::CloseHandle(_hMap); _hMap = 0; }
	_file.close();
	_size = 0;
}

bool FileMap::open(const wchar_t *path, File::Access access, String *pErr)
{
	this->close(); // make sure everything was properly cleaned up

	// Open file.
	if(!_file.open(path, access, pErr)) {
		this->close();
		return false;
	}

	// Mapping into memory.
	_hMap = ::CreateFileMapping(_file.hFile(), 0,
		access == File::READWRITE ? PAGE_READWRITE : PAGE_READONLY, 0, 0, 0);
	if(!_hMap) {
		this->close();
		if(pErr) pErr->fmt(L"CreateFileMapping() failed to create file mapping, error code %d.",
			::GetLastError());
		return false;
	}

	// Get pointer to data block.
	_pMem = ::MapViewOfFile(_hMap,
		access == File::READWRITE ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	if(!_pMem) {
		this->close();
		if(pErr) pErr->fmt(L"MapViewOfFile() failed to map view of file, error code %d.",
			::GetLastError());
		return false;
	}

	_size = _file.size(); // keep file size
	if(pErr) *pErr = L"";
	return true;
}

bool FileMap::setNewSize(int newSize, String *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// It will probably fail if file was opened as read-only.

	if(!_hMap || !_pMem || !_file.hFile()) {
		if(pErr) *pErr = L"File is not mapped into memory.";
		return false;
	}

	// Unmap file, but keep it open.
	::UnmapViewOfFile(_pMem);
	::CloseHandle(_hMap);

	// Truncate/expand file.
	if(!_file.setNewSize(newSize, pErr)) {
		this->close();
		return false;
	}

	// Remap into memory.
	if(!( _hMap = ::CreateFileMapping(_file.hFile(), 0, PAGE_READWRITE, 0, 0, 0) )) {
		this->close();
		if(pErr) pErr->fmt(L"CreateFileMapping() failed to recreate file mapping, error code %d.",
			::GetLastError());
		return false;
	}

	// Get new pointer to data block, old one just became invalid!
	if(!( _pMem = ::MapViewOfFile(_hMap, FILE_MAP_WRITE, 0, 0, 0) )) {
		this->close();
		if(pErr) pErr->fmt(L"MapViewOfFile() failed to remap view of file, error code %d.",
			::GetLastError());
		return false;
	}

	_size = _file.size(); // keep new file size
	if(pErr) *pErr = L"";
	return true;
}

bool FileMap::getContent(Array<BYTE> *pBuf, int offset, int numBytes, String *pErr)
{
	if(!_hMap || !_pMem || !_file.hFile()) {
		if(pErr) *pErr = L"File is not mapped into memory.";
		return false;
	} else if(offset >= _size) {
		if(pErr) *pErr = L"Offset is beyond end of file.";
		return false;
	} else if(numBytes == -1 || offset + numBytes > _size) {
		numBytes = _size - offset; // avoid reading beyond EOF
	}
	
	pBuf->realloc(numBytes);
	::memcpy(&(*pBuf)[0], this->pMem(), numBytes * sizeof(BYTE));
	
	if(pErr) *pErr = L"";
	return true;
}

bool FileMap::getContent(String *pBuf, int offset, int numChars, String *pErr)
{
	Array<BYTE> byteBuf;
	if(!this->getContent(&byteBuf, offset, numChars, pErr))
		return false;

	pBuf->reserve(byteBuf.size());
	for(int i = 0; i < byteBuf.size(); ++i)
		(*pBuf)[i] = (wchar_t)byteBuf[i]; // raw conversion
	
	if(pErr) *pErr = L"";
	return true;
}