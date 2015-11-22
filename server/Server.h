#ifndef __SERVER_H__
#define __SERVER_H__

#include "LogReport.h"
#include <stdint.h>

int makeServerSocket(uint16_t port, LoggerPointer saveLogReport);

#endif