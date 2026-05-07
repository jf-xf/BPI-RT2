/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *  Copyright (c) 2018, Realtek Semiconductor Corp.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */


#include "platform.h"

#include <stdlib.h>   // free(), malloc(), ...
#include <string.h>   // memcpy(), memcmp(), ...
#include <stdio.h>    // printf(), ...
#include <stdarg.h>   // va_list
#include <sys/time.h> // gettimeofday()
#include <unistd.h>

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
#	include <pthread.h> // mutexes, pthread_self()
#endif

////////////////////////////////////////////////////////////////////////////////
// Private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

// *********** libc stuff ******************************************************

// We will use this variable to save the instant when "PLATFORM_INIT()" was
// called. This way we sill be able to get relative timestamps later when
// someone calls "PLATFORM_GET_TIMESTAMP()"
//
static struct timeval tv_begin;

enum {
	VERBOSE_ERROR,
	VERBOSE_WARNING,
	VERBOSE_INFO,
	VERBOSE_DEBUG,
	VERBOSE_TRACE,
	VERBOSE_DETAIL,
	VERBOSE_FULL
};

// The following variable is used to set which "PLATFORM_PRINTF_DEBUG_*()"
// functions should be ignored:
//
//   0 => Only print ERROR messages
//   1 => Print ERROR and WARNING messages
//   2 => Print ERROR, WARNING and INFO messages
//   3 => Print ERROR, WARNING, INFO and DETAIL messages
//
static int verbosity_level = VERBOSE_INFO;

// Mutex to avoid STDOUT "overlaping" due to different threads writing at the
// same time.
//
#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_fp_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

const char *VALID_ETH_INTERFACES_IN_BRIDGE[] = { "eth", "nas", "veip0", NULL };

// Color codes to print messages from different threads in different colors
//
#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

////////////////////////////////////////////////////////////////////////////////
// Platform Logging Utilities
////////////////////////////////////////////////////////////////////////////////

static char       *PLATFORM_LEVEL_COLORS[] = { KRED, KYEL, KGRN, KCYN, KMAG, KBLU, KWHT };
static const char *PLATFORM_LEVEL_NAMES[]  = { "ERROR", "WARN", "INFO", "DEBUG", "TRACE", "DETAIL", "FULL" };

static struct {
	FILE *fp;
	int   quiet;
	char  path[128];
	int   max_log_length;
} PLATFORM_LOG_CONFIG;

void PLATFORM_ENABLE_LOG(void)
{
	PLATFORM_LOG_CONFIG.quiet = 0;
}

void PLATFORM_DISABLE_LOG(void)
{
	PLATFORM_LOG_CONFIG.quiet = 1;
}

void PLATFORM_LOG_FILE_INIT(const char *log_file_path, int log_file_size, int quiet)
{
	PLATFORM_LOG_CONFIG.quiet = quiet;

	if (log_file_path == NULL) {
		printf("%s: Log file name error \n", __FUNCTION__);
		return;
	}

	if (strlen(log_file_path) >= 128) {
		printf("%s: Log file path exceed max length \n", __FUNCTION__);
		return;
	}

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_lock(&log_fp_mutex);
#endif
	strcpy(PLATFORM_LOG_CONFIG.path, log_file_path);
	PLATFORM_LOG_CONFIG.max_log_length = 1024 * log_file_size;

	printf("max_log_size is: %d kB\n", log_file_size);

	if ((PLATFORM_LOG_CONFIG.fp = fopen(log_file_path, "a+e")) == NULL) {
		printf("%s: Can't open log file: %s\n", __FUNCTION__, strerror(errno));
#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
		pthread_mutex_unlock(&log_fp_mutex);
#endif
		return;
	}

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_unlock(&log_fp_mutex);
#endif
	return;
}

FILE *PLATFORM_LOG_FILE_RESET()
{
#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_lock(&log_fp_mutex);
#endif
	char *tmp_fileName = "/var/log/multiap_core_templog.txt";
	FILE *tmp_fp;
	int   ch;
	int   line = 0, estimated_total_line = 0;

	if ((tmp_fp = fopen(tmp_fileName, "we")) == NULL) {
		printf("%s: open log file error!\n", __FUNCTION__);
		tmp_fp = PLATFORM_LOG_CONFIG.fp;
#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
		pthread_mutex_unlock(&log_fp_mutex);
#endif
		return tmp_fp;
	}

	estimated_total_line = PLATFORM_LOG_CONFIG.max_log_length / 50;
	fseek(PLATFORM_LOG_CONFIG.fp, 0, SEEK_SET);
	ch = getc(PLATFORM_LOG_CONFIG.fp);
	while (ch != EOF) {
		if (ch == '\n')
			line++;
		if (line > estimated_total_line / 2)
			putc(ch, tmp_fp);
		ch = getc(PLATFORM_LOG_CONFIG.fp);
	}

	fclose(PLATFORM_LOG_CONFIG.fp);
	fclose(tmp_fp);

	if (remove(PLATFORM_LOG_CONFIG.path) != 0) {
		printf("%s: remove old log file error!\n", __FUNCTION__);
	} else {
		if (rename(tmp_fileName, PLATFORM_LOG_CONFIG.path) != 0) {
			printf("%s: rename temp log file error!\n", __FUNCTION__);
		}
	}

	if ((PLATFORM_LOG_CONFIG.fp = fopen(PLATFORM_LOG_CONFIG.path, "a+e")) == NULL) {
		printf("%s: open new log file error!\n", __FUNCTION__);
	}
	tmp_fp = PLATFORM_LOG_CONFIG.fp;

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_unlock(&log_fp_mutex);
#endif
	return tmp_fp;
}

