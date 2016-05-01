
#include "Path.h"
#include "Str.h"
using std::initializer_list;
using std::wstring;

wstring& Path::trimBackslash(wstring& path)
{
	while (path.back() == L'\\') {
		path.resize(path.size() - 1);
	}
	return path;
}

bool Path::isSame(const wstring& path, const wchar_t* other)
{
	return Str::eqI(path, other);
}

bool Path::hasExtension(const wstring& path, const wchar_t *extension)
{
	return Str::endsWithI(path, extension);
}

bool Path::hasExtension(const wstring& path, initializer_list<const wchar_t*> extensions)
{
	for (const wchar_t *ext : extensions) {
		if (Path::hasExtension(path, ext)) {
			return true;
		}
	}
	return false;
}

wstring& Path::changeExtension(wstring& path, const wchar_t *newExtension)
{
	size_t dotIdx = path.find_last_of(L'.');
	if (dotIdx != wstring::npos) {
		path.resize(dotIdx + 1); // truncate after the dot
	} else {
		path.append(1, L'.');
	}
	path.append(newExtension[0] == L'.' ? newExtension + 1 : newExtension);
	return path;
}

wstring Path::folderFrom(const wstring& path)
{
	wstring ret(path);
	ret.resize(ret.find_last_of(L'\\')); // also remove trailing backslash
	return ret;
}

wstring Path::fileFrom(const wstring& path)
{
	wstring ret(path);
	ret.erase(0, ret.find_last_of(L'\\') + 1);
	return ret;
}