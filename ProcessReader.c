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

//private
char* getNextStrTokString(char* init)
{
    char* next = strtok(init, " ");
    int len = strlen(next);
    char* p = (char*)malloc(sizeof(char)*(len+1));
    strcpy(p, next);
    return p;
}

// Constructor of a Process struct from a processString (output line from the ps command)
void processConstructor(char* processString, Process* this)
{
    this->user = getNextStrTokString(processString);
    this->pid = atoi(strtok(NULL, " "));
    this->cpu = atof(strtok(NULL, " "));
    this->mem = atof(strtok(NULL, " "));
    this->vsz = atoi(strtok(NULL, " "));
    this->rss = atoi(strtok(NULL, " "));
    this->tty = getNextStrTokString(NULL);
    this->stat = getNextStrTokString(NULL);
    this->start = getNextStrTokString(NULL);
    this->time = getNextStrTokString(NULL);

    this->command = (char*)malloc(sizeof(char));
    *this->command = '\0';

    char* commandPart;
    while ((commandPart = strtok(NULL, " ")) != NULL)
    {
        char* spaceAdded;
        if (*this->command == '\0')
        {
            spaceAdded = this->command;
        }
        else
        {
            spaceAdded = stringJoin(this->command, " ");
            safeFree(this->command);
        }

        char* newJoin = stringJoin(spaceAdded, commandPart);
        safeFree(spaceAdded);

        this->command = newJoin;
    }
}

// Destructor for a process
void processDestructor(Process* this)
{
    safeFree(this->user);
    safeFree(this->tty);
    safeFree(this->stat);
    safeFree(this->start);
    safeFree(this->time);
    safeFree(this->command);
    safeFree(this);
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

Process** searchRunningProcesses(int* processesFound, const char* processName)
{
    int i = 0;
    LogReport report;
    
    char* command = stringJoin("ps -u | grep ", processName);
    char** lines = getOutputFromProgram(command, &i, &report);
    safeFree(command);
    
    if (lines == NULL)
    {
        // LogReport has been filled with some error
        saveLogReport(report);
        *processesFound = -1;
        return (Process**)NULL;
    }

    if (i <= 2)
    {
        // Nothing's found. 2 because 2 internal processes are started
        *processesFound = 0;
        return (Process**)NULL;
    }

    // i  > 2
    Process** processes = (Process**)malloc(sizeof(Process*)*(i-2));
    if (!checkMallocResult(processes, &report))
    {
        saveLogReport(report);
        freeOutputFromProgram(lines, i);
        *processesFound = -1;
        return (Process**)NULL;
    }

    int source;
    int destination = 0;

    char* command1 = stringJoin("sh -c ps -u | grep ", processName);
    char* command2 = stringJoin("grep ", processName);

    for (source = 0; source < i; ++source)
    {
        Process* p = (Process*)malloc(sizeof(Process));
        processConstructor(lines[source], p);
        if (compareStrings(p->command, command1) || compareStrings(p->command, command2))
        {
            processDestructor(p);
        }
        else
        {
            processes[destination++] = p;
        }
    }

    safeFree(command1);
    safeFree(command2);

    freeOutputFromProgram(lines, i);

    *processesFound = destination;
    return processes;
}

void destroyProcessArray(Process** array, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        processDestructor(array[i]);
    }
    safeFree(array);
}

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