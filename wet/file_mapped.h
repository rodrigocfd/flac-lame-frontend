/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "file.h"

namespace wet {

class file_mapped final {
private:
	file   _file;
	HANDLE _hMap;
	void*  _pMem;

public:
	~file_mapped() { this->close(); }
	file_mapped() : _hMap(nullptr), _pMem(nullptr) { }

	file_mapped(file_mapped&& fm) : _file(std::move(fm._file)), _hMap(fm._hMap), _pMem(fm._pMem) {
		fm._hMap = nullptr;
		fm._pMem = nullptr;
	}

	file_mapped& operator=(file_mapped&& fm) {
		std::swap(this->_file, fm._file);
		std::swap(this->_hMap, fm._hMap);
		std::swap(this->_pMem, fm._pMem);
		return *this;
	}

	file::access get_access() const { return this->_file.get_access(); }
	size_t       size()             { return this->_file.size(); }
	BYTE*        p_mem() const      { return reinterpret_cast<BYTE*>(this->_pMem); }
	BYTE*        p_past_mem()       { return p_mem() + this->size(); }

	void close() {
		if (this->_pMem) {
			UnmapViewOfFile(this->_pMem);
			this->_pMem = nullptr;
		}
		if (this->_hMap) {
			CloseHandle(this->_hMap);
			this->_hMap = nullptr;
		}
		this->_file.close();
	}

	bool open(const std::wstring& filePath, file::access accessType, std::wstring* pErr = nullptr) {
		this->close();

		// Open file.
		if (!this->_file.open_existing(filePath, accessType, pErr)) {
			this->close();
			return false;
		}

		// Mapping into memory.
		this->_hMap = CreateFileMappingW(this->_file.hfile(), nullptr,
			(accessType == file::access::READWRITE) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
		if (!this->_hMap) {
			DWORD err = GetLastError();
			this->close();
			if (pErr) *pErr = str::format(L"CreateFileMapping() failed to create file mapping, error code %u.", err);
			return false;
		}

		// Get pointer to data block.
		this->_pMem = MapViewOfFile(this->_hMap,
			(accessType == file::access::READWRITE) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
		if (!this->_pMem) {
			DWORD err = GetLastError();
			this->close();
			if (pErr) *pErr = str::format(L"MapViewOfFile() failed to map view of file, error code %u.", err);
			return false;
		}

		if (pErr) pErr->clear();
		return true;
	}

	bool set_new_size(size_t newSize, std::wstring* pErr = nullptr) {
		// This method will truncate or expand the file, according to the new size.
		// It will probably fail if file was opened as read-only.
		if (!this->_hMap || !this->_pMem || !this->_file.hfile()) {
			if (pErr) *pErr = L"File is not mapped into memory.";
			return false;
		}

		// Unmap file, but keep it open.
		UnmapViewOfFile(this->_pMem);
		CloseHandle(this->_hMap);

		// Truncate/expand file.
		if (!this->_file.set_new_size(newSize, pErr)) {
			this->close();
			return false;
		}

		// Remap into memory.
		if (!( this->_hMap = CreateFileMappingW(this->_file.hfile(), 0, PAGE_READWRITE, 0, 0, nullptr) )) {
			DWORD err = GetLastError();
			this->close();
			if (pErr) *pErr = str::format(L"CreateFileMapping() failed to recreate file mapping, error code %u.", err);
			return false;
		}

		// Get new pointer to data block, old one just became invalid!
		if (!( this->_pMem = MapViewOfFile(this->_hMap, FILE_MAP_WRITE, 0, 0, 0) )) {
			DWORD err = GetLastError();
			this->close();
			if (pErr) *pErr = str::format(L"MapViewOfFile() failed to remap view of file, error code %u.", err);
			return false;
		}

		if (pErr) pErr->clear();
		return true;
	}

	bool read(std::vector<BYTE>& buf, size_t offset = 0, size_t numBytes = -1, std::wstring* pErr = nullptr) {
		if (!this->_hMap || !this->_pMem || !this->_file.hfile()) {
			if (pErr) *pErr = L"File is not mapped into memory.";
			return false;
		} else if (offset >= this->size()) {
			if (pErr) *pErr = L"Offset is beyond end of file.";
			return false;
		} else if (numBytes == -1 || offset + numBytes > this->size()) {
			numBytes = this->size() - offset; // avoid reading beyond EOF
		}

		buf.resize(numBytes);
		memcpy(&buf[0], this->p_mem() + offset, numBytes * sizeof(BYTE));

		if (pErr) pErr->clear();
		return true;
	}

	static bool quick_read(const std::wstring& srcFilePath, std::vector<BYTE>& buf, std::wstring* pErr = nullptr) {
		file_mapped fin;
		if (!fin.open(srcFilePath, file::access::READONLY, pErr)) return false;
		if (!fin.read(buf, 0, -1, pErr)) return false;
		fin.close();
		return true;
	}

	static bool quick_read(const std::wstring& srcFilePath, std::wstring& buf, std::wstring* pErr = nullptr) {
		std::vector<BYTE> blob;
		if (!quick_read(srcFilePath, blob, pErr)) return false;
		if (!str::parse_blob(blob, buf, pErr)) return false;
		return true;
	}
};

}//namespace wet