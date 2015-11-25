#include "Logging.h"
#include <stdlib.h>
#include "Process.h"
#include "ProcessManager.h"
#include "ClientSocket.h"
#include "CommunicationManager.h"
#include "memwatch.h"

#define REFRESH_RATE 5
const char* PROGRAM_NAME = "procnanny.client";

int main(int argc, char** argv)
{
    // TODO: logParentInit();
    if (argc < 3)
    {
        LogReport report;
        report.message = "Server host name and port number needed as arguments.";
        report.type = ERROR;
        saveLogReport(report, true);
        return -1;
    }

    if (!killOtherProcessAndVerify(PROGRAM_NAME, saveLogReport))
    {
        exit(-1);
    }

    char* serverHostName = argv[1];
    int serverPort = atoi(argv[2]);
    int sock = makeClientSocket(serverHostName, serverPort);
    if (sock < 0)
    {
        return -1;
    }
    initializeLogger(sock);

    int killCount = monitor(REFRESH_RATE, sock);
    logFinalReport(killCount);
    close(sock);
    return 0;
}