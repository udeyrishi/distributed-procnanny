#include "ProcessReader.h"
#include "Logging.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "memwatch.h"

// Constructor of a Process struct from a processString (output line from the ps command)
void createProcess(char* processString, Process* this)
{
    int pid = atoi(strtok(processString, " "));
    this -> pid = pid;

    char* tty = strtok(NULL, " ");
    int ttyLen = strlen(tty);
    this -> tty = (char*)malloc(sizeof(char)*(ttyLen+1));
    strcpy(this->tty, tty);

    char* time = strtok(NULL, " ");
    int timeLen = strlen(time);
    this -> time = (char*)malloc(sizeof(char)*(timeLen+1));
    strcpy(this->time, time);

    char* cmd = strtok(NULL, " ");
    int cmdLen = strlen(cmd);
    this -> cmd = (char*)malloc(sizeof(char)*(cmdLen+1));
    strcpy(this->cmd, cmd);    
}

// Destructor for a process
void destroyProcess(Process* this)
{
    free(this->tty);
    free(this->time);
    free(this->cmd);
}

// Adapted from: http://stackoverflow.com/questions/19173442/reading-each-line-of-file-into-array
char** getOutputFromProgram(const char* programName, int maxNumberLines, int maxLineLength, int * numberLinesRead, LogReport* report) 
{
    char **lines = (char **)malloc(sizeof(char*)*maxNumberLines);
    
    if (!checkMallocResult(lines, report))
    {
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
    for (i = 0; i < maxNumberLines; ++i)
    {
        int j;
        // Allocate space for the next line
        lines[i] = malloc(maxLineLength);
        if (!checkMallocResult(lines[i], report))
        {
            return (char**)NULL;
        }

        if (fgets(lines[i], maxLineLength - 1, fp) == NULL)
        {
            free(lines[i]);
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
        report -> type = ERROR;
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
        free(output[i]);
    }
    free(output);
}

Process* getRunningProcesses(int maxNumberOfProcesses, int maxProcessLength, int* processesFound, LogReport* report)
{
    int i;
    char** lines = getOutputFromProgram("ps", maxNumberOfProcesses + 1, maxProcessLength, &i, report);

    if (lines == NULL)
    {
        // LogReport has been filled with some error
        return (Process*)NULL;
    }

    // i will always be >= 2, one for bash and one for procnanny. so i-1 >= 1 
    // i-1 because first line is just the heading
    Process* processes = (Process*)malloc(sizeof(Process)*(i-1));
    if (!checkMallocResult(processes, report))
    {
        return (Process*)NULL;
    }


    int j;
    for(j = 1; j < i; ++j)
    {
        createProcess(lines[j], &processes[j-1]);
    }

    freeOutputFromProgram(lines, i);

    *processesFound = i-1;

    return processes;
}

void destroyProcessArray(Process* array, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        destroyProcess(&array[i]);
    }
    free(array);
}

char** readFile(const char* filePath, int maxNumberLines, int maxLineLength, int* numberLinesRead, LogReport* report)
{
    // filePath size + size of "cat " + 1 for \0.
    int sizeBuffer = strlen(filePath) + 5;

    char* catCall = (char*)malloc(sizeof(char)*sizeBuffer);
    if (!checkMallocResult(catCall, report))
    {
        return (char**)NULL;
    }

    int n = sprintf(catCall, "cat %s", filePath);

    // Check: incorrect estimation of bufferSize in ProcessReader.readFile
    assert(n + 1 == sizeBuffer);

    char** lines = getOutputFromProgram(catCall, maxNumberLines, maxLineLength, numberLinesRead, report);
    free(catCall);
    return lines;
}
