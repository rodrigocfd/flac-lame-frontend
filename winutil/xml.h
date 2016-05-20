/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace winutil {

class xml final {
public:
	class node final {
	public:
		std::wstring name;
		std::wstring value;
		std::unordered_map<std::wstring, std::wstring> attrs;
		std::vector<node> children;

		std::vector<node*> children_by_name(const wchar_t* elemName);
		std::vector<node*> children_by_name(const std::wstring& elemName) { return children_by_name(elemName.c_str()); }
		node* first_child_by_name(const wchar_t* elemName);
		node* first_child_by_name(const std::wstring& elemName) { return first_child_by_name(elemName.c_str()); }
	};
public:
	node root;

	xml() = default;
	xml(const xml& other)        : root(other.root) { }
	xml(xml&& other)             : root(std::move(other.root)) { }
	xml(const wchar_t* str)      : xml() { parse(str); }
	xml(const std::wstring& str) : xml(str.c_str()) { }
	xml& operator=(const xml& other);
	xml& operator=(xml&& other);
	bool parse(const wchar_t* str);
	bool parse(const std::wstring& str) { return parse(str.c_str()); }
};

}//namespace winutil