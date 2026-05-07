/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_LOGGER_H_
#define _MAP_LOGGER_H_

#include <stdio.h>
#include <stdarg.h>

enum { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_TRACE, LOG_DETAIL, LOG_FULL };

#define log_error(...)  log_log(LOG_ERROR,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)   log_log(LOG_WARN,   __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)   log_log(LOG_INFO,   __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...)  log_log(LOG_DEBUG,  __FILE__, __LINE__, __VA_ARGS__)
#define log_trace(...)  log_log(LOG_TRACE,  __FILE__, __LINE__, __VA_ARGS__)
#define log_detail(...) log_log(LOG_DETAIL, __FILE__, __LINE__, __VA_ARGS__)
#define log_full(...)   log_log(LOG_FULL,   __FILE__, __LINE__, __VA_ARGS__)

void log_set_fp(FILE *fp);
void log_set_level(int level);

void log_log(int level, const char *file, int line, const char *fmt, ...);

int initLogFile(const char *fileName, int logfilesize);

void enableLogDebug();

void disableLogDebug();

#endif
