//
// File handling.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "Date.h"
#include "Hash.h"

namespace File
{
	inline bool   Exists(const wchar_t *path)                      { return ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES; }
	inline bool   Exists(const String& path)                       { return Exists(path.str()); }
	inline bool   IsDir(const wchar_t *path)                       { return (::GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	inline bool   IsDir(const String& path)                        { return IsDir(path.str()); }
	bool          Delete(const wchar_t *path, String *pErr=nullptr);
	inline bool   Delete(const String& path, String *pErr=nullptr) { return Delete(path.str(), pErr); }
	bool          CreateDir(const wchar_t *path);
	inline bool   CreateDir(const String& path)                    { return CreateDir(path.str()); }
	Date          DateLastModified(const wchar_t *path);
	inline Date   DateLastModified(const String& path)             { return DateLastModified(path.str()); }
	Date          DateCreated(const wchar_t *path);
	inline Date   DateCreated(const String& path)                  { return DateCreated(path.str()); }
	bool          WriteUtf8(const wchar_t *path, const wchar_t *data, String *pErr=nullptr);
	bool          Unzip(const wchar_t *zip, const wchar_t *destFolder, String *pErr=nullptr);
	inline bool   Unzip(const String& zip, const String& destFolder, String *pErr=nullptr) { return Unzip(zip.str(), destFolder.str(), pErr); }
	int           IndexOfBin(const BYTE *pData, int dataLen, const wchar_t *what, bool asWideChar);

	class Path final { // path string utilities
	public:
		inline static void   ChangeExtension(String& path, const wchar_t *extWithoutDot) { path[path.findrCS(L'.') + 1] = L'\0'; path.append(extWithoutDot); }
		inline static void   TrimBackslash(String& path)             { if(!path.isEmpty() && path.endsWithCS(L'\\')) path[path.len() - 1] = L'\0'; }
		inline static String GetPath(const wchar_t *path)            { String ret = path; ret[ ret.findrCS(L'\\') ] = L'\0'; return ret; }
		inline static String GetPath(const String& path)             { return GetPath(path.str()); }
		inline static const wchar_t* GetFilename(const String& path) { return path.ptrAt(path.findrCS(L'\\') + 1); }
	};

	enum class Access { READONLY, READWRITE };

	class Raw final { // automation to a HANDLE of a file
	private:
		HANDLE _hFile;
	public:
		Raw()  : _hFile(nullptr) { }
		~Raw() { close(); }

		HANDLE hFile() const { return _hFile; }
		void   close()       { if(_hFile) { ::CloseHandle(_hFile); _hFile = nullptr; } }
		int    size() const  { return ::GetFileSize(_hFile, nullptr); }
		bool   open(const wchar_t *path, Access access, String *pErr=nullptr);
		bool   open(const String& path, Access access, String *pErr=nullptr) { return open(path.str(), access, pErr); }
		bool   setNewSize(int newSize, String *pErr=nullptr);
		bool   truncate(String *pErr=nullptr)                                { return setNewSize(0, pErr); }
		bool   getContent(Array<BYTE> *pBuf, String *pErr=nullptr) const;
		bool   write(const Array<BYTE>& data, String *pErr=nullptr)          { return write(&data[0], data.size(), pErr); }
		bool   write(const BYTE *pData, int sz, String *pErr=nullptr);
		bool   rewind(String *pErr=nullptr);
	};

	class Mapped final { // automation to a memory-mapped file
	private:
		Raw    _file;
		HANDLE _hMap;
		void  *_pMem;
		int    _size;
	public:
		Mapped() : _hMap(nullptr), _pMem(nullptr), _size(0) { }
		~Mapped() { close(); }

		void  close();
		bool  open(const wchar_t *path, Access access, String *pErr=nullptr);
		bool  open(const String& path, Access access, String *pErr=nullptr) { return open(path.str(), access, pErr); }
		int   size() const     { return _size; }
		BYTE* pMem() const     { return (BYTE*)_pMem; }
		BYTE* pPastMem() const { return pMem() + size(); }
		bool  setNewSize(int newSize, String *pErr=nullptr);
		bool  getContent(Array<BYTE> *pBuf, int offset=0, int numBytes=-1, String *pErr=nullptr) const;
		bool  getContent(String *pBuf, int offset=0, int numChars=-1, String *pErr=nullptr) const;
	};

	class Text final { // automation to text files
	private:
		String   _text;
		wchar_t *_p;
		int      _idxLine;
	public:
		Text() : _p(nullptr), _idxLine(-1) { }
		
		bool          load(const wchar_t *path, String *pErr=nullptr);
		bool          load(const String& path, String *pErr=nullptr) { return load(path.str(), pErr); }
		bool          load(const Mapped *pfm);
		bool          nextLine(String *pBuf);
		int           curLineIndex() const { return _idxLine; }
		void          rewind()             { _p = _text.ptrAt(0); _idxLine = -1; }
		const String* text() const         { return &_text; }
	};

	class Ini final { // automation to INI files, loading into a hash
	private:
		String _path;
		int    _countSections(File::Text *fin) const;
	public:
		Hash<Hash<String>> sections;
		Hash<String>& operator[](const wchar_t *key) { return sections[key]; }
		Hash<String>& operator[](const String& key)  { return sections[key]; }

		Ini& setPath(const wchar_t *iniPath) { _path = iniPath; return *this; }
		Ini& setPath(const String& iniPath)  { return setPath(iniPath.str()); }
		bool load(String *pErr=nullptr);
		bool serialize(String *pErr=nullptr) const;
	};

	class Listing final { // automation for directory enumeration
	private:
		HANDLE          _hFind;
		WIN32_FIND_DATA _wfd;
		wchar_t        *_pattern;
	public:
		explicit Listing(const wchar_t *pattern);
		Listing(const wchar_t *path, const wchar_t *pattern);
		Listing(const String& path, const wchar_t *pattern) : Listing(path.str(), pattern) { }
		~Listing();

		wchar_t* next(wchar_t *buf);
		String*  next(String *pBuf);
	};
}