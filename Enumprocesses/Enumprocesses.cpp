#include <Windows.h>
#include <iostream>
#include <psapi.h>


BOOL ListProcesses() {
    DWORD   aProcesses [1024], 
            cbNeeded = NULL, 
            cProcesses = NULL;

    HANDLE hProcess = NULL;
    HMODULE hModule = NULL;
    WCHAR szProcessName[MAX_PATH];

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        printf("EnumProcesses failed: %d\n", GetLastError());
        return FALSE;
    }
    cProcesses = cbNeeded / sizeof(DWORD);
    printf(" %d processes detected\n", cProcesses);

    for (int i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != NULL) {
            if ((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i])) != NULL) {
                if (!EnumProcessModules(hProcess, &hModule, sizeof(HMODULE), &cbNeeded)) {
                    printf("EnumProcessModules failed: %d\n", GetLastError());
                }
                else {
                    if (!GetModuleBaseName(hProcess, hModule, szProcessName, sizeof(szProcessName) / sizeof(WCHAR))) {
                        printf("GetModuleBaseName failed: %d\n", GetLastError());
                    }
                    else {
                        wprintf(L"Process name: %s, pid: %lu\n", szProcessName, aProcesses[i]);
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }

    return TRUE;

}

int main()
{
    ListProcesses();

    getchar();
    return 0;
}

