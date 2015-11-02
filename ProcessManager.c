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
Process* processConstructor(char* processString)
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
        Process* p = processConstructor(lines[source]);
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

char** readFile(const char* filePath, int* numberLinesRead, LogReport* report)
{
    char* catCall = stringJoin("cat ", filePath);
    char** lines = getOutputFromProgram(catCall, numberLinesRead, report);
    free(catCall);
    return lines;
}

int getProcessesToMonitor(int argc, char** argv, MonitorRequest*** monitorRequests)
{
    LogReport report;
    report.message = (char*)NULL;

    if (argc < 2)
    {
        report.message = "Config file path needed as argument.";
        report.type = ERROR;
        saveLogReport(report);
        printLogReport(report);
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

    // Array of MonitorRequest pointers
    MonitorRequest** requests = (MonitorRequest**)malloc(configLines*sizeof(MonitorRequest*));
    if (!checkMallocResult(requests, &report))
    {
        freeOutputFromProgram(config, configLines);
        saveLogReport(report);
        return -1;
    }

    int i;
    for (i = 0; i < configLines; ++i) 
    {
        MonitorRequest* request = constructMonitorRequest(config[i]);

        if (request == NULL)
        {
            freeOutputFromProgram(config, configLines);
            destroyMonitorRequestArray(requests, configLines);
            return -1;
        }
        requests[i] = request;
    }

    freeOutputFromProgram(config, configLines);
    *monitorRequests = requests;
    return configLines;
}

MonitorRequest* constructMonitorRequest(char* requestString)
{
    MonitorRequest* this = (MonitorRequest*)malloc(sizeof(MonitorRequest));
    LogReport report;
    if (!checkMallocResult(this, &report))
    {
        saveLogReport(report);
        return NULL;
    }

    this->processName = getNextStrTokString(requestString);
    char* monitorDuration = getNextStrTokString(NULL);
    this->monitorDuration = strtoul(monitorDuration, NULL, 10);
    free(monitorDuration);
    return this;
}

void destroyMonitorRequest(MonitorRequest* this)
{
    free(this->processName);
    free(this);
}

void destroyMonitorRequestArray(MonitorRequest** requestArray, int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        destroyMonitorRequest(requestArray[i]);
    }

    free(requestArray);
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
    procs = NULL;

    int verificationNum;
    Process** verificationProcs = searchRunningProcesses(&verificationNum, PROGRAM_NAME);
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

    LogReport report;
    report.message = "Sent kill signal to other procnannys, but they didn't die.";
    report.type = ERROR;
    saveLogReport(report);
    destroyProcessArray(verificationProcs, verificationNum);
    return false;
}

//private
bool isHeadNull(RegisterEntry* head)
{
    return (head == NULL || head->monitoringProcess == (pid_t)0);
}

//private 
bool isProcessAlreadyBeingMonitored(pid_t pid, RegisterEntry* reg)
{
    while (!isHeadNull(reg))
    {
        if (reg->monitoredProcess == pid)
        {
            return true;
        }
        else
        {
            reg = reg->next;
        }
    }
    return false;
}

void refreshRegisterEntries(RegisterEntry* head)
{
    if (isHeadNull(head))
    {
        return;
    }

    time_t currentTime = time(NULL);
    while (head != NULL)
    {
        if (!(head->isAvailable) && (currentTime > head->startingTime + head->monitorDuration))
        {
            ProcessStatusCode message;
            assert(read(head->readFromChildFD, &message, 1) == 1);

            LogReport report;
            switch(message)
            {
                case DIED:
                    logSelfDying(head->monitoredProcess, head->monitoredName, head->monitorDuration);
                    break;

                case KILLED:
                    logProcessKill(head->monitoredProcess, head->monitoredName, head->monitorDuration);
                    break;

                case FAILED:
                    report.message = stringNumberJoin("Failed to kill process with PID: ", (int)head->monitoredProcess);
                    report.type = ERROR;
                    saveLogReport(report);
                    free(report.message);
                    break;

                default:
                    // TODO: UNEXPECTED
                    break;
            }

            head->isAvailable = true;
        }
        head = head->next;
    }
}

//private
RegisterEntry* getFirstFreeChild(RegisterEntry* head)
{
    while (!isHeadNull(head))
    {
        if (head->isAvailable)
        {
            return head;
        }
        else 
        {
            head = head->next;
        }
    }

    return NULL;
}

//private
ProcessStatusCode childMain(pid_t pid, int duration)
{
    sleep(duration);
    int result = kill(pid, SIGKILL);
    if (result == 0)
    {
        return KILLED;
    }
    else
    {
        return DIED;
    }
}

