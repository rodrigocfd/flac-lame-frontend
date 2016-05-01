
#pragma once
#include <map>
#include <string>

class FileIni final {
public:
	std::map<std::wstring, std::map<std::wstring, std::wstring>> data;
	std::wstring path;

	bool         loadFromFile(std::wstring *pErr = nullptr);
	bool         saveToFile(std::wstring *pErr = nullptr) const;
	std::wstring serialize() const;
	
	std::wstring& val(const wchar_t *section,      const wchar_t *key)      { return data.at(section).at(key); }
	std::wstring& val(const std::wstring& section, const wchar_t *key)      { return val(section.c_str(), key); }
	std::wstring& val(const wchar_t *section,      const std::wstring& key) { return val(section, key.c_str()); }
	std::wstring& val(const std::wstring& section, const std::wstring& key) { return val(section.c_str(), key.c_str()); }

	const std::wstring& val(const wchar_t *section,      const wchar_t *key)      const { return data.at(section).at(key); }
	const std::wstring& val(const std::wstring& section, const wchar_t *key)      const { return val(section.c_str(), key); }
	const std::wstring& val(const wchar_t *section,      const std::wstring& key) const { return val(section, key.c_str()); }
	const std::wstring& val(const std::wstring& section, const std::wstring& key) const { return val(section.c_str(), key.c_str()); }
	
	bool hasSection(const wchar_t *section)      const { return data.count(section) > 0; }
	bool hasSection(const std::wstring& section) const { return hasSection(section.c_str()); }
	
	bool hasKey(const wchar_t *section,      const wchar_t *key)      const { return !hasSection(section) ? false : data.at(section).count(key) > 0; }
	bool hasKey(const std::wstring& section, const wchar_t *key)      const { return hasKey(section.c_str(), key); }
	bool hasKey(const wchar_t *section,      const std::wstring& key) const { return hasKey(section, key.c_str()); }
	bool hasKey(const std::wstring& section, const std::wstring& key) const { return hasKey(section.c_str(), key.c_str()); }
};