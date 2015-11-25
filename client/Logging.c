#include "Logging.h"
#include "ClientMessage.h"
#include "CommunicationManager.h"
#include <assert.h>
#include "Utils.h"
#include "memwatch.h"

static int __serverSocket = -1;

void initializeLogger(int serverSocket)
{
    __serverSocket = serverSocket;
}

void saveLogReport(LogReport message, bool verbose)
{
    if (message.type == INFO || message.type == ACTION)
    {
        if (!writeClientMessageStatusCode(__serverSocket, LOG_MESSAGE, saveLogReport) ||
            !writeLogMessage(__serverSocket, message, saveLogReport))
        {
            exit(-1);
        }

        if (verbose)
        {
            printLogReport(message);
        }
    }
    else
    {
        // TODO: maybe this is not the behaviour
        printLogReport(message);
    }
}

void logFinalReport(int killCount)
{
    assert(killCount >= 0);
    if (!writeClientMessageStatusCode(__serverSocket, INT_ACK, saveLogReport) ||
        !writeUInt(__serverSocket, (uint32_t)killCount, saveLogReport))
    {
        exit(-1);
    }
}

void logProcessMonitoringInit(char* processName, pid_t pid)
{
    LogReport report;
    char* message = stringJoin("Initializing monitoring of process '", processName);
    char* message2 = stringJoin(message, "' (PID ");
    free(message);
    message = stringNumberJoin(message2, (int)pid);
    free(message2);
    message2 = stringJoin(message, ")");
    free(message);
    message = NULL;
    report.message = message2;
    report.type = ACTION;
    saveLogReport(report, false);
    free(message2);
}

void logProcessKill(pid_t pid, const char* name, uint32_t duration)
{
    LogReport report;
    char* message = stringNumberJoin("PID ", pid);
    char* message2 = stringJoin(message, " (");
    free(message);
    message = stringJoin(message2, name);
    free(message2);
    message2 = stringJoin(message, ") killed after exceeding ");
    free(message);
    message = stringULongJoin(message2, duration);
    free(message2);
    message2 = stringJoin(message, " seconds");
    free(message);
    report.message = message2;
    report.type = ACTION;
    saveLogReport(report, false);
    free(report.message);
}

void logSelfDying(pid_t pid, const char* name, uint32_t duration)
{
    LogReport report;
    char* message = stringNumberJoin("PID ", pid);
    char* message2 = stringJoin(message, " (");
    free(message);
    message = stringJoin(message2, name);
    free(message2);
    message2 = stringJoin(message, ") died on its own by ");
    free(message);
    message = stringULongJoin(message2, duration);
    free(message2);
    message2 = stringJoin(message, " seconds");
    free(message);
    report.message = message2;
    report.type = INFO;
    saveLogReport(report, false);
    free(report.message);
}