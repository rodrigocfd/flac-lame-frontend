/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <unordered_map>
#include <Windows.h>
#include <winhttp.h>

namespace winutil {

class internet_session final {
private:
	HINTERNET _hSession;
public:
	~internet_session() { close(); }
	internet_session();
	internet_session(internet_session&& is);
	internet_session& operator=(internet_session&& is);
	HINTERNET         hsession() const { return _hSession; }
	void              close();
	bool              init(std::wstring *pErr = nullptr, const wchar_t *userAgent = L"Win32/1.0");
};


class internet_download final {
private:
	const internet_session&   _session;
	HINTERNET                 _hConnect, _hRequest;
	int                       _contentLength, _totalDownloaded;
	std::wstring              _url, _verb, _referrer;
	std::vector<BYTE>         _buffer;
	std::vector<std::wstring> _requestHeaders;
	std::unordered_map<std::wstring, std::wstring> _responseHeaders;
public:
	~internet_download() { abort(); }
	internet_download(const internet_session& session, std::wstring url, std::wstring verb = L"GET");
	void               abort();
	internet_download& add_request_header(const wchar_t *requestHeader);
	internet_download& add_request_header(std::initializer_list<const wchar_t*> requestHeaders);
	internet_download& set_referrer(const wchar_t *referrer);
	internet_download& set_referrer(const std::wstring& referrer) { return set_referrer(referrer.c_str()); }
	bool               start(std::wstring *pErr = nullptr);
	bool               has_data(std::wstring *pErr = nullptr);
	int                get_content_length() const   { return _contentLength; }
	int                get_total_downloaded() const { return _totalDownloaded; }
	float              get_percent() const;
	const std::vector<BYTE>&                              get_buffer() const           { return _buffer; }
	const std::vector<std::wstring>&                      get_request_headers() const  { return _requestHeaders; }
	const std::unordered_map<std::wstring, std::wstring>& get_response_headers() const { return _responseHeaders; }
private:
	bool _init_handles(std::wstring *pErr = nullptr);
	bool _contact_server(std::wstring *pErr = nullptr);
	bool _parse_headers(std::wstring *pErr = nullptr);
	bool _get_incoming_byte_count(DWORD& count, std::wstring *pErr = nullptr);
	bool _receive_bytes(UINT nBytesToRead, std::wstring *pErr = nullptr);
};


class internet_url final {
public:
	internet_url();
	bool           crack(const wchar_t *address, std::wstring *pErr = nullptr);
	bool           crack(const std::wstring& address, std::wstring *pErr = nullptr) { return crack(address.c_str(), pErr); }
	const wchar_t* scheme() const   { return _scheme; }
	const wchar_t* host() const     { return _host; }
	const wchar_t* user() const     { return _user; }
	const wchar_t* pwd() const      { return _pwd; }
	const wchar_t* path() const     { return _path; }
	const wchar_t* extra() const    { return _extra; }
	std::wstring   path_and_extra() const;
	int            port() const     { return _uc.nPort; }
	bool           is_https() const { return _uc.nScheme == INTERNET_SCHEME_HTTPS; }
private:
	URL_COMPONENTS _uc;
	wchar_t        _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
};

}//namespace winutil