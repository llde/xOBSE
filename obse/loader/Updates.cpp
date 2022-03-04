#include "Updates.h"
#include <string>

#include  <winhttp.h>
#include <obse/obse_common/obse_version.h>


#pragma comment(lib, "winhttp.lib")
std::tuple<UpdateType, std::string> CheckForUpdate() {
	const char* releases = "https://api.github.com/repos/llde/xOBSE/releases/latest";
	UpdateType updateType = UpdateType::None;
	std::string version = "";
	//TODO Win 8.1 >= proxy flag
	HINTERNET gSession = WinHttpOpen(L"xOBSE Update", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (gSession) {
		HINTERNET gConnect = WinHttpConnect(gSession, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
		UInt32 pressed = 0;
		if (gConnect) {
			const WCHAR* type = L"application/json";
			const WCHAR* type1 = L"application/vnd.github+json";
			const WCHAR* accept[3] = { type , type1 , 0 };
			HINTERNET gRequest = WinHttpOpenRequest(gConnect, L"GET", L"/repos/llde/xOBSE/releases/latest", NULL, WINHTTP_NO_REFERER, accept, WINHTTP_FLAG_REFRESH | WINHTTP_FLAG_SECURE);
			if (gRequest) {
				BOOL res = WinHttpSendRequest(gRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
				if (res) {
					res = WinHttpReceiveResponse(gRequest, 0);
					if (res) {
						char* buffer = nullptr;
						DWORD read = 0;
						while (true) {
							DWORD bytes = 0;
							WinHttpQueryDataAvailable(gRequest, &bytes);
							if (bytes == 0) break;
							if (buffer == nullptr) buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes);
							else {
								char* oldBuf = buffer;
								buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, read + bytes);
								memcpy(buffer, oldBuf, read);
								HeapFree(GetProcessHeap(), 0, oldBuf);
							}
							DWORD readi = 0;
							WinHttpReadData(gRequest, buffer + read, bytes, &readi);
							read += readi;
						}
						if (buffer != nullptr) {
							char* oldBuf = buffer;
							buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, read + 1);
							memcpy(buffer, oldBuf, read);
							buffer[read] = '\0';
							HeapFree(GetProcessHeap(), 0, oldBuf);
							//		_MESSAGE("%s", buffer);
							std::string json = std::string(buffer);
							size_t tag = json.find("\"tag_name\":") + 12;
							size_t end = json.find("\"", tag);
							std::string sub = json.substr(tag, end - tag);
							size_t dot1 = sub.find(".");
							std::string major = sub.substr(0, dot1);
							std::string rest = sub.substr(dot1 + 1);
							size_t dot2 = rest.find(".");
							std::string minor, hotfix;
							if (dot2 != std::string::npos) {
								minor = rest.substr(0, dot2);
								hotfix = rest.substr(dot2 + 1, rest.size() - dot2 - 1);
							}
							else {
								minor = rest;
								hotfix = "0";
							}
							UInt32 Major = std::stoul(major);
							UInt32 Minor = std::stoul(minor);
							UInt32 Hotfix = std::stoul(hotfix);
							version = sub;
							//							_MESSAGE("%u %u  %u ", Major, Minor, Hotfix);
							if (Major > OBSE_VERSION_INTEGER) {
								updateType = UpdateType::Major;
							}
							else {
								if (Minor > OBSE_VERSION_INTEGER_MINOR) {
									updateType = UpdateType::Minor;
								}
								else {
									if (Hotfix > OBSE_VERSION_INTEGER_HOTIFX) {
										updateType = UpdateType::Hotfix;
									}
								}
							}
							HeapFree(GetProcessHeap(), 0, buffer);
						}
					}
				}
				WinHttpCloseHandle(gRequest);
			}
			WinHttpCloseHandle(gConnect);
		}
		WinHttpCloseHandle(gSession);
	}
	return { updateType, version };
}