#include "Process.h"
#include "Utils.h"
#include "ProgramIO.h"
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "memwatch.h"

// Constructor of a Process struct from a processString (output line from the ps command)
Process* processConstructor(char* processString, LoggerPointer saveLogReport)
{
    Process* this = (Process*)malloc(sizeof(Process));
    LogReport report;
    if (!checkMallocResult(this, &report))
    {
        saveLogReport(report);
        return (Process*)NULL;
    }

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
    return this;
}

// Destructor for a process
void processDestructor(Process* this)
{
    if (this == NULL)
    {
        return;
    }
    free(this->user);
    free(this->tty);
    free(this->stat);
    free(this->start);
    free(this->time);
    free(this->command);
    free(this);
}

void destroyProcessArray(Process** array, int count)
{
    if (array == NULL) 
    {
        return;
    }

    int i;
    for (i = 0; i < count; ++i)
    {
        processDestructor(array[i]);
    }
    free(array);
}

bool killProcess(Process process)
{
    int result = kill(process.pid, SIGKILL);
    return (bool)(result == 0);
}

Process** searchRunningProcesses(int* processesFound, const char* processName, bool ignoreCmdOptions, LoggerPointer saveLogReport)
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
        Process* p = processConstructor(lines[source], saveLogReport);
        if (p == NULL)
        {
            freeOutputFromProgram(lines, i);
            *processesFound = -1;
            return (Process**)NULL;
        }
        if (compareStrings(p->command, processName))
        {
            processes[destination++] = p;
        }
        else if (ignoreCmdOptions)
        {
            char* temp = copyString(p->command);
            char* part = strtok(temp, " ");
            if (compareStrings(part, processName))
            {
                processes[destination++] = p;
            }
            else
            {
                processDestructor(p);
            }
            free(temp);
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

//private
bool killOtherProcessAndVerifyExact(const char* programName, LoggerPointer saveLogReport)
{
    int num = 0;
    Process** procs = searchRunningProcesses(&num, programName, true, saveLogReport);
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
            char* c = stringJoin("Another ", programName);
            char* c2 = stringJoin(c, " found. Killing it. PID: ");
            free(c);
            c = NULL;

            report.message = stringNumberJoin(c2, (int)p->pid);
            free(c2);
            c2 = NULL;

            report.type = INFO;
            saveLogReport(report);
            free(report.message);
            
            if(!killProcess(*p))
            {
                c = stringJoin("Failed to kill another ", programName);
                c2 = stringJoin(c, ". PID: "); 
                free(c);
                c = NULL;

                report.message = stringNumberJoin(c2, (int)p->pid);
                free(c2);
                c2 = NULL;

                report.type = ERROR;
                saveLogReport(report);
                free(report.message);
                return false;
            }
        }
    }

    destroyProcessArray(procs, num);
    procs = NULL;

    int verificationNum;
    Process** verificationProcs = searchRunningProcesses(&verificationNum, programName, true, saveLogReport);
    if (verificationProcs == NULL)
    {
        if (verificationNum == 0)
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

    bool result;
    if (verificationNum == 1 && verificationProcs[0]->pid == getpid())
    {
        result = true;
    }
    else
    {
        LogReport report;
        report.message = "Sent kill signal to other procnannys, but they didn't die.";
        report.type = ERROR;
        saveLogReport(report);
        result = false;
    }

    destroyProcessArray(verificationProcs, verificationNum);
    return result;
}

bool killOtherProcessAndVerify(const char* programName, LoggerPointer saveLogReport)
{
    bool first = killOtherProcessAndVerifyExact(programName, saveLogReport);
    char* nameWithSlash = stringJoin("./", programName);
    bool second = killOtherProcessAndVerifyExact(nameWithSlash, saveLogReport);
    free(nameWithSlash);
    return first || second;
}