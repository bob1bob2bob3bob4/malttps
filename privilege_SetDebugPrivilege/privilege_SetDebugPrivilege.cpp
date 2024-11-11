// privilege_SetDebugPrivilege.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>

int main()
{
    HANDLE token;
    LUID luid;
    TOKEN_PRIVILEGES tp;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
    {
        std::cout << "OpenProcessToken failed: " << GetLastError() << std::endl;
        return 1;
    }
    else {
        if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
        {
            std::cout << "LookupPrivilegeValue failed: " << GetLastError() << std::endl;
            return 1;
        }
        else {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
            {
                std::cout << "AdjustTokenPrivileges failed: " << GetLastError() << std::endl;
                return 1;
            }
            else {
                std::cout << "Privilege set successfully" << std::endl;
            }
        }
    }
    return 0;
}