#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>
#include "Logging.h"

void logger(LogReport report, bool verbose);
int createServer(uint16_t port, const char* configPath);
#endif