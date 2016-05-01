
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class Xml final {
public:
	class Node final {
	public:
		std::wstring name;
		std::wstring value;
		std::unordered_map<std::wstring, std::wstring> attrs;
		std::vector<Node> children;

		std::vector<Node*> getChildrenByName(const wchar_t *elemName);
		std::vector<Node*> getChildrenByName(const std::wstring& elemName) { return getChildrenByName(elemName.c_str()); }
		Node* firstChildByName(const wchar_t *elemName);
		Node* firstChildByName(const std::wstring& elemName) { return firstChildByName(elemName.c_str()); }
	};
public:
	Node root;

	Xml() = default;
	Xml(const Xml& other)        : root(other.root) { }
	Xml(Xml&& other)             : root(std::move(other.root)) { }
	Xml(const wchar_t *str)      : Xml() { parse(str); }
	Xml(const std::wstring& str) : Xml(str.c_str()) { }
	Xml& operator=(const Xml& other);
	Xml& operator=(Xml&& other);
	bool parse(const wchar_t *str);
	bool parse(const std::wstring& str) { return parse(str.c_str()); }
};