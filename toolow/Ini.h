//
// INI file automation, in-memory hash storage.
// Cloudy evening of September 17, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "File.h"
#include "Hash.h"

class Ini {
public:
	Hash<Hash<String>> sections;
	Hash<String>& operator[](const wchar_t *key) { return sections[key]; }
	Hash<String>& operator[](const String& key)  { return sections[key]; }

	Ini& setPath(const wchar_t *iniPath) { _path = iniPath; return *this; }
	bool load(String *pErr=0);
	bool serialize(String *pErr=0) const;
private:
	String _path;
	int _countSections(FileText *fin) const;
};