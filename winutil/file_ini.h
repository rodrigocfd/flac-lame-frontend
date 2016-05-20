/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <map>
#include <string>

namespace winutil {

class file_ini final {
public:
	std::map<std::wstring, std::map<std::wstring, std::wstring>> data;

	bool                load_from_file(const std::wstring& src, std::wstring* pErr = nullptr);
	bool                save_to_file(const std::wstring& dest, std::wstring* pErr = nullptr) const;
	std::wstring        serialize() const;
	std::wstring&       val(const std::wstring& section, const std::wstring& key)           { return data.at(section).at(key); }
	const std::wstring& val(const std::wstring& section, const std::wstring& key) const     { return data.at(section).at(key); }
	bool                has_section(const std::wstring& section) const                      { return data.count(section) > 0; }
	bool                has_key(const std::wstring& section, const std::wstring& key) const { return !has_section(section) ? false : data.at(section).count(key) > 0; }
	file_ini&           add(const std::wstring& section);
	file_ini&           add(const std::wstring& section, const std::wstring& key, const std::wstring& value = L"");
	file_ini&           clear_section(const std::wstring& section);
};

}//namespace winutil