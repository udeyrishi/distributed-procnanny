#ifndef __SERVER_MESSAGE__
#define __SERVER_MESSAGE__

#include "MonitorRequest.h"
#include "LogReport.h"
#include <stdbool.h>

typedef enum { HUP, INT } MessageType;

typedef struct
{
	MessageType type;
	int configLength;
	MonitorRequest** newConfig;
} ServerMessage;

bool sendMessage(ServerMessage message, int sock, LoggerPointer logger);
ServerMessage readMessage(int sock, LoggerPointer logger);

#endif