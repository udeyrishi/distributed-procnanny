#include "LogReport.h"
#include "Utils.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "memwatch.h"

// private
char* getTime()
{
    char* output;
    time_t rawtime;
    time(&rawtime);
    char* t = ctime(&rawtime);
    t[strlen(t) - 1] = '\0';
    output = stringJoin("[", t);
    char* output2 = stringJoin(output, "] ");
    free(output);
    return output2;
}

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

void printLogReport(LogReport report)
{
    char* output = getFormattedReport(report);
    printf("%s\n", output);
    free(output);
}