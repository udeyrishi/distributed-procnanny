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

#ifndef __CLIENT_MESSAGE__
#define __CLIENT_MESSAGE__

#include "LogReport.h"

typedef char ClientMessageStatusCode;

#define LOG_MESSAGE (ClientMessageStatusCode)0xFFFF
#define INT_ACK (ClientMessageStatusCode)0xFFFE

ClientMessageStatusCode readClientMessageStatusCode(int sock, LoggerPointer logger);
bool writeClientMessageStatusCode(int sock, ClientMessageStatusCode statusCode, LoggerPointer logger);
LogReport readLogMessage(int sock, const char* clientName, LoggerPointer logger);
bool writeLogMessage(int serverSocket, LogReport report, LoggerPointer logger);
#endif