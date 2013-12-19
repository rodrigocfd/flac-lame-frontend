//
// In-memory loading of XML files using MSXML2 library.
// Evening of Sunday, September 8, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Hash.h"
#include "ComPtr.h"
#include <MsXml2.h>

class Xml {
public:
	class Node {
	public:
		String       name;
		String       value;
		Hash<String> attrs;
		Array<Node>  children;

		Node()                             { }
		Node(const Node& other)            : name(other.name), value(other.value), attrs(other.attrs), children(other.children) { }
		Node(Node&& other)                 : name((String&&)other.name), value((String&&)other.value), attrs((Hash<String>&&)other.attrs), children((Array<Node>&&)other.children) { }
		Node& operator=(const Node& other) { name = other.name; value = other.value; attrs = other.attrs; children = other.children; return *this; }	
		Node& operator=(Node&& other)      { name = (String&&)other.name; value = (String&&)other; attrs = (Hash<String>&&)other.attrs; children = (Array<Node>&&)other.children; return *this; }	
		Array<Node*> getChildrenByName(const wchar_t *elemName, String::Case sens=String::Case::SENS);
		Node*        firstChildByName(const wchar_t *elemName, String::Case sens=String::Case::SENS);
	};
	
public:
	Node root;

	Xml()                            { }
	Xml(const Xml& other)            : root(other.root) { }
	Xml(Xml&& other)                 : root((Node&&)other.root) { }
	Xml(const wchar_t *str)          { parse(str); }
	Xml& operator=(const Xml& other) { root = other.root; return *this; }
	Xml& operator=(Xml&& other)      { root = (Node&&)other.root; return *this; }
	bool parse(const wchar_t *str);
	bool load(const wchar_t *file, String *pErr=0);

private:
	static void _BuildNode(ComPtr<IXMLDOMNode> elem, Xml::Node *nodebuf);
	static void _ReadAttrs(ComPtr<IXMLDOMNode> elem, Hash<String> *attrbuf);
	static int  _CountChildNodes(ComPtr<IXMLDOMNodeList> nodeList);
};