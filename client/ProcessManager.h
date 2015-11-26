#ifndef __PROC_MANAGER__
#define __PROC_MANAGER__

#include "LogReport.h"
#include "MonitorRequest.h"
#include <stdbool.h>
#include <sys/types.h>

#define CHILD (pid_t)0

typedef struct
{
	pid_t targetPid;
	unsigned long int monitorDuration;
} MonitorMessage;

int monitor(int refreshRate, int serverSocket);
#endif