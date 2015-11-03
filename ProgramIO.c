#include "ProgramIO.h"
#include <stdio.h>
#include <string.h>
#include "memwatch.h"

#define STARTING_ALLOCATION_SIZE 128

// Adapted from: http://stackoverflow.com/questions/19173442/reading-each-line-of-file-into-array
char** getOutputFromProgram(const char* programName, int * numberLinesRead, LogReport* report) 
{
    int currentAllocationSize = STARTING_ALLOCATION_SIZE;
    char **lines = (char **)malloc(sizeof(char*)*currentAllocationSize);
    
    if (!checkMallocResult(lines, report))
    {
        return (char**)NULL;
    }

    FILE *fp = popen(programName, "r");
    if (fp == NULL)
    {
        report -> message = "Error opening program stream file.";
        report -> type = DEBUG;
        return (char**)NULL;
    }

    int i;
    for (i = 0; true; ++i)
    {
        int j;
        if (i >= currentAllocationSize)
        {
            int newSize;

            // Double our allocation and re-allocate
            newSize = currentAllocationSize*2;
            lines = (char**)realloc(lines, sizeof(char*)*newSize);
            if (!checkMallocResult(lines, report))
            {
                return (char**)NULL;
            }
            currentAllocationSize = newSize;
        }

        lines[i] = (char*)NULL;
        size_t zeroSize = 0;
        if (getline(&lines[i], &zeroSize, fp) == -1)
        {
            break;
        }
        
        // Get rid of CR or LF at end of line
        for (j = strlen(lines[i])-1; j >= 0 && (lines[i][j] == '\n' || lines[i][j] == '\r'); j--);

        lines[i][j+1] = '\0';
    }
    
    // Close file
    if (pclose(fp) != 0) 
    {
        report -> message = "Failed to close the program stream.";
        report -> type = DEBUG;
        return (char**)NULL;
    }

    *numberLinesRead = i;

    return lines;
}

void freeOutputFromProgram(char** output, int numberLinesRead)
{
    int i;
    for (i = 0; i < numberLinesRead; ++i)
    {
        //getOutputFromProgram uses getline. Causes WILD free warning with memwatch if freed regularly   
        // Source: http://webdocs.cs.ualberta.ca/~paullu/C379/memwatch-2.71/FAQ
        mwFree_(output[i]);
    }
    free(output);
}

char** readFile(const char* filePath, int* numberLinesRead, LogReport* report)
{
    char* catCall = stringJoin("cat ", filePath);
    char** lines = getOutputFromProgram(catCall, numberLinesRead, report);
    free(catCall);
    return lines;
}