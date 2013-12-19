//
// Reads an ANSI file line by line.
// Coughing at Friday night, October 14, 2011.
// Rewritten for efficiency at Sunday night, October 30, 2011.
// Rewritten for UTF-8 (et al) at Monday, February 11, 2013.
//

#include "File.h"

bool File::Text::load(const wchar_t *path, String *pErr)
{
	File::Mapped fm;
	if(!fm.open(path, Access::READONLY, pErr))
		return false;
	return this->load(&fm);
}

bool File::Text::load(const File::Mapped *pfm)
{
	BYTE *pMem = pfm->pMem(); // the file reading is made upon a memory-mapped file
	BYTE *pPast = pfm->pPastMem();
	
	if((pPast - pMem >= 3) && !::memcmp(pMem, "\xEF\xBB\xBF", 3)) // UTF-8
	{
		pMem += 3; // skip BOM
		_text = String::ParseUtf8(pMem, (int)(pPast - pMem)); // the whole file is loaded into a String as wchar_t
	}
	else if((pPast - pMem >= 4) && !::memcmp(pMem, "\x00\x00\xFE\xFF", 4)) // UTF-32 BE
	{
		pMem += 4;
		return false;
		//...
	}
	else if((pPast - pMem >= 4) && !::memcmp(pMem, "\xFF\xFE\x00\x00", 4)) // UTF-32 LE
	{
		pMem += 4;
		return false;
		//...
	}
	else if((pPast - pMem >= 2) && !::memcmp(pMem, "\xFE\xFF", 2)) // UTF-16 BE
	{
		pMem += 2;
		_text.reserve((int)(pPast - pMem) / 2);
		for(int i = 0; i < (int)(pPast - pMem); i += 2)
			_text[i / 2] = (wchar_t)MAKEWORD(*(pMem + i + 1), *(pMem + i));
	}
	else if((pPast - pMem >= 2) && !::memcmp(pMem, "\xFF\xFE", 2)) // UTF-16 LE
	{
		pMem += 2;
		_text.reserve((int)(pPast - pMem) / 2);
		for(int i = 0; i < (int)(pPast - pMem); i += 2)
			_text[i / 2] = (wchar_t)MAKEWORD(*(pMem + i), *(pMem + i + 1));
	}
	else // ASCII
	{
		int len = (int)(pPast - pMem);
		_text.reserve(len);
		for(int i = 0; i < len; ++i)
			_text[i] = (wchar_t)*(pMem + i); // brute-force char to wchar_t
	}

	this->rewind(); // our seeking pointer to be consumed by nextLine()
	return true;
}

bool File::Text::nextLine(String *pBuf)
{
	if(!*_p) return false; // runner pointer inside our _text String data block
	
	if(_idxLine > -1) { // not 1st line; avoid a 1st blank like to be skipped
		if( (*_p == L'\r' && *(_p + 1) == L'\n') || // CRLF || LFCR
			(*_p == L'\n' && *(_p + 1) == L'\r') ) _p += 2;
		else if(*_p == L'\r' || *_p == L'\n') ++_p; // CR || LF
	}
	++_idxLine;
	
	wchar_t *pRun = _p;
	while(*pRun && *pRun != L'\r' && *pRun != '\n') ++pRun;
	pBuf->reserve((int)(pRun - _p));
	pBuf->copyFrom(_p, (int)(pRun - _p)); // line won't have CR nor LF at end
	
	_p = pRun; // consume
	return true;
}