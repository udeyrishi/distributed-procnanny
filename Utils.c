#include "Utils.h"
#include <stdlib.h>
#include "memwatch.h"

Boolean checkMallocResult(void* pointer, LogReport* report)
{
    if (pointer == NULL)
    {
        report -> message = "Out of memory.";
        report -> type = FATAL;
        return FALSE;
    }
    return TRUE;
}

void safeFree(void* pointer)
{
	free(pointer);
	pointer = NULL;
}

void safeMwFree(void* pointer)
{
	mwFree_(pointer);
	pointer = NULL;
}