//
// In-memory loading of XML files using MSXML2 library.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "Hash.h"
#include "Ptr.h"

class Xml final {
public:
	class Node final {
	public:
		String       name;
		String       value;
		Hash<String> attrs;
		Array<Node>  children;

		Node()                  { }
		Node(const Node& other) : name(other.name), value(other.value), attrs(other.attrs), children(other.children) { }
		Node(Node&& other)      : name(MOVE(other.name)), value(MOVE(other.value)), attrs(MOVE(other.attrs)), children(MOVE(other.children)) { }

		Node& operator=(const Node& other) { name = other.name; value = other.value; attrs = other.attrs; children = other.children; return *this; }
		Node& operator=(Node&& other)      { name = MOVE(other.name); value = MOVE(other.value); attrs = MOVE(other.attrs); children = MOVE(other.children); return *this; }
		Array<Node*> getChildrenByName(const wchar_t *elemName);
		Node*        firstChildByName(const wchar_t *elemName);
	};

public:
	Node root;

	Xml()                   { }
	Xml(const Xml& other)   : root(other.root) { }
	Xml(Xml&& other)        : root(MOVE(other.root)) { }
	Xml(const wchar_t *str) { parse(str); }
	Xml(const String& str)  { parse(str); }

	Xml& operator=(const Xml& other) { root = other.root; return *this; }
	Xml& operator=(Xml&& other)      { root = MOVE(other.root); return *this; }
	bool parse(const wchar_t *str);
	bool parse(const String& str)    { return parse(str.str()); }
	bool load(const wchar_t *file, String *pErr=0);
};