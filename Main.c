#include "Logging.h"
#include "ProcessReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "memwatch.h"

#define MAX_PROCESSES 128
#define MAX_PROCESS_LINE_LENGTH 200

int main(void)
{
	int i;
    LogReport report;
	char** words = getOutputFromProgram("ps", MAX_PROCESSES, MAX_PROCESS_LINE_LENGTH, &i, &report);
    int j;
    for(j = 0; j < i; j++)
    {
        printf("%s\n", words[j]);
    }

    for (;i>=0;i--)
    {
        free(words[i]);
    }
    free(words);
    return 0;
}