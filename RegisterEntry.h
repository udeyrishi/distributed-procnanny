#ifndef __REGISTER_ENTRY_H__
#define __REGISTER_ENTRY_H__

#include "Process.h"
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>

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

RegisterEntry* constuctorRegisterEntry(pid_t monitoringProcess, Process* monitoredProcess, RegisterEntry* next);
RegisterEntry* destructorRegisterEntry(RegisterEntry* this);
void destructChain(RegisterEntry* root);
Process* findMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg, unsigned long int* duration);


#endif