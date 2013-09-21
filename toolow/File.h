//
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "String.h"

//__________________________________________________________________________________________________
// Automation to a HANDLE of a file.
//
class File {
public:
	File() : _hFile(0) { }
	~File() { close(); }

	enum Access { READONLY, READWRITE };

	HANDLE hFile() const { return _hFile; }
	void   close()       { if(_hFile) { ::CloseHandle(_hFile); _hFile = 0; } }
	int    size()        { return ::GetFileSize(_hFile, 0); }
	bool   open(const wchar_t *path, Access access, String *pErr=0);
	bool   setNewSize(int newSize, String *pErr=0);
	bool   truncate(String *pErr=0) { return setNewSize(0, pErr); }
	bool   getContent(Array<BYTE> *pBuf, String *pErr=0);
	bool   write(const Array<BYTE> *pData, String *pErr=0) { return write(&(*pData)[0], pData->size(), pErr); }
	bool   write(const BYTE *pData, int sz, String *pErr=0);
	bool   rewind(String *pErr=0);

	static bool IsDir(const wchar_t *path)  { return (::GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	static bool Exists(const wchar_t *path) { return ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES; }

private:
	HANDLE _hFile;
};

//__________________________________________________________________________________________________
// Automation to a memory-mapped file.
//
class FileMap {
public:
	FileMap() : _hMap(0), _pMem(0), _size(0) { }
	~FileMap() { close(); }

	void  close();
	bool  open(const wchar_t *path, File::Access access, String *pErr=0);
	int   size() const     { return _size; }
	BYTE* pMem() const     { return (BYTE*)_pMem; }
	BYTE* pPastMem() const { return pMem() + size(); }
	bool  setNewSize(int newSize, String *pErr=0);
	bool  getContent(Array<BYTE> *pBuf, int offset=0, int numBytes=-1, String *pErr=0);
	bool  getContent(String *pBuf, int offset=0, int numChars=-1, String *pErr=0);

private:
	File   _file;
	HANDLE _hMap;
	void  *_pMem;
	int    _size;
};

//__________________________________________________________________________________________________
// Automation to text files.
//
class FileText {
public:
	FileText() : _p(0), _idxLine(-1) { }
	
	bool load(const wchar_t *path, String *pErr=0);
	bool load(const FileMap *pfm);
	bool nextLine(String *pBuf);
	int  curLineIndex() const { return _idxLine; }
	void rewind()             { _p = _text.ptrAt(0); _idxLine = -1; }
	
	const String* text() const { return &_text; }
	static bool Write(const wchar_t *path, const wchar_t *data, String *pErr=0);
	
private:
	String   _text;
	wchar_t *_p;
	int      _idxLine;
};