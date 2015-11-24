#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "LogReport.h"

void logger(LogReport report, bool local);
int makeClientSocket(const char* hostName, int port);

#endif