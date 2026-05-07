#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

int do_help(int argc, char **argv)
{
	printf("Usage: wlan_cal [interface]\n");

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int pargc = argc;
	char **pargv = argv;

	if(pargc < 2) {
		do_help(pargc, pargv);
        return(1);
	}
	
	pargc--;
	pargv = &pargv[1];
	
	if(pargc > 0) {
		mib_init();
		ret = do_wlan_cal(pargv[0]);
		mib_deinit();
	}

    return ret;
}

