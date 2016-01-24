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

#include "ClientSocket.h"
#include "Logging.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <strings.h>
#include "memwatch.h"

int makeSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LogReport report;
        report.type = ERROR;
        report.message = "Cannot open socket";
        saveLogReport(report, true);
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
        saveLogReport(report, true);
        return -1;
    }
    return sock;
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
        saveLogReport(report, true);
        return -1;
    }

    struct sockaddr_in server = getServerInfo(host, port);
    return connectToServer(sock, (struct sockaddr*)&server, sizeof(server));
}