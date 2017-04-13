/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <MsXml2.h>
#pragma comment(lib, "msxml2.lib")

namespace wl {

// XML wrapper class to MSXML2 Windows library.
class xml final {
public:
	class node final {
	public:
		std::wstring name;
		std::wstring value;
		std::unordered_map<std::wstring, std::wstring> attrs;
		std::vector<node> children;

		std::vector<node*> children_by_name(const wchar_t* elemName) {
			std::vector<node*> nodeBuf;
			for (node& node : this->children) {
				if (!lstrcmpiW(node.name.c_str(), elemName)) { // case-insensitive match
					nodeBuf.emplace_back(&node);
				}
			}
			return nodeBuf;
		}

		std::vector<node*> children_by_name(const std::wstring& elemName) {
			return this->children_by_name(elemName.c_str());
		}

		node* first_child_by_name(const wchar_t* elemName) {
			for (node& node : this->children) {
				if (!lstrcmpiW(node.name.c_str(), elemName)) { // case-insensitive match
					return &node;
				}
			}
			return nullptr; // not found
		}

		node* first_child_by_name(const std::wstring& elemName) {
			return this->first_child_by_name(elemName.c_str());
		}
	};
public:
	node root;

	xml() = default;
	xml(const xml& other)        : root(other.root) { }
	xml(xml&& other)             : root(std::move(other.root)) { }
	xml(const wchar_t* str)      : xml() { parse(str); }
	xml(const std::wstring& str) : xml(str.c_str()) { }

	xml& operator=(const xml& other) {
		this->root = other.root;
		return *this;
	}

	xml& operator=(xml&& other) {
		this->root = std::move(other.root);
		return *this;
	}

	bool parse(const wchar_t* str) {
		CoInitialize(nullptr); // http://stackoverflow.com/a/7824428

		// Create COM object for XML document.
		IXMLDOMDocument2* doc = nullptr;
		CoCreateInstance(CLSID_DOMDocument30, nullptr, CLSCTX_INPROC_SERVER,
			IID_IXMLDOMDocument, reinterpret_cast<void**>(&doc));
		doc->put_async(FALSE);

		// Parse the XML string.
		VARIANT_BOOL vb = FALSE;
		doc->loadXML(static_cast<BSTR>(const_cast<wchar_t*>(str)), &vb);

		// Get document element and root node from XML.
		IXMLDOMElement* docElem = nullptr;
		doc->get_documentElement(&docElem);

		IXMLDOMNode* rootNode = nullptr;
		docElem->QueryInterface(IID_IXMLDOMNode, reinterpret_cast<void**>(&rootNode));
		_build_node(rootNode, this->root); // recursive

		rootNode->Release(); // must be released before CoUninitialize
		docElem->Release();
		doc->Release();
		CoUninitialize();
		return true;
	}

	bool parse(const std::wstring& str) {
		return this->parse(str.c_str());
	}

private:
	static void _build_node(IXMLDOMNode* xmlnode, xml::node& nodebuf) {
		// Get node name.
		BSTR bstr = nullptr;
		xmlnode->get_nodeName(&bstr);
		nodebuf.name = static_cast<wchar_t*>(bstr);
		SysFreeString(bstr);

		// Parse attributes of node, if any.
		_read_attrs(xmlnode, nodebuf.attrs);

		// Process children, if any.
		VARIANT_BOOL vb = FALSE;
		xmlnode->hasChildNodes(&vb);
		if (vb) {
			IXMLDOMNodeList* nodeList = nullptr;
			xmlnode->get_childNodes(&nodeList);
			nodebuf.children.resize(_count_child_nodes(nodeList));

			int childCount = 0;
			long totalCount = 0;
			nodeList->get_length(&totalCount);

			for (long i = 0; i < totalCount; ++i) {
				IXMLDOMNode* child = nullptr;
				nodeList->get_item(i, &child);

				// Node can be text or an actual child node.
				DOMNodeType type = NODE_INVALID;
				child->get_nodeType(&type);
				if (type == NODE_TEXT) {
					xmlnode->get_text(&bstr);
					nodebuf.value.append(static_cast<wchar_t*>(bstr));
					SysFreeString(bstr);
				} else if (type == NODE_ELEMENT) {
					_build_node(child, nodebuf.children[childCount++]); // recursively
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

	static void _read_attrs(IXMLDOMNode* xmlnode, std::unordered_map<std::wstring, std::wstring>& attrbuf) {
		// Read attribute collection.
		IXMLDOMNamedNodeMap* attrs = nullptr;
		xmlnode->get_attributes(&attrs);

		long attrCount = 0;
		attrs->get_length(&attrCount);
		attrbuf.clear();
		attrbuf.reserve(attrCount);

		for (long i = 0; i < attrCount; ++i) {
			IXMLDOMNode* attr = nullptr;
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

	static int _count_child_nodes(IXMLDOMNodeList* nodeList) {
		int childCount = 0;
		long totalCount = 0;
		nodeList->get_length(&totalCount); // includes text and actual element nodes

		for (long i = 0; i < totalCount; ++i) {
			IXMLDOMNode* child = nullptr;
			nodeList->get_item(i, &child);

			DOMNodeType type = NODE_INVALID;
			child->get_nodeType(&type);
			if (type == NODE_ELEMENT) ++childCount;

			child->Release();
		}
		return childCount;
	}
};

}//namespace wl