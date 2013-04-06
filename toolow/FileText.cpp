//
// Reads an ANSI file line by line.
// Coughing at Friday night, October 14, 2011.
// Rewritten for efficiency at Sunday night, October 30, 2011.
// Rewritten for UTF-8 (et al) at Monday, February 11, 2013.
//

#include "File.h"

bool FileText::load(const wchar_t *path, String *pErr)
{
	FileMap fm;
	if(!fm.open(path, File::READONLY, pErr))
		return false;
	return this->load(&fm);
}

bool FileText::load(const FileMap *pfm)
{
	BYTE *pMem = pfm->pMem(); // the file reading is made upon a memory-mapped file
	BYTE *pPast = pfm->pPastMem();
	
	if((pPast - pMem >= 3) && !memcmp(pMem, "\xEF\xBB\xBF", 3)) // UTF-8
	{
		pMem += 3; // skip BOM
		int len = MultiByteToWideChar(CP_UTF8, 0, (const char*)pMem, pPast - pMem, 0, 0);
		_text.reserve(len);
		MultiByteToWideChar(CP_UTF8, 0, (const char*)pMem, pPast - pMem,
			_text.ptrAt(0), len); // the whole file is loaded into a String as wchar_t
	}
	else if((pPast - pMem >= 4) && !memcmp(pMem, "\x00\x00\xFE\xFF", 4)) // UTF-32 BE
	{
		pMem += 4;
		//...
	}
	else if((pPast - pMem >= 4) && !memcmp(pMem, "\xFF\xFE\x00\x00", 4)) // UTF-32 LE
	{
		pMem += 4;
		//...
	}
	else if((pPast - pMem >= 2) && !memcmp(pMem, "\xFE\xFF", 2)) // UTF-16 BE
	{
		pMem += 2;
		//...
	}
	else if((pPast - pMem >= 2) && !memcmp(pMem, "\xFF\xFE", 2)) // UTF-16 LE
	{
		pMem += 2;
		//...
	}
	else // ASCII
	{
		int len = pPast - pMem;
		_text.reserve(len);
		for(int i = 0; i < len; ++i)
			_text[i] = (wchar_t)*(pMem + i); // brute-force char to wchar_t
	}

	this->rewind(); // our seeking pointer to be consumed by nextLine()
	return true;
}

bool FileText::nextLine(String *pBuf)
{
	if(!*_p) return false; // runner pointer inside our _text String data block
	
	if(!_firstLine) { // avoid a 1st blank like to be skipped
		if( (*_p == L'\r' && *(_p + 1) == L'\n') || // CRLF || LFCR
			(*_p == L'\n' && *(_p + 1) == L'\r') ) _p += 2;
		else if(*_p == L'\r' || *_p == L'\n') ++_p; // CR || LF
	}
	_firstLine = false;
	
	wchar_t *pRun = _p;
	while(*pRun && *pRun != L'\r' && *pRun != '\n') ++pRun;
	pBuf->reserve(pRun - _p);
	pBuf->copyFrom(_p, pRun - _p); // line won't have CR nor LF at end
	
	_p = pRun; // consume
	return true;
}

bool FileText::Write(const wchar_t *path, const wchar_t *data, String *pErr)
{
	bool isUtf8 = false;
	int dataLen = lstrlen(data);
	for(int i = 0; i < dataLen; ++i) {
		if(data[i] > 127) {
			isUtf8 = true;
			break;
		}
	}
	
	File fout;
	if(!fout.open(path, File::READWRITE, pErr))
		return false;
	if(fout.size() && !fout.setNewSize(0, pErr)) // if already exists, truncate to empty
		return false;
	
	// If the text doesn't have any char to make it UTF-8, it'll
	// be simply converted to plain ASCII.
	int newLen = WideCharToMultiByte(CP_UTF8, 0, data, dataLen, 0, 0, 0, 0);
	Array<BYTE> outBuf(newLen + (isUtf8 ? 3 : 0));
	if(isUtf8)
		memcpy(&outBuf[0], "\xEF\xBB\xBF", 3); // write UTF-8 BOM
	WideCharToMultiByte(CP_UTF8, 0, data, dataLen, (char*)&outBuf[isUtf8 ? 3 : 0], newLen, 0, 0);
	if(!fout.write(&outBuf, pErr)) // one single write() to all data, better performance
		return false;
	
	if(pErr) *pErr = L"";
	return true;
}