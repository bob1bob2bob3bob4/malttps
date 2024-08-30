#include <Windows.h>
#include <stdio.h>
#include "pch.h"


VOID MsgBox(LPCWSTR msg) {
	MessageBoxW(NULL, msg, L"MsgBox", MB_OK);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MsgBox(L"Local DLL injection test");
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

