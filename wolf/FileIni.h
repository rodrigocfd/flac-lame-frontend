/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <unordered_map>

namespace wolf {

class FileIni final {
public:
	std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::wstring>> data;

	bool                load(const std::wstring& file, std::wstring *pErr=nullptr);
	bool                load(const wchar_t *file, std::wstring *pErr=nullptr);
	bool                save(const std::wstring& file, std::wstring *pErr=nullptr) const;
	bool                save(const wchar_t *file, std::wstring *pErr=nullptr) const;
	std::wstring        serialize() const;
	std::wstring&       val(const wchar_t *section, const wchar_t *key);
	const std::wstring& val(const wchar_t *section, const wchar_t *key) const;
	bool                hasSection(const wchar_t *section) const;
	bool                hasKey(const wchar_t *section, const wchar_t *key) const;
};

}//namespace wolf