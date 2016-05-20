/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include <Windows.h>
#include "file_text.h"
#include "file_map.h"
#include "str.h"
using namespace winutil;
using std::pair;
using std::vector;
using std::wstring;

file_text::encoding_info file_text::get_encoding(const BYTE* src, size_t sz)
{
	auto match = [&](const BYTE* pBom, int szBom)->bool {
		return (sz >= static_cast<size_t>(szBom)) &&
			!memcmp(src, pBom, sizeof(BYTE) * szBom);
	};

	// https://en.wikipedia.org/wiki/Byte_order_mark

	BYTE utf8[] = { 0xEF, 0xBB, 0xBF };
	if (match(utf8, 3)) {
		return { encoding::UTF8, 3 }; // BOM size in bytes
	}

	BYTE utf16be[] = { 0xFE, 0xFF };
	if (match(utf16be, 2)) {
		return { encoding::UTF16BE, 2 };
	}

	BYTE utf16le[] = { 0xFF, 0xFE };
	if (match(utf16le, 2)) {
		return { encoding::UTF16LE, 2 };
	}

	BYTE utf32be[] = { 0x00, 0x00, 0xFE, 0xFF };
	if (match(utf32be, 4)) {
		return { encoding::UTF32BE, 4 };
	}

	BYTE utf32le[] = { 0xFF, 0xFE, 0x00, 0x00 };
	if (match(utf32le, 4)) {
		return { encoding::UTF32LE, 4 };
	}

	BYTE scsu[] = { 0x0E, 0xFE, 0xFF };
	if (match(scsu, 3)) {
		return { encoding::SCSU, 3 };
	}

	BYTE bocu1[] = { 0xFB, 0xEE, 0x28 };
	if (match(bocu1, 3)) {
		return { encoding::BOCU1, 3 };
	}

	if (IsTextUnicode(src, static_cast<int>(sz), nullptr)) {
		return { encoding::UTF8, 0 }; // no BOM
	}

	return { encoding::ASCII, 0 };
}

file_text::encoding_info file_text::get_encoding(const wstring& filePath, wstring* pErr)
{
	file_map fin;
	if (!fin.open(filePath, file::access::READONLY, pErr)) {
		return { encoding::UNKNOWN, 0 };
	}
	
	if (pErr) pErr->clear();
	return get_encoding(fin.p_mem(), fin.size());
}

const wchar_t* file_text::get_linebreak(const wstring& src)
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

bool file_text::read(wstring& buf, const BYTE* src, size_t sz, wstring* pErr)
{
	encoding_info fileEnc = get_encoding(src, sz);
	src += fileEnc.bomSize; // skip BOM, if any

	auto happy = [&]()->bool {
		if (pErr) pErr->clear();
		return true;
	};

	auto fail = [&](const wchar_t* errMsg)->bool {
		if (pErr) *pErr = errMsg;
		return false;
	};

	switch (fileEnc.encType) {
	case encoding::UNKNOWN:
	case encoding::ASCII:   buf = str::parse_ascii(src, sz); return happy();
	case encoding::UTF8:    buf = str::parse_utf8(src, sz); return happy();
	case encoding::UTF16BE: return fail(L"UTF-16 big endian: encoding not implemented.");
	case encoding::UTF16LE: return fail(L"UTF-16 little endian: encoding not implemented.");
	case encoding::UTF32BE: return fail(L"UTF-32 big endian: encoding not implemented.");
	case encoding::UTF32LE: return fail(L"UTF-32 little endian: encoding not implemented.");
	case encoding::SCSU:    return fail(L"Standard compression scheme for Unicode: encoding not implemented.");
	case encoding::BOCU1:   return fail(L"Binary ordered compression for Unicode: encoding not implemented.");
	}

	return fail(L"Failed to parse: unknown encoding.");
}

bool file_text::read(wstring& buf, const wstring& srcPath, wstring* pErr)
{
	file_map fin;
	if (!fin.open(srcPath, file::access::READONLY, pErr)) {
		return false;
	}

	return read(buf, fin.p_mem(), fin.size(), pErr);
}

bool file_text::read_lines(vector<wstring>& buf, const BYTE* src, size_t sz, wstring* pErr)
{
	wstring blob;
	if (!read(blob, src, sz, pErr)) {
		return false;
	}

	const wchar_t* delim = get_linebreak(blob);
	if (!delim) {
		if (pErr) *pErr = L"Linebreak could not be determined.";
		return false;
	}

	buf = str::explode(blob, delim);
	if (pErr) pErr->clear();
	return true;
}

bool file_text::read_lines(vector<wstring>& buf, const wstring& srcPath, wstring* pErr)
{
	file_map fin;
	if (!fin.open(srcPath, file::access::READONLY, pErr)) {
		return false;
	}

	return read_lines(buf, fin.p_mem(), fin.size(), pErr);
}

bool file_text::write_utf8(const wstring& text, const wstring& destPath, wstring* pErr)
{
	file fout;
	if (!fout.open(destPath, file::access::READWRITE, pErr)) {
		return false;
	}

	std::vector<BYTE> data = str::serialize_utf8(text);
	if (!fout.write(data, pErr)) { // no BOM is written
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}
