
#include "Ini.h"

bool Ini::load(String *pErr)
{
	if(_path.isEmpty()) {
		*pErr = L"INI path not set.";
		return false;
	}

	FileText fin;
	if(!fin.load(_path.str(), pErr)) {
		if(pErr) pErr->insert(0, L"INI file failed to load.\n");
		return false;
	}

	this->sections.removeAll().reserve( this->_countSections(&fin) );

	String line;
	while(fin.nextLine(&line)) {
		if(line[0] == L'[' && line.endsWith(L']')) { // begin section
			String sectionName;
			sectionName.getSubstr(line.str(), 1, -1);
			this->sections[sectionName] = Hash<String>(); // new section is an empty hash
			continue;
		}
		if(this->sections.size() && line.len()) {
			int idxEq = line.find(L'=');
			if(idxEq > -1) {
				String keyName, valStr;
				keyName.getSubstr(line.str(), 0, idxEq);
				valStr.getSubstr(line.str(), idxEq + 1);

				Hash<String> *pLastSection = &this->sections.at(this->sections.size() - 1)->val;
				(*pLastSection)[keyName.trim()] = valStr.trim();
			}
		}
	}

	if(pErr) *pErr = L"";
	return true;
}

bool Ini::serialize(String *pErr) const
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

	if(!FileText::Write(_path.str(), out.str(), pErr)) {
		pErr->insert(0, L"INI file serialization failed.\n");
		return false;
	}

	if(pErr) *pErr = L"";
	return true;
}

int Ini::_countSections(FileText *fin) const
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