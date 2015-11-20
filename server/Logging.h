#ifndef __LOGGING__
#define __LOGGING__

#include <unistd.h>
#include "LogReport.h"

void saveLogReport(LogReport message);
void printLogReport(LogReport report);
void logFinalReport(int killCount);
void logParentInit();
void logProcessMonitoringInit(char* processName, pid_t pid);
void logProcessKill(pid_t pid, const char* name, unsigned long int duration);
void logSelfDying(pid_t pid, const char* name, unsigned long int duration);
void logSighupCatch(char* configFileName);
#endif