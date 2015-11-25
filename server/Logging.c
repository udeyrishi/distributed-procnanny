#include <stdbool.h>
#include "Logging.h"
#include "Utils.h"
#include <string.h>
#include <stdio.h>
#include "memwatch.h"

const char* LOGFILE_ENV_VAR = "PROCNANNYLOGS";
const char* SERVER_INFO_LOGFILE_ENV_VAR = "PROCNANNYSERVERINFO";
const char* LOGFILE_FLASH = "\n===================PROCNANNY v2.0, Udey Rishi===================";

bool appendToFile(const char* path, const char* string)
{
    FILE *f = fopen(path, "a+");
    if (f == NULL)
    {
        LogReport report;
        report.message = "Failed to open log file.";
        report.type = ERROR;
        printLogReport(report);
        return false;
    }

    fprintf(f, "%s\n", string);
    fclose(f);
    return true;
}

void saveLogReport(LogReport report)
{
    char* output = getFormattedReport(report);
    const char* logFile = getenv(LOGFILE_ENV_VAR);
    if (logFile == NULL)
    {
        LogReport report;
        report.message = stringJoin("Environment variable not found: ", LOGFILE_ENV_VAR);
        report.type = ERROR;
        printLogReport(report);
        free(report.message);
    }
    else
    {
        appendToFile(logFile, output);
    }
    free(output);
}

void logSighupCatch(const char* configFileName)
{
    LogReport sighupReport;
    sighupReport.type = INFO;
    char* message = stringJoin("Caught SIGHUP. Configuration file '", configFileName);
    char* message2 = stringJoin(message, "' re-read and sent to all the clients.");
    free(message);
    message = NULL;
    sighupReport.message = message2;
    saveLogReport(sighupReport);
    printLogReport(sighupReport);
    free(sighupReport.message);
}

//private
void logFlash()
{
    const char* logFile = getenv(LOGFILE_ENV_VAR);
    if (logFile == NULL)
    {
        LogReport report;
        report.message = stringJoin("Environment variable not found: ", LOGFILE_ENV_VAR);
        report.type = ERROR;
        printLogReport(report);
        free(report.message);
    }
    else
    {
        appendToFile(logFile, LOGFILE_FLASH);
    }
}

void logClientInit(const char* clientName)
{
    LogReport report;
    report.message = stringJoin("New client initialised. Config file sent. Client node: ", clientName);
    report.type = INFO;
    saveLogReport(report);
    free(report.message);
}

char* getServerInfoMessage(uint16_t port)
{
    //PID 345 on node xyz, port 2309
    // Source: http://stackoverflow.com/questions/504810/how-do-i-find-the-current-machines-full-hostname-in-c-hostname-and-domain-info
    char hostname[1025];
    hostname[1024] = '\0';
    gethostname(hostname, 1024);

    char* c1 = stringNumberJoin("PID ", (int)getpid());
    char* c2 = stringJoin(c1, " on node ");
    free(c1);
    c1 = stringJoin(c2, hostname);
    free(c2);
    c2 = stringJoin(c1, ", port ");
    free(c1);
    c1 = stringNumberJoin(c2, port);
    free(c2);
    return c1;
}

char* getServerInfoLogFileMessage(uint16_t port)
{
    // NODE XYZ PID 345 PORT 2309
    char hostname[1025];
    hostname[1024] = '\0';
    gethostname(hostname, 1024);

    char* c1 = stringJoin("NODE ", hostname);
    char* c2 = stringJoin(c1, " PID ");
    free(c1);
    c1 = stringNumberJoin(c2, (int)getpid());
    free(c2);
    c2 = stringJoin(c1, " PORT ");
    free(c1);
    c1 = stringNumberJoin(c2, port);
    free(c2);
    return c1;
}

void logServerInfo(uint16_t port)
{
    logFlash();

    LogReport report;
    report.type = SERVER_INFO;
    report.message = getServerInfoMessage(port);
    saveLogReport(report);
    printLogReport(report);
    free(report.message);

    const char* serverInfoFile = getenv(SERVER_INFO_LOGFILE_ENV_VAR);
    if (serverInfoFile == NULL)
    {
        LogReport report2;
        report2.message = stringJoin("Environment variable not found: ", SERVER_INFO_LOGFILE_ENV_VAR);
        report2.type = ERROR;
        printLogReport(report2);
        free(report2.message);
    }
    else
    {
        char* c = getServerInfoLogFileMessage(port);
        appendToFile(serverInfoFile, c);
        free(c);
    }
}

void logUnexpectedClientMessageCode(int sock, ClientMessageStatusCode messageCode)
{
    LogReport report;
    report.type = DEBUG;
    char* c = stringNumberJoin("Non-LogReport message was sent by client, when nothing else was expected. Client: ", sock);
    char* c2 = stringJoin(c, ", code: ");
    free(c);
    c = NULL;
    report.message = stringNumberJoin(c2, (int)messageCode);
    free(c2);
    c2 = NULL;
    saveLogReport(report);
    free(report.message);
}

void logFinalServerReport(Client* root)
{
    LogReport report;
    report.type = INFO;
    char* rootMessage;
    bool allocated = false;

    if (root == NULL)
    {
        rootMessage = "Caught SIGINT. Exiting cleanly. No clients ever connected. Kill Count: 0";
    }
    else
    {
        rootMessage = "Caught SIGINT. Exiting cleanly. Kill counts:";
        while (root != NULL)
        {
            char* first = stringJoin(rootMessage, " (");
            if (allocated)
            {
                free(rootMessage);
                rootMessage = NULL;
            }
            else
            {
                allocated = true;
            }
            char* c = stringJoin(first, root->hostName);
            free(first);
            first = NULL;
            char* c2 = stringJoin(c, ", ");
            free(c);
            c = stringULongJoin(c2, (unsigned long int)root->finalKillCount);
            free(c2);
            c2 = stringJoin(c, ") ;");
            free(c);
            rootMessage = c2;
            root = root->nextClient;
        }
    }

    report.message = rootMessage;
    saveLogReport(report);
    printLogReport(report);
    if (allocated)
    {
        free(rootMessage);
    }
}