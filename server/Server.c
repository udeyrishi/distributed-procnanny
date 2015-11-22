#include "Server.h"
#include "Socket.h"
#include <netinet/in.h>
#include <stdbool.h>
#include "memwatch.h"

#define MAX_CONNECTIONS 5

bool bindInternetSocketToPort(int sock, uint16_t port, LoggerPointer saveLogReport)
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

bool setupSocketToListen(int sock, LoggerPointer saveLogReport)
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

// Source: http://www.gnu.org/software/libc/manual/html_node/Inet-Example.html#Inet-Example
int makeServerSocket(uint16_t port, LoggerPointer saveLogReport)
{
    int sock = createNewInternetSocket(saveLogReport);
    if (sock < 0 || !bindInternetSocketToPort(sock, port, saveLogReport) || !setupSocketToListen(sock, saveLogReport))
    {
        return -1;
    }

    return sock;
}