#ifndef __SERVER_LOGGING__
#define __SERVER_LOGGING__

#include <unistd.h>
#include <stdint.h>
#include "ClientManager.h"
#include "ClientMessage.h"
#include "LogReport.h"

void saveLogReport(LogReport message);
void logFinalServerReport(Client* root);
void logClientInit(const char* clientName);
void logServerInfo(uint16_t port);
void logSighupCatch(char* configFileName);
void logUnexpectedClientMessageCode(int sock, ClientMessageStatusCode messageCode);
#endif