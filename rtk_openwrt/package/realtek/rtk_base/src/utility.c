#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "common.h"

typedef void (*sighandler_t)(int);
#define VA_CMD_ARG_SIZE 64
int do_cmd(const char *cmd, char *argv[], int dowait)
{
	pid_t pid, wpid;
	int ret, status=0, fd;
	sigset_t tmpset, origset;
	sighandler_t old_handler;

	sigfillset(&tmpset);
	sigprocmask(SIG_BLOCK, &tmpset, &origset);
	pid = vfork();
	sigprocmask(SIG_SETMASK, &origset, NULL);
	
	old_handler = signal(SIGCHLD, SIG_DFL);
	
	if (pid == 0) {
		/* the child */
		signal(SIGINT, SIG_IGN);
		
		//if don't wait process status, use vfork twice to avoid zombie issue
		if(!dowait)
		{
			pid = vfork();
			if(pid < 0)
			{
				fprintf(stderr, "fork twice of %s failed\n", cmd);
				_exit(EXIT_FAILURE);
			}
			else if(pid > 0)
			{
				_exit(EXIT_SUCCESS);
			}
			setsid();
		}
		
		{
			long i, max_fd = sysconf(_SC_OPEN_MAX);
			for (i = 0; i < max_fd; i++){
				if (i != STDOUT_FILENO && i != STDERR_FILENO && i != STDIN_FILENO)
					close(i);
			}
		}

		argv[0] = (char *)cmd;
#if 0
		char *env[3];
		env[0] = "PATH=sbin:/usr/sbin:/bin:/usr/bin:/etc/scripts";
		env[1] = NULL;
		execvpe(cmd, argv, env);
#else //for some case need inherit environment variables from parent process
		execvp(cmd, argv);
#endif
		fprintf(stderr, "exec %s failed\n", cmd);
		_exit(EXIT_FAILURE);
	} else if (pid > 0) {
		/* need wait child pid (for vfork twice)
		if (!dowait)
			ret = 0;
		else 
		*/
		{
			/* parent, wait till rc process dies before spawning */
			while ((wpid = waitpid(pid,&status,0)) != pid) {
				if (wpid == -1 && errno == ECHILD) {	/* see wait(2) manpage */
					break;
				}
			}

			ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
		}
	} else if (pid < 0) {
		fprintf(stderr, "fork of %s failed\n", cmd);
		ret = -1;
	}
	
	signal(SIGCHLD, old_handler);

	return ret;
}

//return 0:OK, other:fail
int va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[VA_CMD_ARG_SIZE];
	int status;

	if(cmd == NULL){
		fprintf(stderr, "<%s:%d> error command\n", __FUNCTION__, __LINE__);
		return -1;
	}
	if((num+2) > VA_CMD_ARG_SIZE){
		fprintf(stderr, "<%s:%d> argumet size overflow, num=%d\n", __FUNCTION__, __LINE__, num);
		return -1;
	}
	
	//TRACE(STA_SCRIPT, "%s ", cmd);
	va_start(ap, dowait);

	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
		//TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	}

	//TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);

	return status;
}

char* str2BinStr(char *str, int len)
{
	int j;
	char *buf, *pstr, *pbuf;
	if(!str || len<=0) return str;
	
	buf = calloc(len+1, 1);
	if(buf) {
		pbuf = buf;
		pstr = str;
		while(pstr && *pstr && ((pstr-str)<=len)) {
			j = strcspn(pstr, " ,:");
			if(j) {
				memcpy(pbuf, pstr, j);
				pbuf += j;
			}
			pstr += j+1;
		}
		memcpy(str, buf, len+1);
		free(buf);
	}
	return str;
}

#define CHK_IS_HEX(a) ((a>='0'&&a<='9')||(a>='a'&&a<='f')||(a>='A'&&a<='F'))

char* binStr2Str(const char *str, int len, char *val, int vsize, char join)
{
	int i, j, k;
	if(!str || len<=0 || !val || vsize<len) return val;
	
	for(i=0, j=0, k=0;i<len && *(str+i) && j<vsize;i++)
	{
		if(CHK_IS_HEX(str[i])) {
			val[j++] = str[i];
			if((k=((++k)%2)) == 0) {
				val[j++] = join;
			}
		} 
	}
	if(k == 0 && j > 0) j--;
	val[j] = 0;
	return val;
}

char *trim_white_space(char *str)
{
	char *end;

	// Trim leading space
	while (isspace(*str)) str++;

	if(*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end >= str && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}

int file_copy(const char *src, const char *dst)
{
	char buf[1024];
	FILE *fsrc = NULL, *fdst = NULL;
	int len, ret=-1;
	fsrc = fopen(src, "rb"); 
	if(fsrc == NULL) goto error;
	fdst = fopen(dst, "wb"); 
	if(fdst == NULL) goto error;
	
	ret = 0;
	while(!feof(fsrc))
	{
		len = fread(buf, 1, sizeof(buf), fsrc);
		if(len) ret += fwrite(buf, 1, len, fdst);
	}

error:
	if(fsrc) fclose(fsrc);
	if(fdst) fclose(fdst);
	return ret;
}