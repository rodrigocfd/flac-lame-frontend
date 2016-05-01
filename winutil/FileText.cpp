
#include <Windows.h>
#include "FileText.h"
#include "FileMap.h"
#include "Str.h"
using std::pair;
using std::vector;
using std::wstring;

pair<FileText::Encoding, int> FileText::getEncoding(const BYTE *src, size_t sz)
{
	auto match = [&](const BYTE *pBom, int szBom)->bool {
		return (sz >= static_cast<size_t>(szBom)) &&
			!memcmp(src, pBom, sizeof(BYTE) * szBom);
	};

	// https://en.wikipedia.org/wiki/Byte_order_mark

	BYTE utf8[] = { 0xEF, 0xBB, 0xBF };
	if (match(utf8, 3)) {
		return std::make_pair(FileText::Encoding::UTF8, 3); // also return BOM size, in bytes
	}

	BYTE utf16be[] = { 0xFE, 0xFF };
	if (match(utf16be, 2)) {
		return std::make_pair(FileText::Encoding::UTF16BE, 2);
	}

	BYTE utf16le[] = { 0xFF, 0xFE };
	if (match(utf16le, 2)) {
		return std::make_pair(FileText::Encoding::UTF16LE, 2);
	}

	BYTE utf32be[] = { 0x00, 0x00, 0xFE, 0xFF };
	if (match(utf32be, 4)) {
		return std::make_pair(FileText::Encoding::UTF32BE, 4);
	}

	BYTE utf32le[] = { 0xFF, 0xFE, 0x00, 0x00 };
	if (match(utf32le, 4)) {
		return std::make_pair(FileText::Encoding::UTF32LE, 4);
	}

	BYTE scsu[] = { 0x0E, 0xFE, 0xFF };
	if (match(scsu, 3)) {
		return std::make_pair(FileText::Encoding::SCSU, 3);
	}

	BYTE bocu1[] = { 0xFB, 0xEE, 0x28 };
	if (match(bocu1, 3)) {
		return std::make_pair(FileText::Encoding::BOCU1, 3);
	}

	if (IsTextUnicode(src, static_cast<int>(sz), nullptr)) {
		return std::make_pair(FileText::Encoding::UTF8, 0); // no BOM
	}

	return std::make_pair(FileText::Encoding::ASCII, 0);
}

pair<FileText::Encoding, int> FileText::getEncoding(const wchar_t *path, wstring *pErr)
{
	FileMap fin;
	if (!fin.open(path, File::Access::READONLY, pErr)) {
		return std::make_pair(Encoding::UNKNOWN, 0);
	}
	
	if (pErr) pErr->clear();
	return getEncoding(fin.pMem(), fin.size());
}

const wchar_t* FileText::getLinebreak(const wstring& src)
{
	for (size_t i = 0; i < src.size() - 1; ++i) {
		if (src[i] == L'\r') {
			return src[i + 1] == L'\n' ? L"\r\n" : L"\r";
		} else if (src[i] == L'\n') {
			return src[i + 1] == L'\r' ? L"\n\r" : L"\n";
		}
	}
	return nullptr; // unknown
}

bool FileText::read(wstring& buf, const BYTE *src, size_t sz, wstring *pErr)
{
	pair<Encoding, int> fileEnc = getEncoding(src, sz);
	src += fileEnc.second; // skip BOM, if any

	auto happy = [&]()->bool {
		if (pErr) pErr->clear();
		return true;
	};

	auto fail = [&](const wchar_t *errMsg)->bool {
		if (pErr) *pErr = errMsg;
		return false;
	};

	switch (fileEnc.first) {
	case Encoding::UNKNOWN:
	case Encoding::ASCII:   buf = Str::parseAscii(src, sz); return happy();
	case Encoding::UTF8:    buf = Str::parseUtf8(src, sz); return happy();
	case Encoding::UTF16BE: return fail(L"UTF-16 big endian: encoding not implemented.");
	case Encoding::UTF16LE: return fail(L"UTF-16 little endian: encoding not implemented.");
	case Encoding::UTF32BE: return fail(L"UTF-32 big endian: encoding not implemented.");
	case Encoding::UTF32LE: return fail(L"UTF-32 little endian: encoding not implemented.");
	case Encoding::SCSU:    return fail(L"Standard compression scheme for Unicode: encoding not implemented.");
	case Encoding::BOCU1:   return fail(L"Binary ordered compression for Unicode: encoding not implemented.");
	}

	return fail(L"Failed to parse: unknown encoding.");
}

bool FileText::read(wstring& buf, const wchar_t *path, wstring *pErr)
{
	FileMap fin;
	if (!fin.open(path, File::Access::READONLY, pErr)) {
		return false;
	}

	return read(buf, fin.pMem(), fin.size(), pErr);
}

bool FileText::readLines(vector<wstring>& buf, const BYTE *src, size_t sz, wstring *pErr)
{
	wstring blob;
	if (!read(blob, src, sz, pErr)) {
		return false;
	}

	const wchar_t *delim = getLinebreak(blob);
	if (!delim) {
		if (pErr) *pErr = L"Linebreak could not be determined.";
		return false;
	}

	buf = Str::explode(blob, delim);
	if (pErr) pErr->clear();
	return true;
}

bool FileText::readLines(vector<wstring>& buf, const wchar_t *path, wstring *pErr)
{
	FileMap fin;
	if (!fin.open(path, File::Access::READONLY, pErr)) {
		return false;
	}

	return readLines(buf, fin.pMem(), fin.size(), pErr);
}

bool FileText::writeUtf8(const wstring& text, const wchar_t *path, wstring *pErr)
{
	File fout;
	if (!fout.open(path, File::Access::READWRITE, pErr)) {
		return false;
	}

	std::vector<BYTE> data = Str::serializeUtf8(text);
	if (!fout.write(data, pErr)) { // no BOM is written
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}
