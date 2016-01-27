
#include <vector>
#include <Windows.h>
#include "FileText.h"
#include "FileMap.h"
#include "Str.h"
using std::pair;
using std::vector;
using std::wstring;

static pair<FileText::Encoding, int> _tellEncoding(const FileMap& fin)
{
	auto match = [&fin](const BYTE *pBom, int szBom)->bool {
		return (fin.size() >= static_cast<size_t>(szBom)) &&
			!memcmp(fin.pMem(), pBom, sizeof(BYTE) * szBom);
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

	if (IsTextUnicode(fin.pMem(), static_cast<int>(fin.size()), nullptr)) {
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
	return _tellEncoding(fin);
}

pair<FileText::Encoding, int> FileText::getEncoding(const wstring& path, wstring *pErr)
{
	return getEncoding(path.c_str(), pErr);
}

bool FileText::read(wstring& buf, const wchar_t *path, wstring *pErr)
{
	FileMap fin;
	if (!fin.open(path, File::Access::READONLY, pErr)) {
		return false;
	}

	pair<Encoding, int> fileEnc = _tellEncoding(fin);
	vector<BYTE> byteBuf;
	if (!fin.getContent(byteBuf, fileEnc.second, -1, pErr)) { // skip BOM, if any
		return false;
	}
	fin.close();
	if (pErr) pErr->clear();

	switch (fileEnc.first) {
	case Encoding::UNKNOWN:
	case Encoding::ASCII:
		buf = Str::parseAscii(byteBuf);
		return true;
	case Encoding::UTF8:
		buf = Str::parseUtf8(byteBuf);
		return true;
	case Encoding::UTF16BE:
		if (pErr) *pErr = L"UTF-16 big endian: encoding not implemented.";
		return false;
	case Encoding::UTF16LE:
		if (pErr) *pErr = L"UTF-16 little endian: encoding not implemented.";
		return false;
	case Encoding::UTF32BE:
		if (pErr) *pErr = L"UTF-32 big endian: encoding not implemented.";
		return false;
	case Encoding::UTF32LE:
		if (pErr) *pErr = L"UTF-32 little endian: encoding not implemented.";
		return false;
	case Encoding::SCSU:
		if (pErr) *pErr = L"Standard compression scheme for Unicode: encoding not implemented.";
		return false;
	case Encoding::BOCU1:
		if (pErr) *pErr = L"Binary ordered compression for Unicode: encoding not implemented.";
		return false;
	}

	if (pErr) *pErr = L"Failed to parse: unknown encoding.";
	return false;
}

bool FileText::read(wstring& buf, const wstring& path, wstring *pErr)
{
	return read(buf, path.c_str(), pErr);
}

bool FileText::writeUtf8(const wstring& text, const wchar_t *path, wstring *pErr)
{
	File fout;
	if (!fout.open(path, File::Access::READWRITE, pErr)) {
		return false;
	}

	std::vector<BYTE> data = Str::serializeUtf8(text);
	if (!fout.write(data, pErr)) {
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool FileText::writeUtf8(const wstring& text, const wstring& path, wstring *pErr)
{
	return writeUtf8(text, path.c_str(), pErr);
}
