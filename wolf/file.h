/*!
 * @file
 * @brief Automation for file handling.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <unordered_map>
#include "res.h"

namespace wolf {
namespace file {

inline bool Exists(const wchar_t *path)                { return ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES; }
inline bool Exists(const std::wstring& path)           { return Exists(path.c_str()); }
inline bool IsDir(const wchar_t *path)                 { return (::GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0; }
inline bool IsDir(const std::wstring& path)            { return IsDir(path.c_str()); }
bool        Delete(const wchar_t *path, std::wstring *pErr=nullptr);
inline bool Delete(const std::wstring& path, std::wstring *pErr=nullptr) { return Delete(path.c_str(), pErr); }
bool        CreateDir(const wchar_t *path);
inline bool CreateDir(const std::wstring& path)        { return CreateDir(path.c_str()); }
res::Date        DateLastModified(const wchar_t *path);
inline res::Date DateLastModified(const std::wstring& path) { return DateLastModified(path.c_str()); }
res::Date        DateCreated(const wchar_t *path);
inline res::Date DateCreated(const std::wstring& path)      { return DateCreated(path.c_str()); }
bool         WriteUtf8(const wchar_t *path, const wchar_t *data, std::wstring *pErr=nullptr);
inline bool  WriteUtf8(const wchar_t *path, const std::wstring& data, std::wstring *pErr=nullptr) { return WriteUtf8(path, data.c_str(), pErr); }
bool         Unzip(const wchar_t *zip, const wchar_t *destFolder, std::wstring *pErr=nullptr);
inline bool  Unzip(const std::wstring& zip, const std::wstring& destFolder, std::wstring *pErr=nullptr) { return Unzip(zip.c_str(), destFolder.c_str(), pErr); }
int          IndexOfBin(const BYTE *pData, size_t dataLen, const wchar_t *what, bool asWideChar);

namespace path {
	void                ChangeExtension(std::wstring& sPath, const wchar_t *extWithoutDot);
	void                TrimBackslash(std::wstring& sPath);
	std::wstring        GetPath(const wchar_t *sPath);
	inline std::wstring GetPath(const std::wstring& sPath) { return GetPath(sPath.c_str()); }
	std::wstring        GetFilename(const std::wstring& sPath);
}

enum class Access { READONLY, READWRITE };

/// File HANDLE wrapper.
class Raw final {
private:
	HANDLE _hFile;
	Access _access;
public:
	Raw()  : _hFile(nullptr), _access(Access::READONLY) { }
	~Raw() { close(); }

	HANDLE hFile() const     { return _hFile; }
	Access getAccess() const { return _access; }
	void   close();
	size_t size() const      { return ::GetFileSize(_hFile, nullptr); }
	bool   open(const wchar_t *path, Access access, std::wstring *pErr=nullptr);
	bool   open(const std::wstring& path, Access access, std::wstring *pErr=nullptr) { return open(path.c_str(), access, pErr); }
	bool   setNewSize(size_t newSize, std::wstring *pErr=nullptr);
	bool   truncate(std::wstring *pErr=nullptr)                                      { return setNewSize(0, pErr); }
	bool   getContent(std::vector<BYTE>& buf, std::wstring *pErr=nullptr) const;
	bool   write(const BYTE *pData, size_t sz, std::wstring *pErr=nullptr);
	bool   write(const std::vector<BYTE>& data, std::wstring *pErr=nullptr)          { return write(&data[0], data.size(), pErr); }
	bool   rewind(std::wstring *pErr=nullptr);
};

/// Automation to a memory-mapped file.
class Mapped final {
private:
	Raw    _file;
	HANDLE _hMap;
	void  *_pMem;
	size_t _size;
public:
	Mapped() : _hMap(nullptr), _pMem(nullptr), _size(0) { }
	~Mapped() { close(); }

	Access getAccess() const { return _file.getAccess(); }
	void   close();
	bool   open(const wchar_t *path, Access access, std::wstring *pErr=nullptr);
	bool   open(const std::wstring& path, Access access, std::wstring *pErr=nullptr) { return open(path.c_str(), access, pErr); }
	size_t size() const     { return _size; }
	BYTE*  pMem() const     { return (BYTE*)_pMem; }
	BYTE*  pPastMem() const { return pMem() + size(); }
	bool   setNewSize(size_t newSize, std::wstring *pErr=nullptr);
	bool   getContent(std::vector<BYTE>& buf, int offset=0, int numBytes=-1, std::wstring *pErr=nullptr) const;
	bool   getContent(std::wstring& buf, int offset=0, int numChars=-1, std::wstring *pErr=nullptr) const;
};

/// Automation to read text files line-by-line.
class Text final {
private:
	std::wstring _text;
	wchar_t     *_p;
	int          _idxLine;
public:
	Text() : _p(nullptr), _idxLine(-1) { }

	bool load(const wchar_t *path, std::wstring *pErr=nullptr);
	bool load(const std::wstring& path, std::wstring *pErr=nullptr) { return load(path.c_str(), pErr); }
	bool load(const Mapped& fm);
	bool nextLine(std::wstring& buf);
	int  curLineIndex() const        { return _idxLine; }
	void rewind()                    { _p = &_text[0]; _idxLine = -1; }
	const std::wstring& text() const { return _text; }
};

/// Automation to INI files, loaded into two nested associative arrays.
class Ini final {
private:
	std::wstring _path;
public:
	std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::wstring>> sections;
	Ini& setPath(const wchar_t *iniPath)      { _path = iniPath; return *this; }
	Ini& setPath(const std::wstring& iniPath) { return setPath(iniPath.c_str()); }
	const std::wstring& getPath() const       { return _path; }
	bool load(std::wstring *pErr=nullptr);
	bool serialize(std::wstring *pErr=nullptr) const;
private:
	int _countSections(Text *fin) const;
};

/// XML parsing and manipulation.
class Xml final {
public:
	/// A single XML node.
	class Node final {
	public:
		std::wstring name;
		std::wstring value;
		std::unordered_map<std::wstring, std::wstring> attrs;
		std::vector<Node> children;

		std::vector<Node*> getChildrenByName(const wchar_t *elemName);
		Node* firstChildByName(const wchar_t *elemName);
	};
public:
	Node root;

	Xml()                        { }
	Xml(const Xml& other)        : root(other.root) { }
	Xml(Xml&& other)             : root(std::move(other.root)) { }
	Xml(const wchar_t *str)      { parse(str); }
	Xml(const std::wstring& str) { parse(str); }

	Xml& operator=(const Xml& other)    { root = other.root; return *this; }
	Xml& operator=(Xml&& other)         { root = std::move(other.root); return *this; }
	bool parse(const wchar_t *str);
	bool parse(const std::wstring& str) { return parse(str.c_str()); }
};

/// Automation for directory enumeration.
class Listing final {
private:
	HANDLE          _hFind;
	WIN32_FIND_DATA _wfd;
	std::wstring    _pattern;
public:
	explicit Listing(const wchar_t *pattern);
	Listing(const wchar_t *path, const wchar_t *pattern);
	Listing(const std::wstring& path, const wchar_t *pattern) : Listing(path.c_str(), pattern) { }
	~Listing();
	bool next(std::wstring& buf);
		
	static std::vector<std::wstring> GetAll(const wchar_t *pattern);
	static std::vector<std::wstring> GetAll(const wchar_t *path, const wchar_t *pattern);
	static std::vector<std::wstring> GetAll(const std::wstring& path, const wchar_t *pattern) { return GetAll(path.c_str(), pattern); }
};

}//namespace file
}//namespace wolf