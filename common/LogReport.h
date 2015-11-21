#ifndef __LOG_REPORT__
#define __LOG_REPORT__

typedef enum { FATAL, INFO, ACTION, WARNING, ERROR, DEBUG, SERVER_INFO } LogType;

typedef struct
{
    LogType type;
    char* message;
} LogReport;

typedef void (*LoggerPointer)(LogReport);

#endif