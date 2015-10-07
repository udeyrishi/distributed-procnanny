#include "ProcessReader.h"
#include "Logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    
    if (lines == NULL)
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
    for (i = 0; i < maxNumberLines; ++i)
    {
        int j;
        // Allocate space for the next line
        lines[i] = malloc(maxLineLength);
        if (lines[i] == NULL)
        {
            report -> message = "Out of memory.";
            report -> type = FATAL;
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

Process* getRunningProcesses(int maxNumberOfProcesses, int maxProcessLength, int* processesFound, LogReport* report)
{
    int i;
    char** lines = getOutputFromProgram("ps", maxNumberOfProcesses, maxProcessLength, &i, report);

    if (lines == NULL)
    {
        // LogReport has been filled with some error
        return (Process*)NULL;
    }

    // i-1 because first line is just the heading
    Process* processes = (Process*)malloc(sizeof(Process)*(i-1));
    
    if (processes == NULL)
    {
        report -> message = "Out of memory.";
        report -> type = FATAL;
        return (Process*)NULL;
    }

    free(lines[0]);

    int j;
    for(j = 1; j < i; ++j)
    {
        createProcess(lines[j], &processes[j-1]);
        free(lines[j]);
    }

    free(lines);

    *processesFound = i-1;

    return processes;
}