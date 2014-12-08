//
// Some data structures.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
//

#pragma once
#include "String.h"

// Smart pointer.
template<typename T> class Ptr final {
private:
	T   *_ptr;
	int *_counter;
public:
	Ptr()                 : _ptr(nullptr), _counter(nullptr) { }
	Ptr(T *ptr)           : _ptr(nullptr), _counter(nullptr) { operator=(ptr); }
	Ptr(const Ptr& other) : _ptr(nullptr), _counter(nullptr) { operator=(other); }
	~Ptr() {
		if (_counter && !--(*_counter)) {
			delete _ptr;     _ptr = nullptr;
			delete _counter; _counter = nullptr;
		}
	}
	Ptr& operator=(T *ptr) {
		this->~Ptr();
		if (ptr) {
			_ptr = ptr; // take ownership
			_counter = new int(1); // start counter
		}
		return *this;
	}
	Ptr& operator=(const Ptr& other) {
		if (this != &other) {
			this->~Ptr();
			_ptr = other._ptr;
			_counter = other._counter;
			if (_counter) ++(*_counter);
		}
		return *this;
	}
	bool isNull() const         { return _ptr == nullptr; }
	T& operator*()              { return *_ptr; }
	const T* operator->() const { return _ptr; }
	T* operator->()             { return _ptr; }
	operator T*() const         { return _ptr; }
};

// Smart pointer for COM objects.
template<typename T> class ComPtr final {
private:
	T *_ptr;
public:
	ComPtr()                    : _ptr(nullptr) { }
	ComPtr(const ComPtr& other) : _ptr(nullptr) { operator=(other); }
	~ComPtr()                   { this->release(); }
	ComPtr& operator=(const ComPtr& other) {
		if (this != &other) {
			this->~ComPtr();
			_ptr = other._ptr;
			if (_ptr) _ptr->AddRef();
		}
		return *this;
	}
	void release() {
		if (_ptr) {
			_ptr->Release();
			_ptr = nullptr;
		}
	}
	bool coCreateInstance(REFCLSID rclsid) {
		return _ptr ? false :
			SUCCEEDED(::CoCreateInstance(rclsid, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_ptr)));
	}
	bool coCreateInstance(REFCLSID rclsid, REFIID riid) {
		return _ptr ? false :
			SUCCEEDED(::CoCreateInstance(rclsid, 0, CLSCTX_INPROC_SERVER, riid, (void**)&_ptr));
	}
	template<typename COM_INTERFACE> bool queryInterface(REFIID riid, COM_INTERFACE **comPtr) {
		return !_ptr ? false :
			SUCCEEDED(_ptr->QueryInterface(riid, (void**)comPtr));
	}
	bool isNull() const { return _ptr == nullptr; }
	T&   operator*()    { return *_ptr; }
	T*   operator->()   { return _ptr; }
	T**  operator&()    { return &_ptr; }
	operator T*() const { return _ptr; }
};

// Associative array.
template<typename T> class Hash final {
public:
	struct Elem final {
		Elem()                             { }
		Elem(const Elem& other)            : key(other.key), val(other.val) { }
		Elem(Elem&& other)                 : key(MOVE(other.key)), val(MOVE(val)) { }
		explicit Elem(const wchar_t *k)    : key(k) { }
		Elem& operator=(const Elem& other) { key = other.key; val = other.val; return *this; }
		Elem& operator=(Elem&& other)      { key = MOVE(other.key); val = MOVE(other.val); return *this; }
		String key;
		T      val;
	};
private:
	Array<Elem> _elems;
public:
	Hash()                  { }
	Hash(const Hash& other) { operator=(other); }
	Hash(Hash&& other)      { operator=(MOVE(other)); }

	int         size() const                     { return _elems.size(); }
	bool        exists(const wchar_t *key) const { return this->_byKey(key) > -1; }
	const Elem* at(int index) const              { return &_elems[index]; }
	Elem*       at(int index)                    { return &_elems[index]; }

	Hash& operator=(const Hash& other) { _elems.resize(0).append(other._elems); return *this; }
	Hash& operator=(Hash&& other)      { _elems = MOVE(other._elems); return *this; }
	Hash& reserve(int howMany)         { _elems.reserve(howMany); return *this; }
	Hash& removeAll()                  { _elems.resize(0); return *this; }
	Hash& remove(const wchar_t *key)   { return this->remove(this->_byKey(key)); }
	Hash& remove(int index)            { _elems.remove(index); return *this; }

	const T& operator[](const String& key) const  { return operator[](key.str()); }
	const T& operator[](const wchar_t *key) const {
		int idx = this->_byKey(key);
		if (idx == -1) // key not found
			return _elems[0].val; // not right... lame C++ won't allow null references!
		return _elems[idx].val;
	}
	T& operator[](const String& key)  { return operator[](key.str()); }
	T& operator[](const wchar_t *key) {
		int idx = this->_byKey(key);
		if (idx == -1) { // key not found, let's insert it
			_elems.append(Elem(key)); // create entry with default constructor
			idx = _elems.size() - 1;
		}
		return _elems[idx].val;
	}
	
	void each(function<void(Elem& elem)> callback) {
		// Example usage:
		// Hash<int> nums;
		// nums.each([](Hash<int>::Elem& elem) { elem.val += 10; });
		for (Elem& elem : _elems) callback(elem);
	}
	void each(function<void(const Elem& elem)> callback) const {
		// Example usage:
		// Hash<int> nums;
		// nums.each([](const Hash<int>::Elem& elem) { int x = elem.val; });
		for (const Elem& elem : _elems) callback(elem);
	}
private:
	int _byKey(const wchar_t *keyName) const {
		for (int i = 0; i < _elems.size(); ++i) // linear search
			if (_elems[i].key.equalsCS(keyName)) // an empty string is also a valid key
				return i;
		return -1; // not found
	}
};

// XML object.
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
private:
	static void _ReadAttrs(ComPtr<IXMLDOMNode> xmlnode, Hash<String>& attrbuf);
	static int  _CountChildNodes(ComPtr<IXMLDOMNodeList> nodeList);
	static void _BuildNode(ComPtr<IXMLDOMNode> xmlnode, Xml::Node& nodebuf);
};