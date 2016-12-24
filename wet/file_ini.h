/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "file_mapped.h"
#include "hash_map.h"

namespace wet {

class file_ini final {
public:
	using sectionT = hash_map<std::wstring>;
	using keyT = hash_map<std::wstring>::entry;

private:
	hash_map<hash_map<std::wstring>> _sections;

public:
	const hash_map<hash_map<std::wstring>>& sections() const {
		return this->_sections;
	}

	bool load_from_file(const std::wstring& srcFilePath, std::wstring* pErr = nullptr) {
		std::wstring content;
		if (!file_mapped::quick_read(srcFilePath, content, pErr)) return false;
		
		std::vector<std::wstring> lines = str::explode(content, L"\r\n");
		std::wstring strBuf;
		sectionT* pCurSection = nullptr;

		for (std::wstring& line : lines) {
			str::trim(line);
			if (line[0] == L'[' && line.back() == L']') { // begin of section found
				strBuf.clear();
				strBuf.insert(0, &line[1], line.length() - 2); // extract section name
				str::trim(strBuf);
				pCurSection = &this->_sections.add(strBuf).value; // if already exists, will return existent
			} else if (pCurSection && !line.empty() && line[0] != L';') {
				size_t idxEq = line.find_first_of(L'=');
				if (idxEq != std::wstring::npos) {
					strBuf.clear();
					strBuf.insert(0, &line[0], idxEq); // extract key name
					str::trim(strBuf);
					keyT& key = pCurSection->add(strBuf); // if already exists, will return existent

					strBuf.clear();
					strBuf.insert(0, &line[idxEq + 1], line.length() - (idxEq + 1)); // extract value
					str::trim(strBuf);
					key.value = strBuf;
				}
			}
		}
		if (pErr) pErr->clear();
		return true;
	}

	bool save_to_file(const std::wstring& destFilePath, std::wstring* pErr = nullptr) {
		std::wstring out = this->serialize();
		return file::quick_write_utf8(destFilePath, out, true, pErr);
	}

	std::wstring serialize() const {
		std::wstring out;
		bool isFirst = true;

		for (const hash_map<hash_map<std::wstring>>::entry& sec : this->_sections.entries()) {
			if (isFirst) {
				isFirst = false;
			} else {
				out.append(L"\r\n");
			}
			out.append(L"[").append(sec.name).append(L"]\r\n");

			for (const keyT& key : sec.value.entries()) {
				out.append(key.name).append(L"=")
					.append(key.value).append(L"\r\n");
			}
		}
		return out;
	}

	bool has_section(const std::wstring& section) const {
		return this->_sections.has(section);
	}

	sectionT* get_section(const std::wstring& section) {
		hash_map<hash_map<std::wstring>>::entry* sec = this->_sections.get(section);
		return sec ? &sec->value : nullptr;
	}

	const sectionT* get_section(const std::wstring& section) const {
		const hash_map<hash_map<std::wstring>>::entry* sec = this->_sections.get(section);
		return sec ? &sec->value : nullptr;
	}

	bool has_key(const std::wstring& section, const std::wstring& key) const {
		const sectionT* sec = this->get_section(section);
		return sec ? sec->has(key) : false;
	}

	std::wstring* val(const std::wstring& section, const std::wstring& key) {
		sectionT* sec = this->get_section(section);
		return sec ? sec->val(key) : nullptr;
	}

	const std::wstring* val(const std::wstring& section, const std::wstring& key) const {
		const sectionT* sec = this->get_section(section);
		return sec ? sec->val(key) : nullptr;
	}
};

}//namespace wet