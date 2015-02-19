/*!
 * OS-related stuff.
 * Part of OWL - Object Win32 Library.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>

// WinMain automation for *App classes.
#include <crtdbg.h>
#define RUN(AppClass) \
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE h0, LPWSTR cmdLine, int cmdShow) { \
	int ret = 0; \
	{	AppClass wnd; \
		ret = wnd.run(hInst, cmdShow); } \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; }


namespace owl {

struct System final {
	enum class Color { BUTTON=COLOR_BTNFACE, DESKTOP=COLOR_DESKTOP,
		BLACK=COLOR_BTNTEXT, WHITE=COLOR_WINDOW, GREY=COLOR_APPWORKSPACE };
	enum class Cursor { ARROW=32512, IBEAM=32513, CROSS=32515, HAND=32649, NO=32648,
		SIZEALL=32646, SIZENESW=32643, SIZENS=32645, SIZENWSE=32642, SIZEWE=32644 };

	inline static bool HasCtrl()       { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	inline static bool HasShift()      { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
	inline static SIZE GetScreenSize() { return { ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) }; }
	static void  Thread(std::function<void()> callback);
	static DWORD Exec(const wchar_t *cmdLine);
	static DWORD Exec(const std::wstring& cmdLine) { return Exec(cmdLine.c_str()); }
	static void  PopMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo);
	static std::wstring GetExePath();
	static std::wstring GetDesktopPath();
	static std::wstring GetMyDocsPath();
	static std::wstring GetRoamingPath();
};


class Date final {
private:
	SYSTEMTIME _st;
public:
	Date()                              { setNow(); }
	explicit Date(LONGLONG ms)          { setFromMs(ms); }
	explicit Date(const SYSTEMTIME& st) { setFromSt(st); }
	explicit Date(const FILETIME& ft)   { setFromFt(ft); }
	Date& setNow();
	Date& setFromSt(const SYSTEMTIME& st) { ::memcpy(&_st, &st, sizeof(SYSTEMTIME)); return *this; }
	Date& setFromMs(LONGLONG ms);
	Date& setFromFt(const FILETIME& ft);
	const SYSTEMTIME& get() const { return _st; }
	LONGLONG          getTimestamp() const;
	LONGLONG          minus(const Date& other) const;
	Date&             addMs(LONGLONG ms);
	Date&             addSec(LONGLONG sec) { return addMs(sec * 1000); }
	Date&             addMin(LONGLONG min) { return addSec(min * 60); }
	Date&             addHour(LONGLONG h)  { return addMin(h * 60); }
	Date&             addDay(LONGLONG d)   { return addHour(d * 24); }
private:
	static void _StToLi(const SYSTEMTIME& st, LARGE_INTEGER& li);
	static void _LiToSt(const LARGE_INTEGER& li, SYSTEMTIME& st);
};


class Font final {
public:
	struct Info final {
		wchar_t name[LF_FACESIZE];
		int     size;
		bool    bold;
		bool    italic;

		Info() : size(0), bold(false), italic(false) { *name = L'\0'; }
		Info(const Info& other) { operator=(other); }
		Info& operator=(const Info& other) {
			size = other.size;
			bold = other.bold;
			italic = other.italic;
			lstrcpy(name, other.name);
			return *this;
		}
	};
private:
	HFONT _hFont;
public:
	Font()                  : _hFont(nullptr) { }
	Font(HFONT hfont)       : _hFont(hfont)   { }
	Font(const Font& other) : Font() { operator=(other); }
	Font(Font&& other)      : Font() { operator=(std::move(other)); }
	~Font()                 { release(); }

	Font& operator=(HFONT hfont)       { release(); _hFont = hfont; return *this; }
	Font& operator=(const Font& other) { release(); cloneFrom(other); return *this; }
	Font& operator=(Font&& other)      { _hFont = other._hFont; other.release(); return *this; }

	HFONT hFont() const            { return _hFont; }
	void  release()                { if (_hFont) { ::DeleteObject(_hFont); _hFont = nullptr; } }
	Font& create(const wchar_t *name, int size, bool bold=false, bool italic=false);
	Font& create(const Info& info) { return create(info.name, info.size, info.bold, info.italic); }
	Font& cloneFrom(const Font& font);
	Info  getInfo() const;
	Font& apply(HWND hWnd);
	Font& applyOnChildren(HWND hWnd);

	static bool Exists(const wchar_t *name);
	static Info GetDefaultDialogFontInfo();
private:
	static void _LogfontToInfo(const LOGFONT& lf, Info& info);
};


class Icon final {
private:
	HICON _hIcon;
public:
	Icon()  : _hIcon(nullptr) { }
	~Icon() { this->free(); }

	HICON hIcon() const          { return _hIcon; }
	Icon& free()                 { if (_hIcon) ::DestroyIcon(_hIcon); return *this; }
	Icon& operator=(HICON hIcon) { _hIcon = hIcon; return *this; }
	Icon& getFromExplorer(const wchar_t *fileExtension);
	Icon& getFromResource(int iconId, int size, HINSTANCE hInst=nullptr);

	static void IconToLabel(HWND hStatic, int idIconRes, BYTE size);
};


class Xml final {
public:
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

}//namespace owl