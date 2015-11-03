#ifndef __PROC_MANAGER__
#define __PROC_MANAGER__

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

bool killOtherProcNannys();
int monitor(int refreshRate, int argc, char** argv);
#endif