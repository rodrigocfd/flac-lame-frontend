/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <unordered_map>
#include "InternetUrl.h"

namespace wolf {

class InternetDownload final {
private:
	const InternetSession&    _session;
	HINTERNET                 _hConnect, _hRequest;
	int                       _contentLength, _totalDownloaded;
	std::wstring              _url, _verb, _referrer;
	std::vector<BYTE>         _buffer;
	std::vector<std::wstring> _requestHeaders;
	std::unordered_map<std::wstring, std::wstring> _responseHeaders;
public:
	~InternetDownload();
	InternetDownload(const InternetSession& session, std::wstring url, std::wstring verb=L"GET");
	void              abort();
	InternetDownload& addRequestHeaders(std::initializer_list<const wchar_t*> requestHeaders);
	InternetDownload& setReferrer(const wchar_t *referrer);
	bool              start(std::wstring *pErr=nullptr);
	bool              hasData(std::wstring *pErr=nullptr);
	int               getContentLength() const;
	int               getTotalDownloaded() const;
	float             getPercent() const;
	const std::vector<BYTE>&                              getBuffer() const;
	const std::vector<std::wstring>&                      getRequestHeaders() const;
	const std::unordered_map<std::wstring, std::wstring>& getResponseHeaders() const;
private:
	bool _initHandles(std::wstring *pErr=nullptr);
	bool _contactServer(std::wstring *pErr=nullptr);
	bool _parseHeaders(std::wstring *pErr=nullptr);
	bool _getIncomingByteCount(DWORD& count, std::wstring *pErr=nullptr);
	bool _receiveBytes(UINT nBytesToRead, std::wstring *pErr=nullptr);
};

}//namespace wolf