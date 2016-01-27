
#pragma once
#include "File.h"

class FileMap final {
private:
	File   _file;
	HANDLE _hMap;
	void  *_pMem;
	size_t _size;
public:
	~FileMap();
	FileMap();
	FileMap(FileMap&& fm);
	FileMap& operator=(FileMap&& fm);

	File::Access getAccess() const;
	void         close();
	bool         open(const wchar_t *path, File::Access access, std::wstring *pErr = nullptr);
	bool         open(const std::wstring& path, File::Access access, std::wstring *pErr = nullptr);
	size_t       size() const;
	BYTE*        pMem() const;
	BYTE*        pPastMem() const;
	bool         setNewSize(size_t newSize, std::wstring *pErr = nullptr);
	bool         getContent(std::vector<BYTE>& buf, int offset = 0, int numBytes = -1, std::wstring *pErr = nullptr) const;
};