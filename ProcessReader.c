#include "ProcessReader.h"
#include "Logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memwatch.h"

// Source: http://stackoverflow.com/questions/19173442/reading-each-line-of-file-into-array
char** getOutputFromProgram(const char* programName, int maxNumberLines, int maxLineLength, int * numberLinesRead, LogReport* report) 
{
    /* Allocate lines of text */
    char **words = (char **)malloc(sizeof(char*)*maxNumberLines);
    
    if (words == NULL)
    {
        report -> message = "Out of memory.";
        report -> type = FATAL;
        return (char**)NULL;
    }

    FILE *fp = popen(programName, "r");
    if (fp == NULL)
    {
        report -> message = "Error running program.";
        report -> type = FATAL;
        return (char**)NULL;
    }

    int i;
    for (i = 0; i < maxNumberLines; i++)
    {
        int j;
        // Allocate space for the next line
        words[i] = malloc(maxLineLength);
        if (words[i] == NULL)
        {
            report -> message = "Out of memory.";
            report -> type = FATAL;
            return (char**)NULL;
        }

        if (fgets(words[i], maxLineLength - 1, fp) == NULL)
        {
            break;
        }

        // Get rid of CR or LF at end of line
        for (j = strlen(words[i])-1; j >= 0 && (words[i][j] == '\n' || words[i][j] == '\r'); j--);

        words[i][j+1] = '\0';
    }
    
    // Close file
    if (fclose(fp) != 0) 
    {
        report -> message = "Failed to close the program stream.";
        report -> type = ERROR;
    }

    *numberLinesRead = i;

    return words;
}