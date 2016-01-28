
#pragma once
#include <map>
#include <string>

class FileIni final {
public:
	std::map<std::wstring, std::map<std::wstring, std::wstring>> data;
	std::wstring path;

	bool                loadFromFile(std::wstring *pErr = nullptr);
	bool                saveToFile(std::wstring *pErr = nullptr) const;
	std::wstring        serialize() const;
	std::wstring&       val(const wchar_t *section, const wchar_t *key);
	const std::wstring& val(const wchar_t *section, const wchar_t *key) const;
	bool                hasSection(const wchar_t *section) const;
	bool                hasSection(const std::wstring& section) const;
	bool                hasKey(const wchar_t *section, const wchar_t *key) const;
};