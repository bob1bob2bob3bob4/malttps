#include <Windows.h>
#include <stdio.h>

#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <DLL_PATH>\n", argv[0]);
		return 1;
	}

	const char* dll_path = argv[1];
	printf("%s", dll_path);
	printf("Inject \"%s\" to the process %d\n", dll_path, GetCurrentProcessId());

	if(LoadLibraryA(dll_path) == NULL)
	{
		printf("LoadLibraryA() failed, Error: %d\n", GetLastError());
		return 1;
	}
	printf("DLL injected successfully\n");

	getchar();

	return 0;


  
}

