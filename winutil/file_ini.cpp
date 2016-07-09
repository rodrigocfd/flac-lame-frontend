/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include <vector>
#include "file_ini.h"
#include "file_text.h"
#include "str.h"
using namespace winutil;
using std::map;
using std::vector;
using std::wstring;

bool file_ini::load_from_file(const wstring& src, wstring* pErr)
{
	wstring content;
	if (!file_text::read(content, src, pErr)) {
		return false;
	}

	vector<wstring> lines = str::explode(content, L"\r\n");
	wstring curSection, keyBuf, valBuf;

	for (auto& line : lines) {
		str::trim(line);
		if (line[0] == L'[' && line.back() == L']') { // begin of section found
			curSection.clear();
			curSection.insert(0, &line[1], line.length() - 2);
			data.emplace(curSection, map<wstring, wstring>()); // new section added
		} else if (!data.empty() && !line.empty() && line[0] != L';') { // keys will be read only if within a section
			size_t idxEq = line.find_first_of(L'=');
			if (idxEq != wstring::npos) {
				keyBuf.clear();
				keyBuf.insert(0, &line[0], idxEq);
				str::trim(keyBuf);

				valBuf.clear();
				valBuf.insert(0, &line[idxEq + 1], line.length() - (idxEq + 1));
				str::trim(valBuf);

				data[curSection].emplace(keyBuf, valBuf); // create new entry
			}
		}
	}

	if (pErr) pErr->clear();
	return true;
}

bool file_ini::save_to_file(const wstring& dest, wstring* pErr) const
{
	wstring out = serialize();
	return file_text::write_utf8(out, dest, true, pErr);
}

wstring file_ini::serialize() const
{
	wstring out;
	for (const auto& section : data) {
		out.append(L"[").append(section.first).append(L"]\r\n");
		for (const auto& entry : section.second) {
			out.append(entry.first).append(L"=").append(entry.second).append(L"\r\n");
		}
		out.append(L"\r\n");
	}
	return out;
}

file_ini& file_ini::add(const wstring& section)
{
	if (!has_section(section)) {
		data.emplace(section, map<wstring, wstring>()); // new section added
	}
	
	return *this;
}

file_ini& file_ini::add(const wstring& section, const wstring& key, const wstring& value)
{
	if (!has_section(section)) {
		add(section);
	}

	if (has_key(section, key)) {
		val(section, key) = value; // already exist, just update the value
	} else {
		data[section].emplace(key, value); // create new entry
	}

	return *this;
}

file_ini& file_ini::clear_section(const wstring& section)
{
	if (has_section(section)) {
		data[section].clear();
	}

	return *this;
}