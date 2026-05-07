#ifndef __MIB_UCI_H__
#define __MIB_UCI_H__

int mib_get_uci(const char *name, void *e, char *val, int size);
int mib_set_uci(const char *name, void *e, const char *val, int size);
int mib_chain_get_uci(const char *name, int index, char *val, int size);
int mib_chain_set_uci(const char *name, int index, const char *val, int size);
int mib_dump_all_uci(const char *type);
int mib_commit_uci(void);
int mib_config_dump_uci(int set_dump);
int mib_init_uci(void);
int mib_deinit_uci(void);
int mib_default_uci(int type);

#endif