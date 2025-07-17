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

#include <sys/syscall.h>
#include <unistd.h>
#include "comolog.h"
#include <cstdarg>
#if defined(__android__)
#include <android/log.h>
#endif
#include <cstdio>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <elog.h>

namespace como {

static LoggerWriteLog g_loggerWriteLog = nullptr;

static Logger logger;

int   Logger::sLevel = DEBUG;
char  Logger::szSamplingTag[32] = {'S', '\0'};
char  Logger::keyword_buf[MAX_KEYWORD_STR] = {'\0'};
char* Logger::keywords[MAX_KEYWORDS] = {NULL};
int   Logger::keyword_count = 0;

static void GetLocalTimeWithMs(char *currentTime, size_t maxChars);

int Logger::ELogInfoFilterComo(const char *line)
{
    if (keyword_count <= 0) {
        return 1;
    }

    for (int i = 0;  i < keyword_count;  i++) {
        if (strstr(line, keywords[i]) != nullptr) {
            return i + 1;
        }
    }

    return 0;
}

/**
 * Perform in-place splitting on the original keyword_str, replace ',' with
 * '\0', and record each pointer.
 */
static int parse_keywords(char* keyword_str, char* keywords[], int max_keywords)
{
    int count = 0;
    char* token = keyword_str;

    while (('\0' != *token) && (count < max_keywords)) {
        keywords[count++] = token;
        while (('\0' != *token) && (',' != *token)) {
            token++;
        }

        if (',' == *token) {
            *token = '\0';
            token++;
        }
    }
    return count;
}

Logger::Logger() {
    /* close printf buffer */
    setbuf(stdout, NULL);
    /* initialize EasyLogger */
    elog_init();
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
#ifdef ELOG_COLOR_ENABLE
    elog_set_text_color_enabled(true);
#endif
    /* start EasyLogger */
    elog_start();
    pLogInfoFilter = ELogInfoFilterComo;

    /* dynamic set enable or disable for output logs (true or false) */
//    elog_set_output_enabled(false);
    /* dynamic set output logs's level (from ELOG_LVL_ASSERT to ELOG_LVL_VERBOSE) */
//    elog_set_filter_lvl(ELOG_LVL_WARN);
    /* dynamic set output logs's filter for tag */
//    elog_set_filter_tag("main");
    /* dynamic set output logs's filter for keyword */
//    elog_set_filter_kw("Hello");
    /* dynamic set output logs's tag filter */
//    elog_set_filter_tag_lvl("main", ELOG_LVL_WARN);
}

Logger::~Logger() {
    elog_stop();
}

void Logger::D(
    /* [in] */ const char* tag,
    /* [in] */ const char* format, ...)
{
    if (DEBUG > sLevel) {
        return;
    }

    va_list argList;
    char currentTime[64];
    GetLocalTimeWithMs(currentTime, sizeof(currentTime));

    char buf[256];
    snprintf(buf, sizeof(buf)-1, "[%s %ld %s DEBUG %s]: %s", szSamplingTag,
                                 syscall(SYS_gettid), currentTime, tag, format);
    va_start(argList, format);
    elog_output_args_simple(ELOG_LVL_DEBUG, buf, g_loggerWriteLog, argList);
    va_end(argList);
}

void Logger::E(
    /* [in] */ const char* tag,
    /* [in] */ const char* format, ...)
{
    if (ERROR > sLevel) {
        return;
    }

    va_list argList;
    char currentTime[64];
    GetLocalTimeWithMs(currentTime, sizeof(currentTime));

    char buf[256];
    snprintf(buf, sizeof(buf)-1, "[%s %ld %s ERROR %s]: %s", szSamplingTag,
                                 syscall(SYS_gettid), currentTime, tag, format);
    va_start(argList, format);
    elog_output_args_simple(ELOG_LVL_ERROR, buf, g_loggerWriteLog, argList);
    va_end(argList);
}

void Logger::V(
    /* [in] */ const char* tag,
    /* [in] */ const char* format, ...)
{
    if (VERBOSE > sLevel) {
        return;
    }

    va_list argList;
    char currentTime[64];
    GetLocalTimeWithMs(currentTime, sizeof(currentTime));

    char buf[256];
    snprintf(buf, sizeof(buf)-1, "[%s %ld %s VERBOSE %s]: %s", szSamplingTag,
                                 syscall(SYS_gettid), currentTime, tag, format);
    va_start(argList, format);
    elog_output_args_simple(ELOG_LVL_VERBOSE, buf, g_loggerWriteLog, argList);
    va_end(argList);
}

void Logger::W(
    /* [in] */ const char* tag,
    /* [in] */ const char* format, ...)
{
    if (WARNING > sLevel) {
        return;
    }

    va_list argList;
    char currentTime[64];
    GetLocalTimeWithMs(currentTime, sizeof(currentTime));

    char buf[256];
    snprintf(buf, sizeof(buf)-1, "[%s %ld %s WARNING %s]: %s", szSamplingTag,
                                 syscall(SYS_gettid), currentTime, tag, format);
    va_start(argList, format);
    elog_output_args_simple(ELOG_LVL_WARN, buf, g_loggerWriteLog, argList);
    va_end(argList);
}

#if defined(__android__)
static int ToAndroidLogPriority(
    /* [in] */ int level)
{
    switch(level) {
        case Logger::VERBOSE:
            return ANDROID_LOG_VERBOSE;
        case Logger::DEBUG:
            return ANDROID_LOG_DEBUG;
        case Logger::WARNING:
            return ANDROID_LOG_WARN;
        case Logger::ERROR:
            return ANDROID_LOG_ERROR;
        default:
            return ANDROID_LOG_DEFAULT;
    }
}
#endif

void Logger::Log(
    /* [in] */ int level,
    /* [in] */ const char* tag,
    /* [in] */ const char* format, ...)
{
    if (level > sLevel) {
        return;
    }

    va_list argList;
    char currentTime[64];
    GetLocalTimeWithMs(currentTime, sizeof(currentTime));

    char buf[256];
    snprintf(buf, sizeof(buf)-1, "[%s %ld %s LOG %s]: %s", szSamplingTag,
                                 syscall(SYS_gettid), currentTime, tag, format);
    va_start(argList, format);
    elog_output_args_simple(level, buf, g_loggerWriteLog, argList);
    va_end(argList);
}

void Logger::Log(
    /* [in] */ int level,
    /* [in] */ const char* tag,
    /* [in] */ const char* format,
    /* [in] */ va_list argList)
{
    if (level > sLevel) {
        return;
    }

    char currentTime[64];
    GetLocalTimeWithMs(currentTime, sizeof(currentTime));

    char buf[256];
    snprintf(buf, sizeof(buf)-1, "[%s %ld %s LOG %s]: %s", szSamplingTag,
                                 syscall(SYS_gettid), currentTime, tag, format);
    elog_output_args_simple(level, buf, g_loggerWriteLog, argList);
}

void Logger::SetLevel(
    /* [in] */ int level)
{
    sLevel = level;
}

int Logger::GetLevel()
{
    return sLevel;
}

void Logger::SetSamplingTag(
    /* [in] */ const char *szSamplingTag_)
{
    if (nullptr == szSamplingTag_) {
        szSamplingTag[0] = '\0';
        return;
    }

    (void)strncpy(szSamplingTag, szSamplingTag_, sizeof(szSamplingTag));
    szSamplingTag[sizeof(szSamplingTag) - 1] = '\0';
}

void Logger::SetKeywords(
    /* [in] */ const char *keyword_str)
{
    if (nullptr == keyword_str) {
        keyword_buf[0] = '\0';
        keyword_count = 0;
        return;
    }

    (void)strncpy(keyword_buf, keyword_str, sizeof(keyword_buf));
    keyword_buf[sizeof(keyword_buf) - 1] = '\0';

    keyword_count = parse_keywords(keyword_buf, keywords, MAX_KEYWORDS);
}

/*
 * get current time in this format: "2021-07-09 11:02:58.361 +0800"
 */
static void GetLocalTimeWithMs(char *currentTime, size_t maxChars)
{
    if (nullptr == currentTime) {
        return;
    }

    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    // 2021-11-30 12:34:56
    char ymdhms[20];
    // The +hhmm or -hhmm numeric timezone
    char timezone[8];
    struct tm nowTime;
    localtime_r(&curTime.tv_sec, &nowTime);
    (void)strftime(ymdhms, sizeof(ymdhms), "%Y-%m-%d %H:%M:%S", &nowTime);
    (void)strftime(timezone, sizeof(timezone), "%z", &nowTime);

    (void)snprintf(currentTime, maxChars, "%s.%03d %s", ymdhms, milli, timezone);
}

void Logger::Monitor(
    /* [out] */ char *buffer,
    /* [in] */  int bufSize,
    /* [in] */  const char* tag)
{
    if ((nullptr == buffer) || (nullptr == tag) || (bufSize < 1)) {
        return;
    }

    va_list argList;
    char currentTime[64];
    GetLocalTimeWithMs(currentTime, sizeof(currentTime));

    snprintf(buffer, bufSize-1, "[%s %ld %s %s]", szSamplingTag,
                                         syscall(SYS_gettid), currentTime, tag);
}

void Logger::SetLoggerWriteLog(LoggerWriteLog loggerWriteLog)
{
    g_loggerWriteLog = loggerWriteLog;
}

} // namespace como
