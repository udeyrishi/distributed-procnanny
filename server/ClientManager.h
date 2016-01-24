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

#ifndef __CLIENT_MANAGER__
#define __CLIENT_MANAGER__

#include "LogReport.h"
#include <stdint.h>
#include <netinet/in.h>

typedef struct __Client
{
	int sock;
	char* hostName;
	struct __Client* nextClient;
	uint32_t finalKillCount;
} Client;

typedef void (* ClientAction)(Client*);

Client* addClient(const struct sockaddr_in* client, int _sock, LoggerPointer logger);
void cleanupClientChain();
void forEachClient(ClientAction action);

Client* getRootClient();
Client* getClientBySocket(int sock);
#endif