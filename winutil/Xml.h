
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
		Node* firstChildByName(const wchar_t *elemName);
	};
public:
	Node root;

	Xml();
	Xml(const Xml& other);
	Xml(Xml&& other);
	Xml(const wchar_t *str);
	Xml(const std::wstring& str);
	Xml& operator=(const Xml& other);
	Xml& operator=(Xml&& other);
	bool parse(const wchar_t *str);
	bool parse(const std::wstring& str);
};