#ifndef __PROC_MANAGER__
#define __PROC_MANAGER__

#include "Logging.h"
#include "Utils.h"
#include "Process.h"
#include <time.h>
#include <sys/types.h>

typedef char ProcessStatusCode;

#define SIGKILL_CHILD SIGKILL
#define DIED (ProcessStatusCode)2
#define NOT_FOUND (ProcessStatusCode)1
#define KILLED (ProcessStatusCode)0
#define FAILED (ProcessStatusCode)-1
#define CHILD (pid_t)0

typedef struct registerEntry
{
	pid_t monitoringProcess;
	pid_t monitoredProcess;
	char* monitoredName;
	unsigned long int monitorDuration;
	time_t startingTime;
	bool isAvailable;
	int writeToChildFD;
	int readFromChildFD;
	struct registerEntry* next;
} RegisterEntry;

typedef struct
{
	char* processName;
	unsigned long int monitorDuration;
} MonitorRequest;

typedef struct 
{
	pid_t targetPid;
	unsigned long int monitorDuration;
} MonitorMessage;

extern MonitorRequest** monitorRequests;
extern int configLength;
extern RegisterEntry* root;

void cleanupGlobals();
MonitorRequest* constructMonitorRequest(char* requestString);
void destroyMonitorRequest(MonitorRequest* this);
void destroyMonitorRequestArray(MonitorRequest** requestArray, int size);

Process** searchRunningProcesses(int* processesFound, const char* processName);

int getProcessesToMonitor(int argc, char** argv, MonitorRequest*** monitorRequests);

bool killProcess(Process process);
bool killOtherProcNannys();

int refreshRegisterEntries(RegisterEntry* head);

int monitor(int refreshRate, int argc, char** argv);
void setupMonitoring(bool isRetry, char* processName, unsigned long int duration, RegisterEntry* head, RegisterEntry* tail);
void killAllChildren(RegisterEntry* root);
void closeChildEndsOfPipes();

RegisterEntry* constuctorRegisterEntry(pid_t monitoringProcess, Process* monitoredProcess, RegisterEntry* next);
RegisterEntry* destructorRegisterEntry(RegisterEntry* this);
void destructChain(RegisterEntry* root);

// TODO: Maybe a HashMap if time permits?
Process* findMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg, unsigned long int* duration);

#endif