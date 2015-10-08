#include <stdio.h>
#include "Logging.h"
#include "Utils.h"
#include "memwatch.h"

void saveLogReport(LogReport report)
{
    switch(report.type)
    {
        case ERROR:
            printf("Error: ");
            break;
        case WARNING:
            printf("Warning: ");
            break;
        case ACTION:
            printf("Action: ");
            break;
        case FATAL:
            printf("Fatal: ");
            break;
        case INFO:
            printf("Info: ");
            break;
        case DEBUG:
            printf("Debug: ");
            break;
        default:
            break;
    } 
    printf("%s\n", report.message);
}

void printLogReport(LogReport report)
{
    // TODO: fix these
    saveLogReport(report);
}

// TODO: Maybe for next part, write an easier method for general purpose string sizes...
void logFinalReport(int killCount)
{
    LogReport report;
    report.type = INFO;
    char* message = stringNumberJoin("Exiting. ", killCount);
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