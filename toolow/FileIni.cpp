//
// INI file automation, in-memory hash storage.
// Cloudy evening of September 17, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#include "File.h"

bool File::Ini::load(String *pErr)
{
	if(_path.isEmpty()) {
		*pErr = L"INI path not set.";
		return false;
	}

	File::Text fin;
	if(!fin.load(_path.str(), pErr)) {
		if(pErr) pErr->insert(0, L"INI file failed to load.\n");
		return false;
	}

	this->sections.removeAll().reserve( this->_countSections(&fin) );

	String line, name, valstr; // name/val declared here to save reallocs
	while(fin.nextLine(&line)) {
		if(line[0] == L'[' && line.endsWith(L']')) { // begin section found
			name.copyFrom(line.ptrAt(1), line.len() - 2);
			this->sections[name] = Hash<String>(); // new section is an empty hash
			continue;
		}
		if(this->sections.size() && line.len()) { // keys will be read only if within a section
			int idxEq = line.find(L'=');
			if(idxEq > -1) {
				name.copyFrom(line.ptrAt(0), idxEq);
				valstr.copyFrom(line.ptrAt(idxEq + 1), line.len() - (idxEq + 1));

				Hash<String> *pLastSection = &this->sections.at(this->sections.size() - 1)->val;
				(*pLastSection)[name.trim()] = valstr.trim();
			}
		}
	}

	if(pErr) *pErr = L"";
	return true;
}

bool File::Ini::serialize(String *pErr) const
{
	if(_path.isEmpty()) {
		*pErr = L"INI path not set.";
		return false;
	}

	String out;
	for(int i = 0; i < this->sections.size(); ++i) {
		const Hash<Hash<String>>::Elem *section = this->sections.at(i);
		out.append(L'[').append( section->key.str() ).append(L"]\r\n");

		const Hash<String> *entries = &section->val;
		for(int j = 0; j < entries->size(); ++j) {
			const Hash<String>::Elem *entry = entries->at(j);
			out.append( entry->key.str() ).append(L'=').append( entry->val.str() ).append(L"\r\n");
		}
		
		out.append(L"\r\n");
	}

	if(!File::WriteUtf8(_path.str(), out.str(), pErr)) {
		pErr->insert(0, L"INI file serialization failed.\n");
		return false;
	}

	if(pErr) *pErr = L"";
	return true;
}

int File::Ini::_countSections(File::Text *fin) const
{
	int count = 0;
	String line;
	fin->rewind();
	while(fin->nextLine(&line))
		if(line[0] == L'[' && line.endsWith(L']'))
			++count;
	fin->rewind();
	return count;
}