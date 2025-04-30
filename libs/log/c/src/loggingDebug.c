//------------------------------ tabstop = 4 ----------------------------------
//
// If not stated otherwise in this file or this component's LICENSE file the
// following copyright and licenses apply:
//
// Copyright 2024 Comcast Cable Communications Management, LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//
//------------------------------ tabstop = 4 ----------------------------------

/*-----------------------------------------------
 * loggerDebug.c
 *
 * Development implementation of the logging.h facade
 * to print all logging messages to STDOUT.  Like the
 * other implementations, built and included based on
 * #define values
 *
 * Author: jelderton - 6/19/15
 *-----------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <icLog/logging.h>
#include <pthread.h>
#include <sys/time.h>

#include "loggingCommon.h"
#include <glib.h>

static FILE *outputFile = NULL;

static char *getTimeString(void);

int __attribute__((weak)) getDebugLoggerFileDescriptor(void)
{
    return STDOUT_FILENO;
}

/*
 * initialize the logger
 */
__attribute__((constructor(101))) static void initIcLogger(void)
{
    int outputFd = getDebugLoggerFileDescriptor();
    if (outputFd == STDOUT_FILENO)
    {
        outputFile = stdout;
    }
    else if (outputFd == STDERR_FILENO)
    {
        outputFile = stderr;
    }
    else
    {
        outputFile = fdopen(outputFd, "w");
    }
}


/*
 * Issue logging message based on a 'categoryName' and 'priority'
 */
void icLogMsg(const char *file,
              size_t filelen,
              const char *func,
              size_t funclen,
              long line,
              const char *categoryName,
              logPriority priority,
              const char *format,
              ...)
{
    va_list arglist;

    // skip if priority is > logLevel
    //
    if (!shouldLogMessage(priority))
    {
        return;
    }

    // print the cat name, priority, then the message
    // NOTE: trying to keep format the same as zlog.conf
    //
    g_autofree char *timeStr = getTimeString();
    g_autofree char *catPidStr = g_strdup_printf("[%s %d] - ", categoryName, getpid());

    const char *priorityStr = NULL;
    // map priority to Android syntax
    //
    switch (priority)
    {
        case IC_LOG_TRACE:
            priorityStr = "TRACE: ";
            break;

        case IC_LOG_DEBUG:
            priorityStr = "DEBUG: ";
            break;

        case IC_LOG_INFO:
            priorityStr = "INFO: ";
            break;

        case IC_LOG_WARN:
            priorityStr = "WARN: ";
            break;

        case IC_LOG_ERROR:
            priorityStr = "ERROR: ";
            break;

        default:
            priorityStr = "???: ";
            break;
    }

    // preprocess the variable args format, then forward to STDOUT
    //
    va_start(arglist, format);
    g_autofree char *logOut = g_strdup_vprintf(format, arglist);
    va_end(arglist);

    if (outputFile == NULL)
    {
        outputFile = stdout;
    }
    fprintf(outputFile, "%s%s%s%s\n", timeStr, catPidStr, priorityStr, logOut);

    if (fflush(outputFile) == EOF)
    {
        // Output is gone, prevent an infinite cycle when logging any SIGPIPEs
        setIcLogPriorityFilter(IC_LOG_NONE);
    }
}

static char *getTimeString(void)
{
    char buff[32];
    struct timeval now;
    struct tm ptr;

    // get local time & millis
    //
    gettimeofday(&now, NULL);
    localtime_r(&now.tv_sec, &ptr);

    // pretty-print the time
    //
    strftime(buff, 31, "%Y-%m-%d %H:%M:%S", &ptr);

    // add millis to the end of the formatted string
    //
    return g_strdup_printf("%s.%03ld : ", buff, (long) (now.tv_usec / 1000));
}
