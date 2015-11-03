#ifndef __PROC_MANAGER__
#define __PROC_MANAGER__

#include "Logging.h"
#include "Utils.h"
#include "Process.h"
#include "MonitorRequest.h"
#include "RegisterEntry.h"

#define CHILD (pid_t)0

typedef struct 
{
	pid_t targetPid;
	unsigned long int monitorDuration;
} MonitorMessage;

extern MonitorRequest** monitorRequests;
extern int configLength;
extern RegisterEntry* root;

void cleanupGlobals();

Process** searchRunningProcesses(int* processesFound, const char* processName);

int getProcessesToMonitor(int argc, char** argv, MonitorRequest*** monitorRequests);

bool killProcess(Process process);
bool killOtherProcNannys();

int refreshRegisterEntries(RegisterEntry* head);

int monitor(int refreshRate, int argc, char** argv);
void setupMonitoring(bool isRetry, char* processName, unsigned long int duration, RegisterEntry* head, RegisterEntry* tail);
void killAllChildren(RegisterEntry* root);
void closeChildEndsOfPipes();

// TODO: Maybe a HashMap if time permits?

#endif