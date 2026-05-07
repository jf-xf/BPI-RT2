#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "common.h"

enum{
	WEB_CTRL_GPON,
	WEB_CTRL_EPON,
	WEB_CTRL_NONE
};

struct WEB_CTRL_s{
	char *web_file;
	char *mib_name;
	char *mib_value;
	int (*func)(void);
	int do_mib;
} web_ctrl_list[] = {
	[WEB_CTRL_GPON]         = {"/tmp/menu/pon", "PON_MODE", "1", NULL, 1},
	[WEB_CTRL_EPON]         = {"/tmp/menu/pon", "PON_MODE", "2", NULL, 1},
	[WEB_CTRL_NONE]        = {NULL, NULL, NULL, 0}
};

int rtk_web_ctrl_init(void)
{
	char buf[256]={0};
	char cmd[256]={0};
	int i, ret=0, web_show=0;

	va_cmd(BIN_SH, 2, 1, "-c", "mkdir -p /tmp/menu");

	for(i=0; i<(sizeof(web_ctrl_list)/sizeof(struct WEB_CTRL_s)); i++) {
		struct WEB_CTRL_s *w = &web_ctrl_list[i];
		if(w->web_file) {
			if(w->do_mib && w->mib_name && w->mib_value){
				if(mib_get_s(w->mib_name, buf, sizeof(buf)) == 0){
					printf("%s:%d cannot not get mib name %s\n", __FUNCTION__, __LINE__, w->mib_name);
				}
				if(strncmp(buf, w->mib_value, strlen(w->mib_value))==0){
					web_show = 1;
				}
			}
			else if(w->func){
				ret = w->func();
				if(ret == 1){
					web_show = 1;
				}
			}
			if(web_show == 1){
				snprintf(cmd, sizeof(cmd), "touch %s", w->web_file);
				ret = va_cmd(BIN_SH, 2, 1, "-c", cmd);
			}
		}
	}

	return ret;
}

