#include "ProcessManager.h"
#include "Logging.h"
#include "Utils.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "memwatch.h"

#define STARTING_ALLOCATION_SIZE 128

const char* PROGRAM_NAME = "procnanny";

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
            free(this->command);
        }

        char* newJoin = stringJoin(spaceAdded, commandPart);
        free(spaceAdded);

        this->command = newJoin;
    }
}

// Destructor for a process
void processDestructor(Process* this)
{
    free(this->user);
    free(this->tty);
    free(this->stat);
    free(this->start);
    free(this->time);
    free(this->command);
    free(this);
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

Process** searchRunningProcesses(int* processesFound, const char* processName)
{
    int i = 0;
    LogReport report;
    
    char* command = stringJoin("ps -u | grep ", processName);
    char** lines = getOutputFromProgram(command, &i, &report);
    free(command);
    
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
        freeOutputFromProgram(lines, i);
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

    for (source = 0; source < i; ++source)
    {
        Process* p = (Process*)malloc(sizeof(Process));
        processConstructor(lines[source], p);

        if (compareStrings(p->command, processName))
        {
            processes[destination++] = p;
        }
        else
        {
            processDestructor(p);
        }
    }

    freeOutputFromProgram(lines, i);

    *processesFound = destination;
    if (destination == 0)
    {
        destroyProcessArray(processes, destination);
        processes = NULL;
    }
    return processes;
}

void destroyProcessArray(Process** array, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        processDestructor(array[i]);
    }
    free(array);
}

char** readFile(const char* filePath, int* numberLinesRead, LogReport* report)
{
    char* catCall = stringJoin("cat ", filePath);
    char** lines = getOutputFromProgram(catCall, numberLinesRead, report);
    free(catCall);
    return lines;
}

int getProcessesToMonitor(int argc, char** argv, char*** configOutput)
{
    LogReport report;
    report.message = (char*)NULL;

    if (argc < 2)
    {
        report.message = "Config file path needed as argument.";
        report.type = ERROR;
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
        report.type = ERROR;
        saveLogReport(report);
        return -1;
    }

    *configOutput = config;
    return configLines;
}

bool killProcess(Process process)
{
    int result = kill(process.pid, SIGKILL);
    return (bool)(result == 0);
}

bool killOtherProcNannys()
{
    int num = 0;
    Process** procs = searchRunningProcesses(&num, PROGRAM_NAME);
    if (procs == NULL)
    {
        if (num == 0)
        {
            // Nothing found
            return true;
        }
        LogReport report;
        report.message = "Unexpected behaviour. Process** is NULL but count > 0";
        report.type = DEBUG;
        saveLogReport(report);
        return false;
    }

    int i;
    for(i = 0; i < num; ++i)
    {
        Process* p = procs[i];
        if (getpid() != p->pid)
        {
            LogReport report;
            report.message = stringNumberJoin("Another procnanny found. Killing it. PID: ", (int)p->pid);
            report.type = INFO;
            saveLogReport(report);
            free(report.message);
            
            if(!killProcess(*p))
            {
                report.message = stringNumberJoin("Failed to kill another procnanny. PID: ", (int)p->pid);
                report.type = ERROR;
                saveLogReport(report);
                free(report.message);
                return false;
            }
        }
    }

    destroyProcessArray(procs, num);
    return true;
}

pid_t monitor(char* processName, unsigned long int duration, ProcessStatusCode* statusCode, RegisterEntry* tailPointer)
{
    int num = 0;
    Process** procs = searchRunningProcesses(&num, processName);
    if (procs == NULL)
    {
        if (num == 0)
        {
            *statusCode = NOT_FOUND;
            return getpid();
        }
        exit(-1);
    }

    int i;
    
    // Start with current pid
    pid_t pid = getpid();
    
    for(i = 0; i < num; ++i)
    {
        Process* p = procs[i];
        
        if (p -> pid == getpid())
        {
            // If procnannys were killed in the beginning, but a new one was started in between and the user expects to track that. 
            // Should never happen/be done.
            LogReport report;
            report.message = "Config file had procnanny as one of the entries. It will be ignored if no other procnanny is found.";
            report.type = WARNING;
            saveLogReport(report);
            continue;
        }

        logProcessMonitoringInit(processName, p->pid);

        //RegisterEntry* newEntry;
        //Process* copy;
        //pid = p -> pid;
        switch (pid = fork())
        {
            case CHILD:
                sleep(duration);
                int newNum = 0;
                Process** processRecheck = searchRunningProcesses(&newNum, processName);
                if (processRecheck == NULL)
                {
                    if (newNum == 0)
                    {
                        *statusCode = DIED;
                    }
                    else
                    {
                        *statusCode = FAILED;
                    }
                }
                else
                {
                    int checkCounter;
                    for (checkCounter = 0; checkCounter < newNum; ++checkCounter)
                    {
                        if (processRecheck[checkCounter] -> pid == p -> pid)
                        {
                            if(killProcess(*p))
                            {
                                *statusCode = KILLED;
                            }
                            else
                            {
                                *statusCode = FAILED;
                            }
                            break;
                        }
                    }

                    if (checkCounter == newNum)
                    {
                        *statusCode = DIED;
                    }
                }
                destroyProcessArray(processRecheck, newNum);
                i = num;
                break;

            case -1:
                destroyProcessArray(procs, num);
                exit(-1);

            default:
                // parent
                //copy = (Process*)malloc(sizeof(Process));
                //copyProcess(copy, p);
                tailPointer->monitoringProcess = pid;
                tailPointer->monitoredProcess = p->pid;
                tailPointer->monitoredName = copyString(p->command);
                tailPointer->next = constuctorRegisterEntry((pid_t)0, NULL, NULL);
                tailPointer = tailPointer->next;
                //printf("%d", (int)tailPointer->next);
                //assert(tailPointer-> next == NULL);
                //tailPointer->next = newEntry;
                break;
        }
    }

    destroyProcessArray(procs, num);
    return pid;
}

RegisterEntry* constuctorRegisterEntry(pid_t monitoringProcess, Process* monitoredProcess, RegisterEntry* next)
{
    RegisterEntry* entry = (RegisterEntry*)malloc(sizeof(RegisterEntry));
    entry->monitoringProcess = monitoringProcess;
    if (monitoredProcess == NULL)
    {
        entry->monitoredProcess = (pid_t)0;
        entry->monitoredName = NULL;
    }
    else
    {
        entry->monitoredProcess = monitoredProcess->pid;
        entry->monitoredName = copyString(monitoredProcess->command);
    }
    entry->next = next;
    return entry;
}

RegisterEntry* destructorRegisterEntry(RegisterEntry* this)
{
    if (this == NULL)
    {
        return NULL;
    }
    RegisterEntry* next = this->next;
    free(this->monitoredName);
    free(this);
    return next;
}

void destructChain(RegisterEntry* root)
{
    while (root != NULL)
    {
        root = destructorRegisterEntry(root);
    }
}

// Assumes same monitoring won't be called 2x. TODO: fix this
Process* findMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg)
{
    while (reg != NULL)
    {
        if (reg->monitoringProcess == monitoringProcess)
        {
            Process* found = (Process*)malloc(sizeof(Process));
            found->command = reg->monitoredName;
            found->pid = reg->monitoredProcess;
            return found;
        }
        else
        {
            reg = reg->next;
        }
    }
    return NULL;
}