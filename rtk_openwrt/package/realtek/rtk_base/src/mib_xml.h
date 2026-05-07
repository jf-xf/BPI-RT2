#ifndef __MIB_XML_H__
#define __MIB_XML_H__

int mib_get_xml(const char *name, void *e, char *val, int size);
int mib_set_xml(const char *name, void *e, const char *val, int size);
int mib_chain_get_xml(const char *name, int index, char *val, int size);
int mib_chain_set_xml(const char *name, int index, const char *val, int size);
int mib_dump_all_xml(const char *type);
int mib_commit_xml(void);
int mib_config_dump_xml(int set_dump);
int mib_init_xml(void);
int mib_deinit_xml(void);
int mib_default_xml(int type);

#endif