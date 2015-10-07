#ifndef __UTILS_H__
#define __UTILS_H__

#include "Logging.h"

#define TRUE 1
#define FALSE 0

typedef int Boolean;

Boolean checkMallocResult(void* pointer, LogReport* report);

#endif