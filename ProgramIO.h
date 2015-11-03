#ifndef __PROGRAM_IO_H__
#define __PROGRAM_IO_H__

#include "Logging.h"
#include "Utils.h"

char** getOutputFromProgram(const char* programName, int * numberLinesRead, LogReport* report); 
void freeOutputFromProgram(char** output, int numberLinesRead);
char** readFile(const char* filePath, int* numberLinesRead, LogReport* report);
#endif