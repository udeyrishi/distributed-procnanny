#include "ProcNannyRegister.h"
#include <stdlib.h>

RegisterEntry* constuctorRegisterEntry(pid_t monitoringProcess, Process* monitoredProcess, RegisterEntry* next)
{
	RegisterEntry* entry = (RegisterEntry*)malloc(sizeof(RegisterEntry));
    entry->monitoringProcess = monitoringProcess;
    entry->monitoredProcess = monitoredProcess;
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
	if (this->monitoredProcess != NULL)
	{
		processDestructor(this->monitoredProcess);
	}
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


Process* findAndRemoveMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg)
{
	while (reg != NULL)
	{
		if (reg->monitoringProcess == monitoringProcess)
		{
			Process* monitoredProcess = reg->monitoredProcess;
			reg->monitoredProcess = NULL; 
			return monitoredProcess;
		}
		else
		{
			reg = reg->next;
		}
	}
	return NULL;
}