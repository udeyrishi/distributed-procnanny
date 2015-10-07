#include "ProcessReader.h"
#include "Logging.h"
#include "Utils.h"
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "memwatch.h"

#define STARTING_ALLOCATION_SIZE 128

// Constructor of a Process struct from a processString (output line from the ps command)
void processConstructor(char* processString, Process* this)
{
    int pid = atoi(strtok(processString, " "));
    this -> pid = (pid_t)pid;

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
void processDestructor(Process* this)
{
    safeFree(this->tty);
    safeFree(this->time);
    safeFree(this->cmd);
}

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
        report -> message = "Error running program.";
        report -> type = FATAL;
        return (char**)NULL;
    }

    int i;
    for (i = 0; TRUE; ++i)
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
        //getOutputFromProgram uses getline. Causes WILD free warning with memwatch if freed regularly   
        // Source: http://webdocs.cs.ualberta.ca/~paullu/C379/memwatch-2.71/FAQ
        safeMwFree(output[i]);
    }
    safeFree(output);
}

Process* getRunningProcesses(int* processesFound)
{
    int i;
    LogReport report;
    char** lines = getOutputFromProgram("ps", &i, &report);

    if (lines == NULL)
    {
        // LogReport has been filled with some error
        saveLogReport(report);
        return (Process*)NULL;
    }

    // i will always be >= 2, one for bash and one for procnanny. so i-1 >= 1 
    // i-1 because first line is just the heading
    Process* processes = (Process*)malloc(sizeof(Process)*(i-1));
    if (!checkMallocResult(processes, &report))
    {
        saveLogReport(report);
        return (Process*)NULL;
    }

    int j;
    for(j = 1; j < i; ++j)
    {
        processConstructor(lines[j], &processes[j-1]);
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
        processDestructor(&array[i]);
    }
    safeFree(array);
}

// private

char** readFile(const char* filePath, int* numberLinesRead, LogReport* report)
{
    char* catCall = stringJoin("cat ", filePath);
    char** lines = getOutputFromProgram(catCall, numberLinesRead, report);
    safeFree(catCall);
    return lines;
}

int getProcessesToMonitor(int argc, char** argv, char*** configOutput)
{
    LogReport report;
    report.message = (char*)NULL;

    if (argc < 2)
    {
        report.message = "Config file path needed as argument";
        report.type = FATAL;
        saveLogReport(report);
        return -1;
    }

    char* configPath = argv[1];
    int configLines = 0;
    char** config = readFile(configPath, &configLines, &report);
    
    if (report.message != NULL)
    {
        if (configLines > 0)
        {
            freeOutputFromProgram(config, configLines);
        }
        saveLogReport(report);
        return -1;
    }

    if (configLines < 2)
    {
        freeOutputFromProgram(config, configLines);
        report.message = "Bad config file. Number of lines should be greater than 1.";
        report.type = FATAL;
        saveLogReport(report);
        return -1;
    }

    *configOutput = config;
    return configLines;
}

Boolean killProcess(Process process)
{
    int result = kill(process.pid, SIGKILL);
    if (result == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}