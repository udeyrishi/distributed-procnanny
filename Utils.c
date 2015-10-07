#include "Utils.h"
#include <stdlib.h>

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