#ifndef __PROC_MANAGER__
#define __PROC_MANAGER__

#include <stdbool.h>
#include <sys/types.h>

#define CHILD (pid_t)0

typedef struct 
{
	pid_t targetPid;
	unsigned long int monitorDuration;
} MonitorMessage;

bool killOtherProcNannys();
int monitor(int refreshRate, int argc, char** argv);
#endif