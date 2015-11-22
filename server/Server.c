#include "Server.h"
#include "Socket.h"
#include <netinet/in.h>
#include <stdbool.h>
#include "CommunicationManager.h"
#include "MonitorRequest.h"
#include <assert.h>
#include "Utils.h"
#include "memwatch.h"

#define MAX_CONNECTIONS 5

int masterSocket = -1;
fd_set clients;
MonitorRequest** monitorRequests = NULL;
int configLength = -1;

void rereadConfig(int argc, char** argv)
{
    destroyMonitorRequestArray(monitorRequests, configLength);
    monitorRequests = NULL;
    configLength = 0;
    configLength = getProcessesToMonitor(argc, argv, &monitorRequests, logger);

    if (configLength == -1)
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
        saveLogReport(report);
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
        saveLogReport(report);
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
void sendConfigToClient(int sock)
{
    assert(configLength >= 0);
    writeUInt(sock, (uint32_t)configLength);
    int i;
    for (i = 0; i < configLength; ++i)
    {
        writeString(sock, monitorRequests[i]->processName);
        writeUInt(sock, monitorRequests[i]->monitorDuration);
    }
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
        saveLogReport(report);
        free(report.message);
        exit(-1);
    }
    FD_SET(clientSocket, &clients);
    sendConfigToClient(clientSocket);
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
    size_t size = readData(sock, (char *)&(report.type), sizeof(report.type));
    if (size < 0)
    {
        LogReport error;
        error.message = stringNumberJoin("Reading data failed from socket: ", sock);
        error.type = ERROR;
        saveLogReport(error);
        free(error.message);
        exit(-1);
    }
    assert(size == sizeof(report.type));
    saveLogReport(report);
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
        logClientMessage(sock);
    }
}

void closeSockets()
{
    close(masterSocket);
    masterSocket = -1;
    int i;
    for (i = 0; i < FD_SETSIZE; ++i)
    {
        if (FD_ISSET(i, &clients))
        {
            close(i);
        }
    }
}

void createServer(uint16_t port, int argc, char** argv)
{
    rereadConfig(argc, argv);
    makeServerSocket(port);
    // manageReads
    destroyGlobals();
    closeSockets();
}