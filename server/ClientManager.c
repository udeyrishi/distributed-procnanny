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

#include "ClientManager.h"
#include "Utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include "memwatch.h"

static Client* rootClient = NULL;
static Client* tailClient = NULL;

Client* addClient(const struct sockaddr_in* client, int _sock, LoggerPointer logger)
{
    Client* newClient = (Client*)malloc(sizeof(Client));
    LogReport report;
    if (!checkMallocResult(newClient, &report))
    {
        logger(report, false);
        exit(-1);
    }
    newClient->sock = _sock;
    newClient->nextClient = NULL;
    newClient->finalKillCount = (uint32_t)0;

    // Source: http://beej.us/guide/bgnet/output/html/multipage/getnameinfoman.html
    char hostName[1024]; // good enough
    char service[20]; // don't care
    getnameinfo((struct sockaddr*)client, sizeof(struct sockaddr_in), hostName, sizeof(hostName), service, sizeof(service), 0);
    newClient->hostName = copyString(hostName);

    if (tailClient == NULL)
    {
        rootClient = newClient;
        tailClient = newClient;
    }
    else
    {
        tailClient->nextClient = newClient;
        tailClient = newClient;
    }

    return newClient;
}

void cleanupClientChain()
{
    while (rootClient != NULL)
    {
        Client* temp = rootClient-> nextClient;
        free(rootClient->hostName);
        free(rootClient);
        rootClient = temp;
    }
    tailClient = NULL;
}

void forEachClient(ClientAction action)
{
    Client* current = rootClient;
    while (current != NULL)
    {
        action(current);
        current = current->nextClient;
    }
}

Client* getRootClient()
{
    return rootClient;
}

Client* getClientBySocket(int sock)
{
    Client* current = rootClient;

    while (current != NULL)
    {
        if (current->sock == sock)
        {
            return current;
        }
        else
        {
            current = current->nextClient;
        }
    }

    return NULL;
}