/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <Windows.h>

namespace wet {

class drop_files final {
private:
	HDROP _hDrop;

public:
	drop_files(HDROP hDrop) : _hDrop(hDrop) { }
	
	HDROP hdrop() const {
		return this->_hDrop;
	}

	UINT count() const {
		return DragQueryFileW(this->_hDrop, 0xFFFFFFFF, nullptr, 0);
	}

	std::vector<std::wstring> files() const {
		std::vector<std::wstring> files(this->count()); // alloc return vector

		for (size_t i = 0; i < files.size(); ++i) {
			files[i].resize(DragQueryFileW(this->_hDrop,
				static_cast<UINT>(i), nullptr, 0) + 1, L'\0'); // alloc path string
			DragQueryFileW(this->_hDrop, static_cast<UINT>(i), &files[i][0],
				static_cast<UINT>(files[i].size()));
			files[i].resize(files[i].size() - 1); // trim null
		}

		DragFinish(this->_hDrop);

		std::sort(files.begin(), files.end(),
			[](const std::wstring& a, const std::wstring& b)->bool {
				return lstrcmpiW(a.c_str(), b.c_str()) < 0;
			});
		return files;
	}

	POINT pos() const {
		POINT pt = { 0 };
		DragQueryPoint(this->_hDrop, &pt);
		return pt;
	}
};

}//namespace wet