#include "Server.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <signal.h>
#include "CommunicationManager.h"
#include "MonitorRequest.h"
#include "ClientMessage.h"
#include <assert.h>
#include "Utils.h"
#include "ServerMessage.h"
#include "ClientManager.h"
#include "memwatch.h"

#define MAX_CONNECTIONS 5

int masterSocket = -1;
fd_set activeSockets;

MonitorRequest** monitorRequests = NULL;
const char* configLocation = NULL;
int configLength = -1;
bool sigintReceived = false;
bool sighupReceived = false;

void rereadConfig()
{
    destroyMonitorRequestArray(monitorRequests, configLength);
    monitorRequests = NULL;
    configLength = 0;
    configLength = getProcessesToMonitor(configLocation, &monitorRequests, logger);

    if (configLength < 0)
    {
        // Already logged
        exit(-1);
    }
}

//private
void destroyGlobals()
{
    destroyMonitorRequestArray(monitorRequests, configLength);
    configLength = -1;
    monitorRequests = NULL;
    cleanupClientChain();
}

void logger(LogReport report, bool verbose)
{
    saveLogReport(report);
    if (verbose)
    {
        printLogReport(report);
    }
}

int createNewInternetSocket(LoggerPointer saveLogReport)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LogReport report;
        report.message = "Failed to create a new socket.";
        report.type = ERROR;
        saveLogReport(report, false);
    }

    return sock;
}

//private
bool bindInternetSocketToPort(int sock, uint16_t port)
{
    struct sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(port);
    socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *) &socketAddress, sizeof(socketAddress)) < 0)
    {
        LogReport report;
        report.message = "Failed to bind socket to address.";
        report.type = ERROR;
        logger(report, false);
        return false;
    }

    return true;
}

// private
bool setupSocketToListen(int sock)
{
    if (listen(sock, MAX_CONNECTIONS) < 0)
    {
        LogReport report;
        report.message = "Failed to listen on main socket";
        report.type = ERROR;
        logger(report, false);
        return false;
    }

    return true;
}

//private
// Source: http://www.gnu.org/software/libc/manual/html_node/Inet-Example.html#Inet-Example
void makeServerSocket(uint16_t port)
{
    int sock = createNewInternetSocket(logger);
    if (sock < 0 || !bindInternetSocketToPort(sock, port) || !setupSocketToListen(sock))
    {
        exit(-1);
    }

    masterSocket = sock;
}

// private
void registerNewClient()
{
    struct sockaddr_in client;
    size_t size = sizeof(client);
    int clientSocket = accept(masterSocket, (struct sockaddr *)&client, &size);

    Client* newClient = addClient(&client, clientSocket, logger);

    if (clientSocket < 0)
    {
        LogReport report;
        report.message = stringNumberJoin("Failed to listen to new incoming client request on master socket: ", masterSocket);
        report.type = ERROR;
        logger(report, false);
        free(report.message);
        exit(-1);
    }
    FD_SET(clientSocket, &activeSockets);
    if (!sendConfig(clientSocket, configLength, monitorRequests, logger))
    {
        exit(-1);
    }

    logClientInit(newClient->hostName);
}

//private
void logClientMessage(int sock)
{
    Client* client = getClientBySocket(sock);
    if (client == NULL)
    {
        LogReport error;
        error.message = stringNumberJoin("LogReport message from unknown socket: ", sock);
        error.type = DEBUG;
        logger(error, false);
        free(error.message);
        exit(-1);
    }
    char* clientName = client->hostName;
    LogReport report = readLogMessage(sock, clientName, logger);
    logger(report, false);
    free(report.message);
}

//private
void dataReceivedCallback(int sock)
{
    if (sock == masterSocket)
    {
        registerNewClient();
    }
    else
    {
        ClientMessageStatusCode messageCode = readClientMessageStatusCode(sock, logger);
        if (messageCode == LOG_MESSAGE)
        {
            logClientMessage(sock);
        }
        else
        {
            logUnexpectedClientMessageCode(sock, messageCode);
            exit(-1);
        }
    }
}

void sendSigintAndUpdateKillCount(Client* client)
{
    ServerMessage intMessage;
    intMessage.type = INT;
    if (!sendMessage(intMessage, client->sock, logger))
    {
        exit(-1);
    }

    // clear up any pending LOG_MESSAGES that might have been generated in this time
    ClientMessageStatusCode messageCode;
    for (messageCode = readClientMessageStatusCode(client->sock, logger);
         messageCode == LOG_MESSAGE;
         messageCode = readClientMessageStatusCode(client->sock, logger))
    {
        logClientMessage(client->sock);
    }

    if (messageCode == INT_ACK)
    {
        client->finalKillCount = readUInt(client->sock, logger);
        close(client->sock);
    }
    else
    {
        logUnexpectedClientMessageCode(client->sock, messageCode);
        exit(-1);
    }
}

//private
void sigintHandler(int signum)
{
    if (signum == SIGINT)
    {
        sigintReceived = true;
    }
}

void sighupHandler(int signum)
{
    if (signum == SIGHUP)
    {
        sighupReceived = true;
    }
}

void sendHupMessage(Client* client)
{
    ServerMessage message;
    message.type = HUP;
    message.configLength = configLength;
    message.newConfig = monitorRequests;
    if (!sendMessage(message, client->sock, logger))
    {
        exit(-1);
    }
}

void sendPendingWrites()
{
    if (sighupReceived)
    {
        sighupReceived = false;
        rereadConfig();
        forEachClient(sendHupMessage);
    }
}

void createServer(uint16_t port, const char* configPath)
{
    // setup
    signal(SIGINT, sigintHandler);
    signal(SIGHUP, sighupHandler);
    configLocation = configPath;
    rereadConfig();
    makeServerSocket(port);
    FD_ZERO(&activeSockets);
    FD_SET(masterSocket, &activeSockets);

    // main loop
    manageReads(&activeSockets, NULL, &sigintReceived, &sighupReceived, sendPendingWrites, dataReceivedCallback, NULL, logger);

    // teardown
    close(masterSocket);
    forEachClient(sendSigintAndUpdateKillCount);
    logFinalServerReport(getRootClient());
    destroyGlobals();
}