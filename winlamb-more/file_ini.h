/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "file_mapped.h"
#include "dictionary.h"

namespace wl {

// Wrapper to an INI file.
class file_ini final {
private:
	dictionary_str<dictionary_str_str> _sections;

public:
	const dictionary_str<dictionary_str_str>& sections() const {
		return this->_sections;
	}

	bool load_from_file(const std::wstring& srcFilePath, std::wstring* pErr = nullptr) {
		std::wstring content;
		if (!file_mapped::quick_read(srcFilePath, content, pErr)) return false;
		
		std::vector<std::wstring> lines = str::explode(content, L"\r\n");
		std::wstring strBuf;
		dictionary_str_str* pCurSection = nullptr;

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
					dictionary_str_str::item& newEntry = pCurSection->add(strBuf); // if already exists, will return existent

					strBuf.clear();
					strBuf.insert(0, &line[idxEq + 1], line.length() - (idxEq + 1)); // extract value
					str::trim(strBuf);
					newEntry.value = strBuf;
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

		for (const dictionary_str<dictionary_str_str>::item& sec : this->_sections.entries()) {
			if (isFirst) {
				isFirst = false;
			} else {
				out.append(L"\r\n");
			}
			out.append(L"[").append(sec.key).append(L"]\r\n");

			for (const dictionary_str_str::item& entry : sec.value.entries()) {
				out.append(entry.key).append(L"=")
					.append(entry.value).append(L"\r\n");
			}
		}
		return out;
	}

	bool has_section(const std::wstring& sectionName) const {
		return this->_sections.has(sectionName);
	}

	dictionary_str_str* get_section(const std::wstring& sectionName) {
		dictionary_str<dictionary_str_str>::item* sec = this->_sections.entry(sectionName);
		return sec ? &sec->value : nullptr;
	}

	const dictionary_str_str* get_section(const std::wstring& sectionName) const {
		const dictionary_str<dictionary_str_str>::item* sec = this->_sections.entry(sectionName);
		return sec ? &sec->value : nullptr;
	}

	bool has_key(const std::wstring& sectionName, const std::wstring& keyName) const {
		const dictionary_str_str* sec = this->get_section(sectionName);
		return sec ? sec->has(keyName) : false;
	}

	std::wstring* val(const std::wstring& sectionName, const std::wstring& keyName) {
		dictionary_str_str* sec = this->get_section(sectionName);
		return sec ? sec->val(keyName) : nullptr;
	}

	const std::wstring* val(const std::wstring& sectionName, const std::wstring& keyName) const {
		const dictionary_str_str* sec = this->get_section(sectionName);
		return sec ? sec->val(keyName) : nullptr;
	}
};

}//namespace wl