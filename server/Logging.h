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

#ifndef __SERVER_LOGGING__
#define __SERVER_LOGGING__

#include "ClientManager.h"
#include "ClientMessage.h"
#include "LogReport.h"
#include <stdint.h>
#include <unistd.h>

void saveLogReport(LogReport message);
void logFinalServerReport(Client* root);
void logClientInit(const char* clientName);
void logServerInfo(uint16_t port);
void logSighupCatch(const char* configFileName);
void logUnexpectedClientMessageCode(int sock, ClientMessageStatusCode messageCode);
#endif