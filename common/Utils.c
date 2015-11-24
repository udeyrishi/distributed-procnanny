#include "Utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "memwatch.h"

bool checkMallocResult(void* pointer, LogReport* report)
{
    if (pointer == NULL)
    {
        report -> message = "Out of memory.";
        report -> type = DEBUG;
        return false;
    }
    return true;
}

char* stringJoin(const char* first, const char* second)
{
    int len1 = strlen(first);
    int len2 = strlen(second);
    int sizeBuffer = len1 + len2 + 1;
    char* joined = (char*)malloc(sizeof(char)*sizeBuffer);
    LogReport report;
    if (!checkMallocResult(joined, &report))
    {
        return (char*)NULL;
    }

    strncpy(joined, first, len1);
    strncpy(joined+len1, second, len2);
    joined[len1 + len2] = '\0';
    assert(strlen(joined) + 1 == sizeBuffer);
    return joined;
}

char* stringNumberJoin(const char* first, int second)
{
	char str[15]; // good enough
    sprintf(str, "%d", second);
    return stringJoin(first, str);
}

char* stringULongJoin(const char* first, unsigned long int second)
{
    char str[15]; // good enough
    sprintf(str, "%lu", second);
    return stringJoin(first, str);
}

char* numberStringJoin(int first, const char* second)
{
	char str[15]; // good enough
    sprintf(str, "%d", first);
    return stringJoin(str, second);
}

bool compareStrings(const char* first, const char* second)
{
	return (bool)(strcmp(first, second) == 0);
}

char* copyString(char* source)
{
    return stringJoin("", source);
}

char* getNextStrTokString(char* init)
{
    char* next = strtok(init, " ");
    int len = strlen(next);
    char* p = (char*)malloc(sizeof(char)*(len+1));
    strcpy(p, next);
    return p;
}