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
	std::wstring iniPath;

	bool         load_from_file(std::wstring *pErr = nullptr);
	bool         save_to_file(std::wstring *pErr = nullptr) const;
	std::wstring serialize() const;
	
	std::wstring& val(const wchar_t *section,      const wchar_t *key)      { return data.at(section).at(key); }
	std::wstring& val(const std::wstring& section, const wchar_t *key)      { return val(section.c_str(), key); }
	std::wstring& val(const wchar_t *section,      const std::wstring& key) { return val(section, key.c_str()); }
	std::wstring& val(const std::wstring& section, const std::wstring& key) { return val(section.c_str(), key.c_str()); }

	const std::wstring& val(const wchar_t *section,      const wchar_t *key)      const { return data.at(section).at(key); }
	const std::wstring& val(const std::wstring& section, const wchar_t *key)      const { return val(section.c_str(), key); }
	const std::wstring& val(const wchar_t *section,      const std::wstring& key) const { return val(section, key.c_str()); }
	const std::wstring& val(const std::wstring& section, const std::wstring& key) const { return val(section.c_str(), key.c_str()); }
	
	bool has_section(const wchar_t *section)      const { return data.count(section) > 0; }
	bool has_section(const std::wstring& section) const { return has_section(section.c_str()); }
	
	bool has_key(const wchar_t *section,      const wchar_t *key)      const { return !has_section(section) ? false : data.at(section).count(key) > 0; }
	bool has_key(const std::wstring& section, const wchar_t *key)      const { return has_key(section.c_str(), key); }
	bool has_key(const wchar_t *section,      const std::wstring& key) const { return has_key(section, key.c_str()); }
	bool has_key(const std::wstring& section, const std::wstring& key) const { return has_key(section.c_str(), key.c_str()); }

	file_ini& add(const wchar_t *section);
	file_ini& add(const std::wstring& section) { return add(section.c_str()); }

	file_ini& add(const wchar_t *section,      const wchar_t *key,      const wchar_t *value = L"");
	file_ini& add(const std::wstring& section, const wchar_t *key,      const wchar_t *value = L"")      { return add(section.c_str(), key, value); }
	file_ini& add(const wchar_t *section,      const std::wstring& key, const wchar_t *value = L"")      { return add(section, key.c_str(), value); }
	file_ini& add(const wchar_t *section,      const wchar_t *key,      const std::wstring& value = L"") { return add(section, key, value.c_str()); }
	file_ini& add(const std::wstring& section, const std::wstring& key, const wchar_t *value = L"")      { return add(section.c_str(), key.c_str(), value); }
	file_ini& add(const std::wstring& section, const wchar_t *key,      const std::wstring& value = L"") { return add(section.c_str(), key, value.c_str()); }
	file_ini& add(const wchar_t *section,      const std::wstring& key, const std::wstring& value = L"") { return add(section, key.c_str(), value.c_str()); }
	file_ini& add(const std::wstring& section, const std::wstring& key, const std::wstring& value = L"") { return add(section.c_str(), key.c_str(), value.c_str()); }

	file_ini& clear_section(const wchar_t *section);
	file_ini& clear_section(const std::wstring& section) { return clear_section(section.c_str()); }
};

}//namespace winutil