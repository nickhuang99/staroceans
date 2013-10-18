#include "log.h"

FILE* FILELog::stderr_file = NULL;

FILELog::FILELog()
{
    if (stderr_file == NULL)
    {
        stderr_file = freopen("stderr.log", "w ",stderr);
    }
}

FILELog::~FILELog()
{
    fflush(stderr_file);
}
