//
// File handling.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
//

#pragma once
#include "System.h"
#include "DataStructs.h"

struct File final {
	static inline bool  Exists(const wchar_t *path)                      { return ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES; }
	static inline bool  Exists(const String& path)                       { return Exists(path.str()); }
	static inline bool  IsDir(const wchar_t *path)                       { return (::GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	static inline bool  IsDir(const String& path)                        { return IsDir(path.str()); }
	static bool         Delete(const wchar_t *path, String *pErr=nullptr);
	static inline bool  Delete(const String& path, String *pErr=nullptr) { return Delete(path.str(), pErr); }
	static bool         CreateDir(const wchar_t *path);
	static inline bool  CreateDir(const String& path)                    { return CreateDir(path.str()); }
	static System::Date DateLastModified(const wchar_t *path);
	static inline System::Date DateLastModified(const String& path)      { return DateLastModified(path.str()); }
	static System::Date DateCreated(const wchar_t *path);
	static inline System::Date DateCreated(const String& path)           { return DateCreated(path.str()); }
	static bool         WriteUtf8(const wchar_t *path, const wchar_t *data, String *pErr=nullptr);
	static bool         Unzip(const wchar_t *zip, const wchar_t *destFolder, String *pErr=nullptr);
	static inline bool  Unzip(const String& zip, const String& destFolder, String *pErr=nullptr) { return Unzip(zip.str(), destFolder.str(), pErr); }
	static int          IndexOfBin(const BYTE *pData, int dataLen, const wchar_t *what, bool asWideChar);

	class Path final { // path string utilities
	public:
		inline static void   ChangeExtension(String& path, const wchar_t *extWithoutDot) { path[path.findrCS(L'.') + 1] = L'\0'; path.append(extWithoutDot); }
		inline static void   TrimBackslash(String& path)             { if (!path.isEmpty() && path.endsWithCS(L'\\')) path[path.len() - 1] = L'\0'; }
		inline static String GetPath(const wchar_t *path)            { String ret = path; ret[ ret.findrCS(L'\\') ] = L'\0'; return ret; }
		inline static String GetPath(const String& path)             { return GetPath(path.str()); }
		inline static const wchar_t* GetFilename(const String& path) { return path.ptrAt(path.findrCS(L'\\') + 1); }
	};

	enum class Access { READONLY, READWRITE };

	class Raw final { // automation to a HANDLE of a file
	private:
		HANDLE _hFile;
		Access _access;
	public:
		Raw()  : _hFile(nullptr), _access(Access::READONLY) { }
		~Raw() { close(); }

		HANDLE hFile() const { return _hFile; }
		Access getAccess() const { return _access; }
		void   close();
		int    size() const  { return ::GetFileSize(_hFile, nullptr); }
		bool   open(const wchar_t *path, Access access, String *pErr=nullptr);
		bool   open(const String& path, Access access, String *pErr=nullptr) { return open(path.str(), access, pErr); }
		bool   setNewSize(int newSize, String *pErr=nullptr);
		bool   truncate(String *pErr=nullptr)                                { return setNewSize(0, pErr); }
		bool   getContent(Array<BYTE>& buf, String *pErr=nullptr) const;
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

		Access getAccess() const { return _file.getAccess(); }
		void  close();
		bool  open(const wchar_t *path, Access access, String *pErr=nullptr);
		bool  open(const String& path, Access access, String *pErr=nullptr) { return open(path.str(), access, pErr); }
		int   size() const     { return _size; }
		BYTE* pMem() const     { return (BYTE*)_pMem; }
		BYTE* pPastMem() const { return pMem() + size(); }
		bool  setNewSize(int newSize, String *pErr=nullptr);
		bool  getContent(Array<BYTE>& buf, int offset=0, int numBytes=-1, String *pErr=nullptr) const;
		bool  getContent(String& buf, int offset=0, int numChars=-1, String *pErr=nullptr) const;
	};

	class Text final { // automation to read text files line-by-line
	private:
		String   _text;
		wchar_t *_p;
		int      _idxLine;
	public:
		Text() : _p(nullptr), _idxLine(-1) { }

		bool          load(const wchar_t *path, String *pErr=nullptr);
		bool          load(const String& path, String *pErr=nullptr) { return load(path.str(), pErr); }
		bool          load(const Mapped& fm);
		bool          nextLine(String& buf);
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

		bool next(wchar_t *buf);
		bool next(String& buf);
	};
};