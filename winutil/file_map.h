/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include "file.h"

namespace winutil {

class file_map final {
private:
	file   _file;
	HANDLE _hMap;
	void*  _pMem;
	size_t _size;
public:
	~file_map() { close(); }
	file_map();
	file_map(file_map&& fm);
	file_map& operator=(file_map&& fm);

	file::access get_access() const { return _file.get_access(); }
	void         close();
	bool         open(const std::wstring& filePath, file::access accessType, std::wstring* pErr = nullptr);
	size_t       size() const       { return _size; }
	BYTE*        p_mem() const      { return reinterpret_cast<BYTE*>(_pMem); }
	BYTE*        p_past_mem() const { return p_mem() + size(); }
	bool         set_new_size(size_t newSize, std::wstring* pErr = nullptr);
	bool         get_content(std::vector<BYTE>& buf, size_t offset = 0, size_t numBytes = -1, std::wstring* pErr = nullptr) const;
};

}//namespace winutil