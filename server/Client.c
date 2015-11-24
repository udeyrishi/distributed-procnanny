#include "Client.h"
#include "Utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include "memwatch.h"

static Client* rootClient = NULL;
static Client* tailClient = NULL;

void addClient(const struct sockaddr_in* client, int _sock, LoggerPointer logger)
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

    // Source: http://stackoverflow.com/questions/10236204/how-to-get-the-name-of-the-client-when-receiving-an-http-request
    struct hostent *hostName;
    struct in_addr ipv4addr;

    inet_pton(AF_INET, inet_ntoa(client->sin_addr), &ipv4addr);
    hostName = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    newClient->hostName = copyString(hostName->h_name);

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
}

void cleanupClientChain()
{
    while (rootClient != NULL)
    {
        Client* temp = rootClient-> nextClient;
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