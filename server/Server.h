#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>
#include "Logging.h"

void logger(LogReport report, bool verbose);
void createServer(uint16_t port, int argc, char** argv);
#endif