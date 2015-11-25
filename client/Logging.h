#ifndef __CLIENT_LOGGING__
#define __CLIENT_LOGGING__

#include <unistd.h>
#include <stdint.h>
#include "LogReport.h"

void initializeLogger(int serverSocket);
void saveLogReport(LogReport message, bool verbose);
void logFinalReport(int killCount);
//void logParentInit();
void logProcessMonitoringInit(char* processName, pid_t pid);
void logProcessKill(pid_t pid, const char* name, uint32_t duration);
void logSelfDying(pid_t pid, const char* name, uint32_t duration);

#endif