void PLATFORM_LOG_FILE_CHECK()
{
	long curr_log_file_size = 0;

	if ((curr_log_file_size = ftell(PLATFORM_LOG_CONFIG.fp)) == -1) {
		printf("%s: Error retrieving log file size: %s", __FUNCTION__, strerror(errno));
		return;
	}

	if (curr_log_file_size >= PLATFORM_LOG_CONFIG.max_log_length) {
		PLATFORM_LOG_CONFIG.fp = PLATFORM_LOG_FILE_RESET();
	}

	return;
}

void PLATFORM_PRINTF_LOG(int log_level, const char *format, va_list args)
{
	if (verbosity_level < log_level) {
		return;
	}

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_lock(&printf_mutex);
#endif
	INT32U ts = PLATFORM_GET_TIMESTAMP();
	char   tss[100], content[1024]; // timestamp & log message

	sprintf(tss, "[%03d.%03d] ", ts / 1000, ts % 1000);
	vsprintf(content, format, args);

	/* print log message to console */
	if (!PLATFORM_LOG_CONFIG.quiet) {
		printf("%s", PLATFORM_LEVEL_COLORS[log_level]); //set coloration according to log level
		printf("%s", tss);
		if (log_level == VERBOSE_ERROR || log_level == VERBOSE_WARNING) {
			printf("%s   : ", PLATFORM_LEVEL_NAMES[log_level]);
		}
		printf("%s", content);
		printf("%s", KNRM); // reset coloration
	}

	/* save log message to log file */
	if (PLATFORM_LOG_CONFIG.fp != NULL) {
		PLATFORM_LOG_FILE_CHECK();
		fprintf(PLATFORM_LOG_CONFIG.fp, "%s", tss);
		if (log_level == VERBOSE_ERROR || log_level == VERBOSE_WARNING) {
			fprintf(PLATFORM_LOG_CONFIG.fp, "%s   : ", PLATFORM_LEVEL_NAMES[log_level]);
		}
		fprintf(PLATFORM_LOG_CONFIG.fp, "%s", content);
		fflush(PLATFORM_LOG_CONFIG.fp);
	}

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_unlock(&printf_mutex);
#endif
	return;
}

void PLATFORM_PRINTF_ERROR(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	PLATFORM_PRINTF_LOG(VERBOSE_ERROR, format, arglist);
	va_end(arglist);
}

void PLATFORM_PRINTF_WARNING(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	PLATFORM_PRINTF_LOG(VERBOSE_WARNING, format, arglist);
	va_end(arglist);
}

void PLATFORM_PRINTF_INFO(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	PLATFORM_PRINTF_LOG(VERBOSE_INFO, format, arglist);
	va_end(arglist);
}

void PLATFORM_PRINTF_DEBUG(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	PLATFORM_PRINTF_LOG(VERBOSE_DEBUG, format, arglist);
	va_end(arglist);
}

void PLATFORM_PRINTF_TRACE(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	PLATFORM_PRINTF_LOG(VERBOSE_TRACE, format, arglist);
	va_end(arglist);
}

void PLATFORM_PRINTF_DETAIL(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	PLATFORM_PRINTF_LOG(VERBOSE_DETAIL, format, arglist);
	va_end(arglist);
}

void PLATFORM_PRINTF_FULL(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	PLATFORM_PRINTF_LOG(VERBOSE_FULL, format, arglist);
	va_end(arglist);
}

////////////////////////////////////////////////////////////////////////////////
// Platform Logging Utilities
////////////////////////////////////////////////////////////////////////////////

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
#	define ENABLE_COLOR (1)
#else
#	define ENABLE_COLOR (0)
#endif

char *_enableColor(void)
{
#if ENABLE_COLOR
	static pthread_t first_thread = 0;

	if (0 == first_thread) {
		first_thread = pthread_self();
	}

	if (pthread_self() == first_thread) {
		return KWHT;
	} else {
		static char *aux[] = { KRED, KGRN, KYEL, KBLU, KMAG, KCYN, KWHT };
		return aux[(pthread_self() - first_thread) % 6];
	}
#else
	return "";
#endif
}

