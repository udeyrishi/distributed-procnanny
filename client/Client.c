#include "Client.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <strings.h>
#include "memwatch.h"

void logger(LogReport report, bool local)
{
    if (local)
    {
        printLogReport(report);
    }
    else
    {
        // TODO: send over network
    }
}

int makeSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LogReport report;
        report.type = ERROR;
        report.message = "Cannot open socket";
        logger(report, true);
        return -1;
    }
    return sock;
}

struct sockaddr_in getServerInfo(const struct hostent* host, int port)
{
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    bcopy(host->h_addr, &(server.sin_addr), host->h_length);
    server.sin_family = host->h_addrtype;
    server.sin_port = htons(port);

    return server;
}

int connectToServer(int sock, const struct sockaddr* server, size_t size)
{
    if (connect(sock, server, size) < 0) 
    {
        LogReport report;
        report.type = ERROR;
        report.message = "Cannot connect to server";
        logger(report, true);
        return -1;
    }
    return 0;
}

int makeClientSocket(const char* hostName, int port)
{
    int sock = makeSocket();
    if (sock < 0)
    {
        return sock;
    }

    struct hostent* host = gethostbyname(hostName);
    if (host == NULL)
    {
        LogReport report;
        report.type = ERROR;
        report.message = "Cannot get host description";
        logger(report, true);
        return -1;
    }

    struct sockaddr_in server = getServerInfo(host, port);
    return connectToServer(sock, (struct sockaddr*)&server, sizeof(server));
}