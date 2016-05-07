/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include <Windows.h>
#include <MsXml2.h>
#pragma comment(lib, "msxml2.lib")
#include "xml.h"
using namespace winutil;
using std::unordered_map;
using std::vector;
using std::wstring;

vector<xml::node*> xml::node::children_by_name(const wchar_t *elemName)
{
	int howMany = 0;
	size_t firstIndex = -1, lastIndex = -1;
	for (size_t i = 0; i < children.size(); ++i) {
		if (!lstrcmpi(children[i].name.c_str(), elemName)) { // case-insensitive match
			++howMany;
			if (firstIndex == -1) firstIndex = i;
			lastIndex = i;
		}
	}

	vector<node*> nodeBuf;
	nodeBuf.reserve(howMany); // alloc return array

	howMany = 0;
	for (size_t i = firstIndex; i <= lastIndex; ++i) {
		if (!lstrcmpi(children[i].name.c_str(), elemName)) {
			nodeBuf.emplace_back(&children[i]);
		}
	}
	return nodeBuf;
}

xml::node* xml::node::first_child_by_name(const wchar_t *elemName)
{
	for (node& node : children) {
		if (!lstrcmpi(node.name.c_str(), elemName)) { // case-insensitive match
			return &node;
		}
	}
	return nullptr; // not found
}


xml& xml::operator=(const xml& other)
{
	root = other.root;
	return *this;
}

xml& xml::operator=(xml&& other)
{
	root = std::move(other.root);
	return *this;
}

static void _readAttrs(IXMLDOMNode *xmlnode, unordered_map<wstring, wstring>& attrbuf)
{
	// Read attribute collection.
	IXMLDOMNamedNodeMap *attrs = nullptr;
	xmlnode->get_attributes(&attrs);

	long attrCount = 0;
	attrs->get_length(&attrCount);
	attrbuf.clear();
	attrbuf.reserve(attrCount);

	for (long i = 0; i < attrCount; ++i) {
		IXMLDOMNode *attr = nullptr;
		attrs->get_item(i, &attr);

		DOMNodeType type = NODE_INVALID;
		attr->get_nodeType(&type);
		if (type == NODE_ATTRIBUTE) {
			BSTR bstr = nullptr;
			attr->get_nodeName(&bstr); // get attribute name

			VARIANT var = { 0 };
			attr->get_nodeValue(&var); // get attribute value

			attrbuf.emplace(static_cast<wchar_t*>(bstr), static_cast<wchar_t*>(var.bstrVal)); // add hash entry
			SysFreeString(bstr);
			VariantClear(&var);
		}
		attr->Release();
	}
	attrs->Release();
}

static int _countChildNodes(IXMLDOMNodeList *nodeList)
{
	int childCount = 0;
	long totalCount = 0;
	nodeList->get_length(&totalCount); // includes text and actual element nodes

	for (long i = 0; i < totalCount; ++i) {
		IXMLDOMNode *child = nullptr;
		nodeList->get_item(i, &child);

		DOMNodeType type = NODE_INVALID;
		child->get_nodeType(&type);
		if (type == NODE_ELEMENT) ++childCount;

		child->Release();
	}
	return childCount;
}

static void _buildNode(IXMLDOMNode *xmlnode, xml::node& nodebuf)
{
	// Get node name.
	BSTR bstr = nullptr;
	xmlnode->get_nodeName(&bstr);
	nodebuf.name = static_cast<wchar_t*>(bstr);
	SysFreeString(bstr);

	// Parse attributes of node, if any.
	_readAttrs(xmlnode, nodebuf.attrs);

	// Process children, if any.
	VARIANT_BOOL vb = FALSE;
	xmlnode->hasChildNodes(&vb);
	if (vb) {
		IXMLDOMNodeList *nodeList = nullptr;
		xmlnode->get_childNodes(&nodeList);
		nodebuf.children.resize(_countChildNodes(nodeList));

		int childCount = 0;
		long totalCount = 0;
		nodeList->get_length(&totalCount);

		for (long i = 0; i < totalCount; ++i) {
			IXMLDOMNode *child = nullptr;
			nodeList->get_item(i, &child);

			// Node can be text or an actual child node.
			DOMNodeType type = NODE_INVALID;
			child->get_nodeType(&type);
			if (type == NODE_TEXT) {
				xmlnode->get_text(&bstr);
				nodebuf.value.append(static_cast<wchar_t*>(bstr));
				SysFreeString(bstr);
			} else if (type == NODE_ELEMENT) {
				_buildNode(child, nodebuf.children[childCount++]); // recursively
			} else {
				// (L"Unhandled node type: %d.\n", type);
			}
			child->Release();
		}
		nodeList->Release();
	} else {
		// Assumes that only a leaf node can have text.
		xmlnode->get_text(&bstr);
		nodebuf.value = static_cast<wchar_t*>(bstr);
		SysFreeString(bstr);
	}
}

bool xml::parse(const wchar_t *str)
{
	CoInitialize(nullptr); // http://stackoverflow.com/questions/7824383/double-calls-to-coinitialize

	// Create COM object for XML document.
	IXMLDOMDocument2 *doc = nullptr;
	CoCreateInstance(CLSID_DOMDocument30, nullptr, CLSCTX_INPROC_SERVER,
		IID_IXMLDOMDocument, reinterpret_cast<void**>(&doc));
	doc->put_async(FALSE);

	// Parse the XML string.
	VARIANT_BOOL vb = FALSE;
	doc->loadXML(static_cast<BSTR>(const_cast<wchar_t*>(str)), &vb);

	// Get document element and root node from XML.
	IXMLDOMElement *docElem = nullptr;
	doc->get_documentElement(&docElem);

	IXMLDOMNode *rootNode = nullptr;
	docElem->QueryInterface(IID_IXMLDOMNode, reinterpret_cast<void**>(&rootNode));
	_buildNode(rootNode, root); // recursive

	rootNode->Release(); // must be released before CoUninitialize
	docElem->Release();
	doc->Release();
	CoUninitialize();
	return true;
}