void setupMonitoring(char* processName, unsigned long int duration, RegisterEntry* head, RegisterEntry* tail)
{
    int num = 0;
    Process** runningProcesses = searchRunningProcesses(&num, processName);
    if (runningProcesses == NULL)
    {
        // Nothing to be done
        if (num == 0)
        {
            LogReport report;
            report.message = stringJoin("No process found with name: ", processName);
            report.type = INFO;
            saveLogReport(report);
            free(report.message);
            return;
        }
        exit(-1);
    }

    int i;
    for(i = 0; i < num; ++i)
    {
        Process* p = runningProcesses[i];

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

        if (isProcessAlreadyBeingMonitored(p->pid, head))
        {
            continue;
        }

        logProcessMonitoringInit(processName, p->pid);

        RegisterEntry* freeChild = getFirstFreeChild(head);
        if (freeChild == NULL)
        {
            // fork a new child
            int writeToChildFD[2];
            int readFromChildFD[2];
            if (pipe(writeToChildFD) < 0)
            {
                destroyProcessArray(runningProcesses, num);
                LogReport report;
                report.message = "Pipe creation error when trying to monitor new process.";
                report.type = ERROR;
                saveLogReport(report);
                printLogReport(report);
                // TODO: Kill all children
                // TODO: Log all final kill count
                exit(-1);
            }

            if (pipe(readFromChildFD) < 0)
            {
                destroyProcessArray(runningProcesses, num);
                LogReport report;
                report.message = "Pipe creation error when trying to monitor new process.";
                report.type = ERROR;
                saveLogReport(report);
                printLogReport(report);
                // TODO: Kill all children
                // TODO: Log all final kill count
                exit(-1);
            }

            pid_t forkPid = fork();
            switch (forkPid)
            {
                case -1:
                    destroyProcessArray(runningProcesses, num);
                    exit(-1);

                case CHILD:
                    close(writeToChildFD[1]);
                    close(readFromChildFD[0]);
                    int writing = readFromChildFD[1];
                    int reading = writeToChildFD[0];
                    pid_t targetPid = p->pid;
                    destroyProcessArray(runningProcesses, num);
                    
                    while (true)
                    {
                        ProcessStatusCode childStatus = childMain(targetPid, duration);
                        write(writing, &childStatus, 1);

                        MonitorMessage message;
                        assert(read(reading, &message, sizeof(MonitorMessage)) == sizeof(MonitorMessage));
                        targetPid = message.targetPid;
                        duration = message.monitorDuration;
                    }
                    
                    break;

                default:
                     // parent
                    close(writeToChildFD[0]);
                    close(readFromChildFD[1]);
                    tail->monitoringProcess = forkPid;
                    tail->monitoredProcess = p->pid;
                    tail->monitorDuration = duration;
                    tail->monitoredName = copyString(p->command);
                    tail->startingTime = time(NULL);
                    tail->isAvailable = false;
                    tail->writeToChildFD = writeToChildFD[1];
                    tail->readFromChildFD = readFromChildFD[0];
                    tail->next = constuctorRegisterEntry((pid_t)0, NULL, NULL);
                    tail = tail->next;
                    break;
            }

        }
        else
        {
            // use freeChild
            freeChild->isAvailable = false;
            freeChild->monitoredProcess = p->pid;
            free(freeChild->monitoredName);
            freeChild->monitoredName = copyString(p->command);
            freeChild->monitorDuration = duration;
            freeChild->startingTime = time(NULL);
            MonitorMessage message;
            message.targetPid = p->pid;
            message.monitorDuration = duration;
            write(freeChild->writeToChildFD, &message, sizeof(MonitorMessage));
        }
    }

    destroyProcessArray(runningProcesses, num);
}

/*
pid_t monitor(char* processName, unsigned long int duration, ProcessStatusCode* statusCode, RegisterEntry* head,
              RegisterEntry* tailPointer)
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

        // find p->pid starting at head. If found, skip it, else monitor
        if (isProcessAlreadyBeingMonitored(p->pid, head))
        {
            continue;
        }

        logProcessMonitoringInit(processName, p->pid);

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
                tailPointer->monitoringProcess = pid;
                tailPointer->monitoredProcess = p->pid;
                tailPointer->monitorDuration = duration;
                tailPointer->monitoredName = copyString(p->command);
                tailPointer->next = constuctorRegisterEntry((pid_t)0, NULL, NULL);
                tailPointer = tailPointer->next;
                break;
        }
    }

    destroyProcessArray(procs, num);
    return pid;
}
*/

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

Process* findMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg, unsigned long int* duration)
{
    while (reg != NULL)
    {
        if (reg->monitoringProcess == monitoringProcess)
        {
            Process* found = (Process*)malloc(sizeof(Process));
            found->command = reg->monitoredName;
            found->pid = reg->monitoredProcess;
            *duration = reg->monitorDuration;
            return found;
        }
        else
        {
            reg = reg->next;
        }
    }
    return NULL;
}