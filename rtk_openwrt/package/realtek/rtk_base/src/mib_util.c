#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

int do_mib_get(int argc, char **argv);
int do_mib_set(int argc, char **argv);
int do_mib_all(int argc, char **argv);
int do_mib_commit(int argc, char **argv);
int do_mib_default(int argc, char **argv);
int do_mib_help(int argc, char **argv);
int do_mib_setType(int argc, char **argv);
int do_mib_getType(int argc, char **argv);

enum{
	MIB_CMD_GET,
	MIB_CMD_SET,
	MIB_CMD_ALL,
	MIB_CMD_COMMIT,
	MIB_CMD_DEFAULT,
	MIB_CMD_SETTYPE, 
	MIB_CMD_GETTYPE,
	MIB_CMD_HELP,
	MIB_CMD_NONE
};

struct MIB_CMD_s{
	char *cmd;
	char *help;
	int (*func)(int, char **);
	int do_mib;
} mib_cmd_list[] = {
	[MIB_CMD_GET]         = {"get", "[MIB-NAME]", do_mib_get, 1},
	[MIB_CMD_SET]         = {"set", "[MIB-NAME] [MIB-VALUE]", do_mib_set, 1},
	[MIB_CMD_ALL]         = {"all", "[cs/hs]", do_mib_all, 1},
	[MIB_CMD_COMMIT]      = {"commit", NULL, do_mib_commit, 1},
	[MIB_CMD_DEFAULT]     = {"default", "[cs/hs/uci]", do_mib_default, 0},
	[MIB_CMD_SETTYPE]     = {"set_type", "[xml/uci/auto]", do_mib_setType, 0},
	[MIB_CMD_GETTYPE]     = {"get_type", NULL, do_mib_getType, 0},
	[MIB_CMD_HELP]        = {"help", NULL, do_mib_help, 0},
	[MIB_CMD_NONE]        = {NULL, NULL, NULL, 0}
};

struct MIB_CMD_s* get_mib_cmd(int argc, char **argv)
{
	int i;
	char *cmd;
	if(argc > 0 && (cmd = argv[0])) {
		for(i=0; i<(sizeof(mib_cmd_list)/sizeof(struct MIB_CMD_s)); i++) {
			struct MIB_CMD_s *c = &mib_cmd_list[i];
			if(c->cmd && !strcmp(c->cmd, cmd))
				return c;
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int pargc = argc;
	char **pargv = argv;
    struct MIB_CMD_s *cmd;
	
	if(pargc < 2) {
		do_mib_help(pargc, pargv);
        return(1);
	}
	
	pargc--;
	pargv = &pargv[1];
	cmd = get_mib_cmd(pargc, pargv);
	
	pargc--;
	if(pargc > 0) {
		pargv = &pargv[1];
	}
	else pargv = NULL;
	
	if(!cmd) {
		do_mib_help(pargc, pargv);
		return 1;
	}

	if(cmd->func) {
		if(cmd->do_mib) {
			mib_init();
			mib_config_dump(1);
			
			ret = cmd->func(pargc, pargv);
			
			mib_commit();
			mib_deinit();
		}
		else { 
			ret = cmd->func(pargc, pargv);
		}
	}
	else ret = 1;

    return ret;
}

int do_mib_help(int argc, char **argv)
{
	int i;
	printf("Usage: mib {command}\n");
	printf("command:\n");
	for(i=0; i<(sizeof(mib_cmd_list)/sizeof(struct MIB_CMD_s)); i++) {
		struct MIB_CMD_s *c = &mib_cmd_list[i];
		if(c->cmd) {
			printf("  %8s  %s\n", c->cmd, (c->help)?c->help:"");
		}
	}
	return 0;
}

int do_mib_get(int argc, char **argv)
{
	int ret = 0;
	char *name;
	if(argc > 0 && (name = argv[0])) {
		if(strchr(name, '.'))
			ret = mib_chain_get(name, -1, NULL, 0);
		else
			ret = mib_get_s(name, NULL, 0);
	}
	return (ret == 1) ? 0 : 1;
}

int do_mib_set(int argc, char **argv)
{
	int ret = 0;
	char *name, *val;
	if(argc > 1 && (name = argv[0]) && (val = argv[1])) {
		if(strchr(name, '.'))
			ret = mib_chain_update(name, -1, val, strlen(val));
		else {
			MIB_ENTRY *e = NULL;
			e = mib_lookup_entry(name);
			ret = _mib_set_s(name, e, val, strlen(val), 0);
		}
	}
	return (ret == 1) ? 0 : 1;
}

int do_mib_all(int argc, char **argv)
{
	int ret = 1;
	char *type = NULL;
	if(argc > 0) {
		type = argv[0];
	}
	mib_dump_all(type);
	return (ret == 1) ? 0 : 1;
}

int do_mib_commit(int argc, char **argv)
{
	int ret = 1;
	mib_commit();
	return (ret == 1) ? 0 : 1;
}

int do_mib_default(int argc, char **argv)
{
	int ret = 1;
	int type = MIB_DATA_UCI;
	
	if(argc >= 1) {
		if(!strcmp(argv[0], "hs")) {
			type = MIB_DATA_HS;
		}
		else if(!strcmp(argv[0], "cs")) {
			//type = MIB_DATA_CS;
			type = MIB_DATA_UCI;
		}
		else if(!strcmp(argv[0], "uci")) {
			type = MIB_DATA_UCI;
		} 
		else return 1;
	}
	
	mib_default(type);
	
	return (ret == 1) ? 0 : 1;
}

int do_mib_setType(int argc, char **argv)
{
	int ret = 0;
	char *name, *p;
	
	if(argc > 0 && (name = argv[0])) {
		ret = mib_set_type(name);
		
		if(ret == 1) {
			printf("Set MIB_TYPE = %s\n", name);
		}
	}
	return (ret == 1) ? 0 : 1;
}

int do_mib_getType(int argc, char **argv)
{
	int ret = 0;
	char name[64];
	
	ret = mib_get_type(name, sizeof(name)-1);
	if(ret == 1) {
		printf("MIB_TYPE = %s\n", name);
	}
	
	return (ret == 1) ? 0 : 1;
}
