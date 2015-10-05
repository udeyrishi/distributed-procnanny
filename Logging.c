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
        case NORMAL:
            printf("Normal: ");
            break;
        case FATAL:
            printf("Fatal: ");
            break;
        default:
            printf("Default: ");
            break;
    } 
    printf("%s\n", report.message);
}