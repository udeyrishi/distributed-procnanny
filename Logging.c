#include <stdio.h>
#include "Logging.h"
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