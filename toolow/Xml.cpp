//
// In-memory loading of XML files using MSXML2 library.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "Xml.h"
#include "File.h"
#include <MsXml2.h>
#pragma comment(lib, "msxml2.lib")

static void _ReadAttrs(ComPtr<IXMLDOMNode> xmlnode, Hash<String>& attrbuf)
{
	// Read attribute collection.
	ComPtr<IXMLDOMNamedNodeMap> attrs;
	xmlnode->get_attributes(&attrs);
	
	long attrCount = 0;
	attrs->get_length(&attrCount);
	attrbuf.reserve(attrCount);

	for (long i = 0; i < attrCount; ++i) {
		ComPtr<IXMLDOMNode> attr;
		attrs->get_item(i, &attr);

		DOMNodeType type;
		attr->get_nodeType(&type);
		if (type == NODE_ATTRIBUTE) {
			BSTR bstr;
			attr->get_nodeName(&bstr); // get attribute name

			VARIANT var = { 0 };
			attr->get_nodeValue(&var); // get attribute value
			
			attrbuf[(wchar_t*)bstr] = (wchar_t*)var.bstrVal; // add hash entry
			SysFreeString(bstr);
			VariantClear(&var);
		}
	}
}

static int _CountChildNodes(ComPtr<IXMLDOMNodeList> nodeList)
{
	int childCount = 0;
	long totalCount = 0;
	nodeList->get_length(&totalCount); // includes text and actual element nodes
	
	for (long i = 0; i < totalCount; ++i) {
		ComPtr<IXMLDOMNode> child;
		nodeList->get_item(i, &child);

		DOMNodeType type;
		child->get_nodeType(&type);
		if (type == NODE_ELEMENT)
			++childCount;
	}

	return childCount;
}

static void _BuildNode(ComPtr<IXMLDOMNode> xmlnode, Xml::Node& nodebuf)
{
	// Get node name.
	BSTR bstr;
	xmlnode->get_nodeName(&bstr);
	nodebuf.name = (wchar_t*)bstr;
	SysFreeString(bstr);

	// Parse attributes of node, if any.
	_ReadAttrs(xmlnode, nodebuf.attrs);

	// Process children, if any.
	VARIANT_BOOL vb;
	xmlnode->hasChildNodes(&vb);
	if (vb) {
		ComPtr<IXMLDOMNodeList> nodeList;
		xmlnode->get_childNodes(&nodeList);
		nodebuf.children.resize(_CountChildNodes(nodeList));

		int childCount = 0;
		long totalCount = 0;
		nodeList->get_length(&totalCount);

		for (long i = 0; i < totalCount; ++i) {
			ComPtr<IXMLDOMNode> child;
			nodeList->get_item(i, &child);

			// Node can be text or an actual child node.
			DOMNodeType type;
			child->get_nodeType(&type);
			if (type == NODE_TEXT) {
				xmlnode->get_text(&bstr);
				nodebuf.value.append((wchar_t*)bstr);
				SysFreeString(bstr);
			} else if (type == NODE_ELEMENT) {
				_BuildNode(child, nodebuf.children[childCount++]); // recursively
			} else {
				// (L"Unhandled node type: %d.\n", type);
			}
		}
	} else {
		// Assumes that only a leaf node can have text.
		xmlnode->get_text(&bstr);
		nodebuf.value = (wchar_t*)bstr;
		SysFreeString(bstr);
	}
}


Array<Xml::Node*> Xml::Node::getChildrenByName(const wchar_t *elemName)
{
	int howMany = 0;
	int firstIndex = -1, lastIndex = -1;
	for (int i = 0; i < this->children.size(); ++i) {
		if (this->children[i].name.equalsCI(elemName)) { // case-insensitive match
			++howMany;
			if (firstIndex == -1) firstIndex = i;
			lastIndex = i;
		}
	}

	Array<Node*> nodeBuf(howMany); // alloc return array

	howMany = 0;
	for (int i = firstIndex; i <= lastIndex; ++i)
		if (this->children[i].name.equalsCI(elemName))
			nodeBuf[howMany++] = &this->children[i];
	
	return nodeBuf;
}

Xml::Node* Xml::Node::firstChildByName(const wchar_t *elemName)
{
	int iChild = -1;
	for (int i = 0; i < this->children.size(); ++i) {
		if (this->children[i].name.equalsCI(elemName)) { // case-insensitive match
			iChild = i;
			break;
		}
	}
	return iChild == -1 ? nullptr : &this->children[iChild];
}

bool Xml::parse(const wchar_t *str)
{
	CoInitialize(nullptr); // http://stackoverflow.com/questions/7824383/double-calls-to-coinitialize
	
	// Create COM object for XML document.
	ComPtr<IXMLDOMDocument2> doc;
	doc.coCreateInstance(CLSID_DOMDocument30, IID_IXMLDOMDocument);
	doc->put_async(FALSE);

	// Parse the XML string.
	VARIANT_BOOL vb;
	doc->loadXML((BSTR)str, &vb);

	// Get document element and root node from XML.
	ComPtr<IXMLDOMElement> docElem;
	doc->get_documentElement(&docElem);

	ComPtr<IXMLDOMNode> rootNode;
	docElem.queryInterface(IID_IXMLDOMNode, &rootNode);
	_BuildNode(rootNode, this->root); // recursive

	rootNode.release(); // must be released before CoUninitialize
	docElem.release();
	doc.release();
	CoUninitialize();
	return true;
}

bool Xml::load(const wchar_t *file, String *pErr)
{
	File::Text fin;
	if (!fin.load(file, pErr)) return false;
	return this->parse(fin.text()->str());
}