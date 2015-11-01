#ifndef __PROC_MANAGER__
#define __PROC_MANAGER__

#include "Logging.h"
#include "Utils.h"
#include <sys/types.h>

typedef int ProcessStatusCode;

#define DIED (ProcessStatusCode)2
#define NOT_FOUND (ProcessStatusCode)1
#define KILLED (ProcessStatusCode)0
#define FAILED (ProcessStatusCode)-1
#define CHILD (pid_t)0

typedef struct 
{
	char* user;
	pid_t pid;
	double cpu;
	double mem;
	int vsz;
	int rss;
	char* tty;
	char* stat;
	char* start;
	char* time;
	char* command;
} Process;

typedef struct registerEntry
{
	pid_t monitoringProcess;
	pid_t monitoredProcess;
	char* monitoredName;
	unsigned long int monitorDuration;
	struct registerEntry* next;
} RegisterEntry;

typedef struct
{
	char* processName;
	unsigned long int monitorDuration;
} MonitorRequest;

MonitorRequest* constructMonitorRequest(char* requestString);
void destroyMonitorRequest(MonitorRequest* this);
void destroyMonitorRequestArray(MonitorRequest** requestArray, int size);

char** getOutputFromProgram(const char* programName, int * numberLinesRead, LogReport* report); 
void freeOutputFromProgram(char** output, int numberLinesRead); 

Process** searchRunningProcesses(int* processesFound, const char* processName);

void destroyProcessArray(Process** array, int count);
Process* processConstructor(char* processString);
void processDestructor(Process* this);

char** readFile(const char* filePath, int* numberLinesRead, LogReport* report);

int getProcessesToMonitor(int argc, char** argv, MonitorRequest*** monitorRequests);

bool killProcess(Process process);
bool killOtherProcNannys();

pid_t monitor(char* processName, unsigned long int duration, ProcessStatusCode* statusCode, RegisterEntry* tailPointer);

RegisterEntry* constuctorRegisterEntry(pid_t monitoringProcess, Process* monitoredProcess, RegisterEntry* next);
RegisterEntry* destructorRegisterEntry(RegisterEntry* this);
void destructChain(RegisterEntry* root);

// TODO: Maybe a HashMap if time permits?
Process* findMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg, int* duration);

#endif