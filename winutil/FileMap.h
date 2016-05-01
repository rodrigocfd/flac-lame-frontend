
#pragma once
#include "File.h"

class FileMap final {
private:
	File   _file;
	HANDLE _hMap;
	void  *_pMem;
	size_t _size;
public:
	~FileMap() { close(); }
	FileMap();
	FileMap(FileMap&& fm);
	FileMap& operator=(FileMap&& fm);

	File::Access getAccess() const { return _file.getAccess(); }
	void         close();
	bool         open(const wchar_t *path, File::Access access, std::wstring *pErr = nullptr);
	bool         open(const std::wstring& path, File::Access access, std::wstring *pErr = nullptr) { return open(path.c_str(), access, pErr); }
	size_t       size() const     { return _size; }
	BYTE*        pMem() const     { return reinterpret_cast<BYTE*>(_pMem); }
	BYTE*        pPastMem() const { return pMem() + size(); }
	bool         setNewSize(size_t newSize, std::wstring *pErr = nullptr);
	bool         getContent(std::vector<BYTE>& buf, size_t offset = 0, size_t numBytes = -1, std::wstring *pErr = nullptr) const;
};