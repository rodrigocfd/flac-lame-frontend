
#include <vector>
#include "FileIni.h"
#include "FileText.h"
#include "Str.h"
using std::map;
using std::vector;
using std::wstring;

bool FileIni::loadFromFile(wstring *pErr)
{
	if (path.empty()) {
		if (pErr) *pErr = L"Current path is empty.";
		return false;
	}

	wstring content;
	if (!FileText::read(content, path, pErr)) {
		return false;
	}

	vector<wstring> lines = Str::explode(content, L"\r\n");
	wstring curSection, keyBuf, valBuf;

	for (auto& line : lines) {
		Str::trim(line);
		if (line[0] == L'[' && line.back() == L']') { // begin of section found
			curSection.clear();
			curSection.insert(0, &line[1], line.length() - 2);
			data.emplace(curSection, map<wstring, wstring>()); // new section added
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

	if (pErr) pErr->clear();
	return true;
}

bool FileIni::saveToFile(wstring *pErr) const
{
	if (path.empty()) {
		if (pErr) *pErr = L"Current path is empty.";
		return false;
	}

	wstring out = serialize();
	return FileText::writeUtf8(out, path, pErr);
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