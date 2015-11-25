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
#endif