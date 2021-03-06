/*
Copyright 2015 Udey Rishi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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