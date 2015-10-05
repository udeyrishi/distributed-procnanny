#ifndef __PROC_READER__
#define __PROC_READER__

#include "Logging.h"

char** getOutputFromProgram(const char* programName, int maxNumberLines, int maxLineLength, int * numberLinesRead, LogReport* report); 

#endif