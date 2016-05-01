
#pragma once
#include <string>
#include <unordered_map>
#include <Windows.h>
#include <winhttp.h>

class InternetSession final {
private:
	HINTERNET _hSession;
public:
	~InternetSession() { close(); }
	InternetSession();
	InternetSession(InternetSession&& is);
	InternetSession& operator=(InternetSession&& is);
	HINTERNET        hSession() const { return _hSession; }
	void             close();
	bool             init(std::wstring *pErr = nullptr, const wchar_t *userAgent = L"Win32/1.0");
};


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
	~InternetDownload() { abort(); }
	InternetDownload(const InternetSession& session, std::wstring url, std::wstring verb = L"GET");
	void              abort();
	InternetDownload& addRequestHeader(const wchar_t *requestHeader);
	InternetDownload& addRequestHeader(std::initializer_list<const wchar_t*> requestHeaders);
	InternetDownload& setReferrer(const wchar_t *referrer);
	InternetDownload& setReferrer(const std::wstring& referrer) { return setReferrer(referrer.c_str()); }
	bool              start(std::wstring *pErr = nullptr);
	bool              hasData(std::wstring *pErr = nullptr);
	int               getContentLength() const   { return _contentLength; }
	int               getTotalDownloaded() const { return _totalDownloaded; }
	float             getPercent() const;
	const std::vector<BYTE>&                              getBuffer() const          { return _buffer; }
	const std::vector<std::wstring>&                      getRequestHeaders() const  { return _requestHeaders; }
	const std::unordered_map<std::wstring, std::wstring>& getResponseHeaders() const { return _responseHeaders; }
private:
	bool _initHandles(std::wstring *pErr = nullptr);
	bool _contactServer(std::wstring *pErr = nullptr);
	bool _parseHeaders(std::wstring *pErr = nullptr);
	bool _getIncomingByteCount(DWORD& count, std::wstring *pErr = nullptr);
	bool _receiveBytes(UINT nBytesToRead, std::wstring *pErr = nullptr);
};


class InternetUrl final {
public:
	InternetUrl();
	bool           crack(const wchar_t *address, std::wstring *pErr = nullptr);
	bool           crack(const std::wstring& address, std::wstring *pErr = nullptr) { return crack(address.c_str(), pErr); }
	const wchar_t* scheme() const  { return _scheme; }
	const wchar_t* host() const    { return _host; }
	const wchar_t* user() const    { return _user; }
	const wchar_t* pwd() const     { return _pwd; }
	const wchar_t* path() const    { return _path; }
	const wchar_t* extra() const   { return _extra; }
	std::wstring   pathAndExtra() const;
	int            port() const    { return _uc.nPort; }
	bool           isHttps() const { return _uc.nScheme == INTERNET_SCHEME_HTTPS; }
private:
	URL_COMPONENTS _uc;
	wchar_t        _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
};