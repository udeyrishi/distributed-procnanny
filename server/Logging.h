#ifndef __SERVER_LOGGING__
#define __SERVER_LOGGING__

#include <unistd.h>
#include <stdint.h>
#include "ClientManager.h"
#include "ClientMessage.h"
#include "LogReport.h"

void saveLogReport(LogReport message);
void logFinalServerReport(Client* root);
//void logFinalReport(int killCount); // different form though
void logClientInit(const char* clientName);
void logServerInfo(uint16_t port);
//void logProcessMonitoringInit(char* processName, pid_t pid);
//void logProcessKill(pid_t pid, const char* name, unsigned long int duration);
//void logSelfDying(pid_t pid, const char* name, unsigned long int duration);
void logSighupCatch(char* configFileName);
void logUnexpectedClientMessageCode(int sock, ClientMessageStatusCode messageCode);
#endif