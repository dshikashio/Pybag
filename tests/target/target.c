#include <Windows.h>
#include <stdio.h>


int wmain(int argc, wchar_t **argv)
{
    printf("%S: Hello\n", argv[0]);
    return 0;
}