char *_disableColor(void)
{
#if ENABLE_COLOR
	return KNRM;
#else
	return "";
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Platform API: libc stuff
////////////////////////////////////////////////////////////////////////////////

void *PLATFORM_MALLOC(INT32U size)
{
	void *p;

	if (0 == size)
		return NULL;

	p = malloc(size);

	if (NULL == p) {
		printf("ERROR: Out of memory!\n");
		exit(1);
	}

	return p;
}

void *PLATFORM_ZALLOC(INT32U size)
{
	void *p;

	if (0 == size)
		return NULL;

	p = calloc(1, size);

	if (NULL == p) {
		printf("ERROR: Out of memory!\n");
		exit(1);
	}

	return p;
}

void PLATFORM_FREE(void *ptr)
{
	if (NULL != ptr) {
		free(ptr);
		// ptr = NULL;
	}
	return;
}

void PLATFORM_SAFE_FREE(void **ptr)
{
	if (NULL != *ptr) {
		free(*ptr);
		*ptr = NULL;
	}
	return;
}

void *PLATFORM_REALLOC(void *ptr, INT32U size)
{
	void *p;

	if (0 == size) {
		PLATFORM_PRINTF_WARNING("realloc to zero!\n");
		PLATFORM_FREE(ptr);
		return NULL;
	}

	p = realloc(ptr, size);

	if (NULL == p) {
		printf("ERROR: Out of memory!\n");
		exit(1);
	}
	return p;
}

void *PLATFORM_MEMSET(void *dest, INT8U c, INT32U n)
{
	return memset(dest, c, n);
}

void *PLATFORM_MEMCPY(void *dest, const void *src, INT32U n)
{
	return memcpy(dest, src, n);
}

INT8U PLATFORM_MEMCMP(const void *s1, const void *s2, INT32U n)
{
	int aux;

	aux = memcmp(s1, s2, n);

	if (0 == aux) {
		return 0;
	} else {
		return 1;
	}
}

char *PLATFORM_STRCAT(char *dest, const char *src)
{
	return strcat(dest, src);
}

int PLATFORM_STRCMP(const char *s1, const char *s2)
{
	if (s1 != NULL && s2 != NULL)
		return strcmp(s1, s2);
	else {
		PLATFORM_PRINTF_ERROR("NULL parameter in %s!!\n", __FUNCTION__);
		return 1;
	}
}

char *PLATFORM_STRCPY(char *dest, const char *src)
{
	return strcpy(dest, src);
}

char *PLATFORM_STRSTR(const char *str, const char *substr)
{
	return strstr(str, substr);
}
INT32U PLATFORM_STRLEN(const char *s)
{
	return strlen(s);
}

char *PLATFORM_STRDUP(const char *s)
{
	return strdup(s);
}

char *PLATFORM_STRNCAT(char *dest, const char *src, INT32U n)
{
	return strncat(dest, src, n);
}

void PLATFORM_SNPRINTF(char *dest, INT32U n, const char *format, ...)
{
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(dest, n, format, arglist);
	va_end(arglist);

	return;
}

void PLATFORM_VSNPRINTF(char *dest, INT32U n, const char *format, va_list ap)
{
	vsnprintf(dest, n, format, ap);

	return;
}

void PLATFORM_PRINTF(const char *format, ...)
{
	va_list arglist;

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_lock(&printf_mutex);
#endif

	va_start(arglist, format);
	vprintf(format, arglist);
	va_end(arglist);

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_unlock(&printf_mutex);
#endif

	return;
}

void PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(int level)
{
	verbosity_level = level;
}

INT8U PLATFORM_NEED_DETAIL_PRINT()
{
	return verbosity_level >= VERBOSE_DETAIL;
}

void PLATFORM_HEXDUMP(const char *title, const INT8U *buf, INT16U len)
{
	INT32U  ts;
	int      i;

	if (verbosity_level < VERBOSE_DEBUG) {
		return;
	}

	ts = PLATFORM_GET_TIMESTAMP();

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_lock(&printf_mutex);
#endif

	printf(KCYN);

	printf("[%03d.%03d] ", ts / 1000, ts % 1000);

	printf("%s - (len=%lu):", title, (unsigned long) len);

	if (buf == NULL) {
		printf(" [NULL]");
	} else {
		for (i = 0; i < len; i++)
			printf(" %02x", buf[i]);
	}

	printf("\n");

	printf(KNRM);

#ifndef _FLAVOUR_X86_WINDOWS_MINGW_
	pthread_mutex_unlock(&printf_mutex);
#endif
	return;
}

INT32U PLATFORM_GET_TIMESTAMP(void)
{
	struct timeval tv_end;
	INT32U         diff;

	gettimeofday(&tv_end, NULL);

	diff = (tv_end.tv_usec - tv_begin.tv_usec) / 1000 + (tv_end.tv_sec - tv_begin.tv_sec) * 1000;

	return diff;
}

char *PLATFORM_GET_VALID_INTERFACE_NAME(const char *interface_name, const char **valid_interfaces)
{
	char *valid_interface_name = NULL;
	int   i                    = 0;
	if (interface_name == NULL)
		return NULL;
	while (valid_interfaces[i] != NULL) {
		if (NULL != (valid_interface_name = strstr(interface_name, valid_interfaces[i])))
			return valid_interface_name;
		i++;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Platform API: Initialization functions
////////////////////////////////////////////////////////////////////////////////

INT8U PLATFORM_INIT(void)
{

	// Call "_timeval_print()" for the first time so that the initialization
	// time is saved for future reference.
	//
	gettimeofday(&tv_begin, NULL);

	return 1;
}
