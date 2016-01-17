
#include <vector>
#include "FileIni.h"
#include "FileMap.h"
#include "Str.h"
using std::unordered_map;
using std::vector;
using std::wstring;

bool FileIni::load(const wstring& file, wstring *pErr)
{
	return load(file.c_str(), pErr);
}

bool FileIni::load(const wchar_t *file, wstring *pErr)
{
	FileMap fm;
	if (!fm.open(file, File::Access::READONLY, pErr)) return false;

	wstring content;
	if (!fm.getAnsiContent(content, 0, -1, pErr)) return false;

	fm.close();

	vector<wstring> lines = Str::explode(content, L"\r\n");
	wstring curSection, keyBuf, valBuf;

	for (auto& line : lines) {
		Str::trim(line);
		if (line[0] == L'[' && line.back() == L']') { // begin of section found
			curSection.clear();
			curSection.insert(0, &line[1], line.length() - 2);
			data.emplace(curSection, unordered_map<wstring, wstring>()); // new section added
		} else if (!data.empty() && !line.empty() && line[0] != L';') { // keys will be read only if within a section
			size_t idxEq = line.find_first_of(L'=');
			if (idxEq != wstring::npos) {
				keyBuf.clear();
				keyBuf.insert(0, &line[0], idxEq);
				Str::trim(keyBuf);

				valBuf.clear();
				valBuf.insert(0, &line[idxEq + 1], line.length() - (idxEq + 1));
				Str::trim(valBuf);

				data[curSection].emplace(keyBuf, valBuf);
			}
		}
	}
	return true;
}

bool FileIni::save(const wstring& file, wstring *pErr) const
{
	return save(file.c_str(), pErr);
}

bool FileIni::save(const wchar_t *file, wstring *pErr) const
{
	wstring out = serialize();

	File fout;
	if (!fout.open(file, File::Access::READWRITE, pErr)) return false;
	if (!fout.setNewSize(0, pErr)) return false;

	vector<BYTE> raw(out.size(), 0x00);
	for (size_t i = 0; i < out.size(); ++i) {
		raw[i] = static_cast<BYTE>(out[i]); // brute-force
	}

	if (!fout.write(raw, pErr)) return false;
	return true;
}

wstring FileIni::serialize() const
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

wstring& FileIni::val(const wchar_t *section, const wchar_t *key)
{
	return data.at(section).at(key);
}

const wstring& FileIni::val(const wchar_t *section, const wchar_t *key) const
{
	return data.at(section).at(key);
}

bool FileIni::hasSection(const wchar_t *section) const
{
	return data.find(section) != data.end();
}

bool FileIni::hasKey(const wchar_t *section, const wchar_t *key) const
{
	if (!hasSection(section)) return false;
	return data.at(section).find(key) != data.at(section).end();
}