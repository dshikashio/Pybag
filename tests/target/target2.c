#include <Windows.h>
#include <stdio.h>

PVOID ADDRESS = (PVOID)0x222000;

int wmain(int argc, wchar_t **argv)
{
    PVOID ptr = NULL;

    ptr = VirtualAlloc(ADDRESS, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (ptr == NULL)
    {
        printf("VirtualAlloc Failed: %08x\n", GetLastError());
        return -1;
    }

    printf("%S: Hello : %p\n", argv[0], ptr);

    PDWORD stop = (PDWORD)ptr;

    while (*stop == 0)
    {
        printf("Sleep\n");
        Sleep(2000);
    }

    //
    // set registers
    // end with 0xcc
    //
    // execute buffer


    return 0;
}

