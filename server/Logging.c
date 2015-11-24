#include <stdio.h>
#include "Logging.h"
#include "Utils.h"
#include <time.h>
#include <string.h>
#include "memwatch.h"

const char* LOGFILE_ENV_VAR = "PROCNANNYLOGS";
const char* SERVER_INFO_LOGFILE_ENV_VAR = "PROCNANNYSERVERINFO";
const char* LOGFILE_FLASH = "\n===================PROCNANNY v2.0, Udey Rishi===================";

// private
char* getTime()
{
    char* output;
    time_t rawtime;
    time (&rawtime);
    char* t = ctime(&rawtime);
    t[strlen(t) - 1] = '\0';
    output = stringJoin("[", t);
    char* output2 = stringJoin(output, "] ");
    free(output);
    return output2;
}

// private
char* getFormattedReport(LogReport report)
{
    char* output;
    char* currentTime = getTime();
    switch(report.type)
    {
        case ERROR:
            output = stringJoin(currentTime, "Error: ");
            break;
        case WARNING:
            output = stringJoin(currentTime, "Warning: ");
            break;
        case ACTION:
            output = stringJoin(currentTime, "Action: ");
            break;
        case FATAL:
            output = stringJoin(currentTime, "Fatal: ");
            break;
        case INFO:
            output = stringJoin(currentTime, "Info: ");
            break;
        case DEBUG:
            output = stringJoin(currentTime, "Debug: ");
            break;
        case SERVER_INFO:
            output = stringJoin(currentTime, "procnanny server: ");
            break;
        default:
            output = stringJoin(currentTime, "Unknown: ");
            break;
    } 
    free(currentTime);
    currentTime = NULL;
    char* output2 = stringJoin(output, report.message);
    free(output);
    return output2;
}

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

void printLogReport(LogReport report)
{
    char* output = getFormattedReport(report);
    printf("%s\n", output);
    free(output);
}

// TODO: Maybe for next part, write an easier method for general purpose string sizes...
void logFinalReport(int killCount)
{
    LogReport report;
    report.type = INFO;
    char* message = stringNumberJoin("Caught SIGINT. Exiting cleanly. ", killCount);
    char* message2 = stringJoin(message, " process(es) killed.");
    free(message);
    report.message = message2;
    saveLogReport(report);
    printLogReport(report);
    free(message2);
}

void logProcessMonitoringInit(char* processName, pid_t pid)
{
    LogReport report;
    char* message = stringJoin("Initializing monitoring of process '", processName); 
    char* message2 = stringJoin(message, "' (PID ");
    free(message);
    message = stringNumberJoin(message2, (int)pid);
    free(message2);
    message2 = stringJoin(message, ").");
    free(message);
    message = NULL;
    report.message = message2;
    report.type = ACTION;
    saveLogReport(report);
    free(message2);
}

void logProcessKill(pid_t pid, const char* name, unsigned long int duration)
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
    message2 = stringJoin(message, " seconds.");
    free(message);
    report.message = message2;
    report.type = ACTION;
    saveLogReport(report);
    free(report.message);
}

void logSelfDying(pid_t pid, const char* name, unsigned long int duration)
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
    message2 = stringJoin(message, " seconds.");
    free(message);
    report.message = message2;
    report.type = INFO;
    saveLogReport(report);
    free(report.message);
}

void logSighupCatch(char* configFileName)
{
    LogReport sighupReport;
    sighupReport.type = INFO;
    char* message = stringJoin("Caught SIGHUP. Configuration file '", configFileName); 
    char* message2 = stringJoin(message, "' re-read.");
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

void logParentInit()
{
    logFlash();

    LogReport parentInfo;
    parentInfo.message = stringNumberJoin("Parent process is PID ", getpid());
    parentInfo.type = INFO;
    saveLogReport(parentInfo);
    free(parentInfo.message);
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

void logUnexpectedClientMessageCode(int sock, char messageCode)
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