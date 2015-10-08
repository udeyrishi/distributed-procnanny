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