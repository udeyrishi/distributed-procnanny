#include "Server.h"
#include "Socket.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <signal.h>
#include "CommunicationManager.h"
#include "MonitorRequest.h"
#include "ClientMessage.h"
#include <assert.h>
#include "Utils.h"
#include "ServerMessage.h"
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
}

void logger(LogReport report, bool verbose)
{
    saveLogReport(report);
    if (verbose)
    {
        printLogReport(report);
    }
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
}

//private
void logClientMessage(int sock)
{
    LogReport report;
    report.message = readString(sock, logger);
    if (report.message < 0)
    {
        exit(-1);
    }
    size_t size = readData(sock, (char *)&(report.type), sizeof(report.type), logger);
    if (size < 0)
    {
        exit(-1);
    }
    assert(size == sizeof(report.type));
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
        char messageCode = readClientMessageStatusCode(sock, logger);
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

void sendSigintToClients()
{
    int i;
    for (i = 0; i < FD_SETSIZE; ++i)
    {
        if (i != masterSocket && FD_ISSET(i, &activeSockets))
        {
            ServerMessage intMessage;
            intMessage.type = INT;
            if (!sendMessage(intMessage, i, logger))
            {
                exit(-1);
            }

            // clear up any pending LOG_MESSAGES that might have been generated in this time
            char messageCode;
            for (messageCode = readClientMessageStatusCode(i, logger); 
                 messageCode == LOG_MESSAGE; 
                 messageCode = readClientMessageStatusCode(i, logger))
            {
                logClientMessage(i);
            }

            if (messageCode == INT_ACK)
            {
                close(i);
            }
            else
            {
                logUnexpectedClientMessageCode(i, messageCode);
                exit(-1);
            }
        }
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

void sendPendingWrites()
{
    if (sighupReceived)
    {
        sighupReceived = false;
        rereadConfig();
        int i;
        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (i != masterSocket && FD_ISSET(i, &activeSockets))
            {
                // send with status. Either new config or SIGINT
                ServerMessage message;
                message.type = HUP;
                message.configLength = configLength;
                message.newConfig = monitorRequests;
                if (!sendMessage(message, i, logger))
                {
                    exit(-1);
                }
            }
        }

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
    FD_ZERO (&activeSockets);
    FD_SET (masterSocket, &activeSockets);

    // main loop
    manageReads(&activeSockets, NULL, &sigintReceived, &sighupReceived, sendPendingWrites, dataReceivedCallback, NULL, logger);    

    // teardown
    destroyGlobals();
    close(masterSocket);
    sendSigintToClients();
}