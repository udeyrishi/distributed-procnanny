#include "Socket.h"
#include <netinet/in.h>
#include "memwatch.h"

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