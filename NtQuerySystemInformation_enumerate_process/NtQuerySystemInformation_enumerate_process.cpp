
#include <iostream>
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>

typedef NTSTATUS (NTAPI* fnNtQuerySystemInformation)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID                    SystemInformation,
	ULONG                    SystemInformationLength,
	PULONG                   ReturnLength
);

BOOL EnumerateProcesses(LPCWSTR ProcName)
{
	HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
	if (hNtdll == NULL)
	{
		printf("Failed to get handle to ntdll.dll: %d\n", GetLastError());
		return FALSE;
	}

	fnNtQuerySystemInformation NtQuerySystemInformation = (fnNtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
	if (NtQuerySystemInformation == NULL)
	{
		printf("Failed to get address of NtQuerySystemInformation: %d\n", GetLastError());
		return FALSE;
	}

	ULONG ReturnLength = 0;
	NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &ReturnLength);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
	{
		printf("Failed to get length of system information: %d\n", status);
		return FALSE;
	}

	PVOID pBuffer = VirtualAlloc(NULL, ReturnLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pBuffer == NULL)
	{
		printf("Failed to alloc mmemory: %d\n", GetLastError());
		return FALSE;
	}

	status = NtQuerySystemInformation(SystemProcessInformation, pBuffer, ReturnLength, &ReturnLength);
	if (status != STATUS_SUCCESS)
	{
		printf("Failed to get system information: %d\n", status);
		return FALSE;
	}

	PSYSTEM_PROCESS_INFORMATION pCurrent = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
	while (TRUE)
	{
		if(pCurrent->ImageName.Length && wcscmp(pCurrent->ImageName.Buffer, ProcName) ==0){
			std::wcout << "Process ID: " << pCurrent->UniqueProcessId << std::endl;
			std::wcout << "Process Name: " << pCurrent->ImageName.Buffer << std::endl;
			break;
		}
		if(!pCurrent->NextEntryOffset)
			break;
		pCurrent = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);

	}
	VirtualFree(pBuffer, 0, MEM_RELEASE);

	return TRUE;
}

int main()
{
	EnumerateProcesses(L"notepad.exe");
	getchar();
	return 0;
}

