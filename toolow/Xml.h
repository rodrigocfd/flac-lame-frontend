//
// In-memory loading of XML files using MSXML2 library.
// Evening of Sunday, September 8, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Ptr.h"
#include "Hash.h"
#include <MsXml2.h>

class Xml {
public:
	class Node {
	public:
		String name;
		String value;
		Hash<String> attrs;
		Array<Node> children;
		void getChildrenByName(const wchar_t *elemName, Array<Node*> *nodeBuf);
		Node* firstChildByName(const wchar_t *elemName);
	};
	
	Node root;
	bool parse(const wchar_t *str);
	bool load(const wchar_t *file, String *pErr=0);

private:
	static void _BuildNode(ComPtr<IXMLDOMNode> elem, Xml::Node *nodebuf);
	static void _ReadAttrs(ComPtr<IXMLDOMNode> elem, Hash<String> *attrbuf);
	static int  _CountChildNodes(ComPtr<IXMLDOMNodeList> nodeList);
};