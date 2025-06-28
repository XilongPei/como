//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#ifndef __COMO_LOGGER_H__
#define __COMO_LOGGER_H__

#include <cstdarg>
#include "comodef.h"

namespace como {

using LoggerWriteLog = int(*)(const char *, size_t);

/*
    Logger will produce the following log line:

    SamplingTag       +- month      +- millisecond
    +                 |  +- day     |   +- timezone     +- message
    |                 |  |          |   |               |
   [Sample 1239 2022-08-08 11:02:58.361 +0800 LOG tag]: log information
           |               |                  |   |
           |               +- time            |   +- tag information
           +- thread id                       +- DEBUG,ERROR,VERBOSE,WARNING
*/
class COM_PUBLIC Logger
{
public:
    Logger();
    ~Logger();

    static void D(
        /* [in] */ const char* tag,
        /* [in] */ const char* format, ...);

    static void E(
        /* [in] */ const char* tag,
        /* [in] */ const char* format, ...);

    static void V(
        /* [in] */ const char* tag,
        /* [in] */ const char* format, ...);

    static void W(
        /* [in] */ const char* tag,
        /* [in] */ const char* format, ...);

    static void Log(
        /* [in] */ int level,
        /* [in] */ const char* tag,
        /* [in] */ const char* format, ...);

    static void Log(
        /* [in] */ int level,
        /* [in] */ const char* tag,
        /* [in] */ const char* format,
        /* [in] */ va_list argList);

    static void SetLevel(
        /* [in] */ int level);

    static int GetLevel();

    static void SetSamplingTag(
        /* [in] */ const char *szSamplingTag_);

    static void SetKeywords(
        /* [in] */ const char *keyword_str);

    static void Monitor(
        /* [out] */ char *buffer,
        /* [in] */  int bufSize,
        /* [in] */  const char* tag);

    static void SetLoggerWriteLog(LoggerWriteLog loggerWriteLog);

public:
    /* refer to external/EasyLogger/easylogger/inc/elog.h
    #define ELOG_LVL_ASSERT                      0
    #define ELOG_LVL_ERROR                       1
    #define ELOG_LVL_WARN                        2
    #define ELOG_LVL_INFO                        3
    #define ELOG_LVL_DEBUG                       4
    #define ELOG_LVL_VERBOSE                     5
    */
    static constexpr int ASSERT  = 0;
    static constexpr int ERROR   = 1;
    static constexpr int WARNING = 2;
    static constexpr int INFO    = 3;
    static constexpr int DEBUG   = 4;
    static constexpr int VERBOSE = 5;

private:
    COM_LOCAL static int sLevel;
    COM_LOCAL static char szSamplingTag[32];

    static constexpr int MAX_KEYWORD_STR = 128;
    static constexpr int MAX_KEYWORDS    = 8;
    COM_LOCAL static char  keyword_buf[MAX_KEYWORD_STR];
    COM_LOCAL static char* keywords[MAX_KEYWORDS];
    COM_LOCAL static int   keyword_count;

    static int ELogInfoFilterComo(const char *line);
};

} // namespace como

/*
statement:
    #define Logger::D   do () while(0);
cause:
    warning: ISO C++11 requires whitespace after the macro name

so, we define Logger_D and call it to avoid this warning when Logger::D is empty
*/
#ifdef DISABLE_LOGGER
#define Logger_D(format, ...)
#define Logger_E(format, ...)
#define Logger_V(format, ...)
#define Logger_W(format, ...)
#define Logger_Log(format, ...)
#else
#define Logger_D Logger::D
#define Logger_E Logger::E
#define Logger_V Logger::V
#define Logger_W Logger::W
#define Logger_Log Logger::Log
#endif

#endif // __COMO_LOGGER_H__

