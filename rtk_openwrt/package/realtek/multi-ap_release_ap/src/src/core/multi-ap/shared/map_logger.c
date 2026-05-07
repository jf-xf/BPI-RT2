/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

#include "map_logger.h"

#define LOG_USE_COLOR

static pthread_mutex_t print_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct {
	FILE *fp;
	int   level;
	int   quiet;
	char  path[128];
	int   max_log_length;
} LOG_CONFIG;

#define NRM "\x1B[0m"
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"

#ifdef LOG_USE_COLOR
static const char *level_colors[] = { RED, YEL, GRN, CYN, MAG, BLU, WHT };
#endif
static const char *level_names[] = { "ERROR", "WARN", "INFO", "DEBUG", "TRACE", "DETAIL", "FULL" };

FILE *resetLogFile()
{
	/* Acquire lock */
	pthread_mutex_lock(&print_log_mutex);

	char *tmp_fileName = "/var/log/easymesh_templog.txt";
	FILE *tmp_fp;
	int   ch;
	int   line = 0, estimated_total_line = 0;

	if ((tmp_fp = fopen(tmp_fileName, "we")) == NULL) {
		printf("%s: open log file error!\n", __FUNCTION__);
		/* Release lock */
		pthread_mutex_unlock(&print_log_mutex);
		return LOG_CONFIG.fp;
	}

	estimated_total_line = LOG_CONFIG.max_log_length / 50;
	fseek(LOG_CONFIG.fp, 0, SEEK_SET);
	ch = getc(LOG_CONFIG.fp);
	while (ch != EOF) {
		if (ch == '\n')
			line++;
		if (line > estimated_total_line / 2)
			putc(ch, tmp_fp);
		ch = getc(LOG_CONFIG.fp);
	}

	fclose(LOG_CONFIG.fp);
	fclose(tmp_fp);

	if (remove(LOG_CONFIG.path) != 0) {
		printf("%s: remove old log file error!\n", __FUNCTION__);
	} else {
		if (rename(tmp_fileName, LOG_CONFIG.path) != 0) {
			printf("%s: rename temp log file error!\n", __FUNCTION__);
		}
	}

	if ((LOG_CONFIG.fp = fopen(LOG_CONFIG.path, "a+e")) == NULL) {
		printf("%s: open new log file error!\n", __FUNCTION__);
	}

	/* Release lock */
	pthread_mutex_unlock(&print_log_mutex);

	return LOG_CONFIG.fp;
}

int checkResetFile()
{
	long fileSize = 0;

	if ((fileSize = ftell(LOG_CONFIG.fp)) == -1) {
		printf("%s: Error retrieving log file size: %s", __FUNCTION__, strerror(errno));
		return 0;
	}

	if (fileSize >= LOG_CONFIG.max_log_length) {
		LOG_CONFIG.fp = resetLogFile();
	}

	return 1;
}

int initLogFile(const char *fileName, int logfilesize)
{
	if (fileName == NULL) {
		printf("%s: File name error for log file\n", __FUNCTION__);
		return 1;
	}

	if (strlen(fileName) >= 128)
		return 1;

	strcpy(LOG_CONFIG.path, fileName);

	LOG_CONFIG.max_log_length = 1024 * logfilesize;
	printf("max_log_size is: %d kB\n", LOG_CONFIG.max_log_length / 1024);

	if ((LOG_CONFIG.fp = fopen(fileName, "a+e")) == NULL) {
		printf("%s: Can't open log file: %s\n", __FUNCTION__, strerror(errno));
		return 1;
	}

	return 0;
}

void log_log(int level, const char *file, int line, const char *fmt, ...)
{

	if (LOG_CONFIG.level <= level) {
		return;
	}

	/* Get current time */
	time_t t  = time(NULL);
	struct tm *lt = localtime(&t);
	/* Log to stderr */
	if (!LOG_CONFIG.quiet) {
		va_list args;
		char	buf[16];
		buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
		if (level >= LOG_TRACE) {
			fprintf(
				stderr, "%s %s%-6s %s:%d:\x1b[0m ",
				buf, level_colors[level], level_names[level], file, line);
		} else {
			fprintf(
				stderr, "%s %s%-6s:\x1b[0m ",
				buf, level_colors[level], level_names[level]);
		}
#else
		if (level >= LOG_TRACE) {
			fprintf(stderr, "%s %-6s %s:%d: ", buf, level_names[level], file, line);
		} else {
			fprintf(stderr, "%s %-6s: ", buf, level_names[level]);
		}
#endif
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		// fprintf(stderr, "\n");
		fflush(stderr);
	}

	/* Log to file */
	if (LOG_CONFIG.fp != NULL) {
		va_list args;
		char	buf[32];
		buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
		if (level >= LOG_TRACE) {
			fprintf(LOG_CONFIG.fp, "%s %-6s %s:%d: ", buf, level_names[level], file, line);
		} else {
			fprintf(LOG_CONFIG.fp, "%s %-6s : ", buf, level_names[level]);
		}
		va_start(args, fmt);
		checkResetFile();
		vfprintf(LOG_CONFIG.fp, fmt, args);
		va_end(args);
		fprintf(LOG_CONFIG.fp, "\n");
		fflush(LOG_CONFIG.fp);
	}
}

void enableLogDebug()
{
	LOG_CONFIG.quiet = 0;
}

void disableLogDebug()
{
	LOG_CONFIG.quiet = 1;
}

void log_set_fp(FILE *fp)
{
	LOG_CONFIG.fp = fp;
}

void log_set_level(int level)
{
	LOG_CONFIG.level = level;
}
