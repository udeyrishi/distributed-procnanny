#ifndef __UTILS_H__
#define __UTILS_H__

#include "LogReport.h"
#include <stdbool.h>

bool checkMallocResult(void* pointer, LogReport* report);
char* stringJoin(const char* first, const char* second);
char* stringNumberJoin(const char* first, int second);
char* stringULongJoin(const char* first, unsigned long int second);
char* numberStringJoin(int first, const char* second);
bool compareStrings(const char* first, const char* second);
char* copyString(char* source);
char* getNextStrTokString(char* init);
#endif