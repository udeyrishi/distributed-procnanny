#ifndef __SERVER_CLIENT__
#define __SERVER_CLIENT__

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

void addClient(const struct sockaddr_in* client, int _sock, LoggerPointer logger);
void cleanupClientChain();
void forEachClient(ClientAction action);

Client* getRootClient();
#endif