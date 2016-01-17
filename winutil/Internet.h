
#pragma once
#include <string>
#include <unordered_map>
#include <Windows.h>
#include <winhttp.h>

class InternetSession final {
private:
	HINTERNET _hSession;
public:
	~InternetSession();
	InternetSession();
	InternetSession(InternetSession&& is);
	InternetSession& operator=(InternetSession&& is);
	HINTERNET        hSession() const;
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
	~InternetDownload();
	InternetDownload(const InternetSession& session, std::wstring url, std::wstring verb = L"GET");
	void              abort();
	InternetDownload& addRequestHeader(const wchar_t *requestHeader);
	InternetDownload& addRequestHeader(std::initializer_list<const wchar_t*> requestHeaders);
	InternetDownload& setReferrer(const wchar_t *referrer);
	bool              start(std::wstring *pErr = nullptr);
	bool              hasData(std::wstring *pErr = nullptr);
	int               getContentLength() const;
	int               getTotalDownloaded() const;
	float             getPercent() const;
	const std::vector<BYTE>&                              getBuffer() const;
	const std::vector<std::wstring>&                      getRequestHeaders() const;
	const std::unordered_map<std::wstring, std::wstring>& getResponseHeaders() const;
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
	bool           crack(const std::wstring& address, std::wstring *pErr = nullptr);
	const wchar_t* scheme() const;
	const wchar_t* host() const;
	const wchar_t* user() const;
	const wchar_t* pwd() const;
	const wchar_t* path() const;
	const wchar_t* extra() const;
	std::wstring   pathAndExtra() const;
	int            port() const;
	bool           isHttps() const;
private:
	URL_COMPONENTS _uc;
	wchar_t        _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
};