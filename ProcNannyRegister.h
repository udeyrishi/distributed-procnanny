#ifndef __PROC_NANNY_REGISTER__
#define __PROC_NANNY_REGISTER__

#include <sys/types.h>
#include "ProcessReader.h"

RegisterEntry* constuctorRegisterEntry(pid_t monitoringProcess, Process* monitoredProcess, RegisterEntry* next);
RegisterEntry* destructorRegisterEntry(RegisterEntry* this);
void destructChain(RegisterEntry* root);
// TODO: Maybe a HashMap if time permits?
Process* findMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg);

#endif