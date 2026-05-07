#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <uci.h>
#include "common.h"
#include "mib_uci.h"

static struct uci_context *uci_ctx = NULL;
static struct uci_package *mib_page = NULL;
static int uci_change = 0;
static int dump = 0;

#define UCI_DATABASE "rtkmib"
#define TAB_BLANK_COUNT 4

#define UCI_CONFIG_DEFAULT "/etc/rtkmib_default"
#define UCI_CONFIG "/etc/config/rtkmib"

static int load_uci(void)
{
	int rc;
	if(uci_ctx) return 0;
	uci_ctx = uci_alloc_context(); 
	if(uci_ctx)
	{
		rc = uci_load(uci_ctx, UCI_DATABASE, &mib_page);
		if(rc == UCI_OK)
			return 0;
	}
	return -1;
}

static int unload_uci(void)
{
	if(mib_page){
		uci_unload(uci_ctx, mib_page);
		mib_page = NULL;
	}
	if(uci_ctx) {
		uci_free_context(uci_ctx);
		uci_ctx = NULL;
	}
	return 0;
}

static void mib_dump_uci_option(struct uci_option *opt, int _blanck)
{
	struct uci_element *e;
	
	if(!opt) return;

	if(opt->type == UCI_TYPE_LIST)
	{
		printf("%*s%s=", _blanck, "", opt->e.name);
		uci_foreach_element(&opt->v.list, e)
			printf("'%s' ", e->name);
		printf("\n");
	}
	else {
		printf("%*s%s=%s\n", _blanck, "", opt->e.name, opt->v.string);
	}
}

static void mib_dump_uci_section(struct uci_section *sec, int index, int _blanck)
{
	struct uci_element *e;
	
	if(!sec) return;
	
	if(1 || !strcmp(sec->e.name, "mib"))
		printf("# %*s[%s]\n", _blanck, "", sec->e.name);
	else 
		printf("# %*s[%s.%d]\n", _blanck, "", sec->e.name, index);
	
	uci_foreach_element(&sec->options, e)
	{
		mib_dump_uci_option(uci_to_option(e), _blanck+TAB_BLANK_COUNT);
	}
}

static void mib_dump_uci_package(struct uci_package *page)
{
	int i=0;
	struct uci_element *e;
	
	if(!page) return;
	
	uci_foreach_element(&page->sections, e)
	{
		mib_dump_uci_section(uci_to_section(e), i, 0);
		i++;
	}
}

int mib_get_uci(const char *name, void *e, char *val, int size)
{
	struct uci_section *s;
	struct uci_option *o;
	MIB_ENTRY *mib_e = (MIB_ENTRY *)e;
	
	if(!uci_ctx || !mib_page) return -1;
	
	s = uci_lookup_section(uci_ctx, mib_page, "mib");
	if(s)
	{
		o = uci_lookup_option(uci_ctx, s, name);
		if(o)
		{
			if(val) {
				strncpy(val, o->v.string, size-1);
				*((char*)(val+size-1)) = '\0';
			}
			if(dump)
				printf("%s=%s\n", name, o->v.string);
			return 1;
		}
		else if(mib_e && mib_e->def_val)
		{
			if(val) {
				strncpy(val, mib_e->def_val, size-1);
				*((char*)(val+size-1)) = '\0';
			}
			if(dump)
				printf("%s=%s\n", name, mib_e->def_val);
		}
	}
	return 0;
}

int mib_set_uci(const char *name, void *e, const char *val, int size)
{
	int ret = 0;
	struct uci_ptr ptr;
	struct uci_section *s;
	MIB_ENTRY *mib_e = (MIB_ENTRY *)e;
	
	if(!val || size<=0) return -1;
	if(!uci_ctx || !mib_page) return -1;

	s = uci_lookup_section(uci_ctx, mib_page, "mib");
	if(s)
	{
		memset(&ptr, 0, sizeof(ptr));
		ptr.p = mib_page;
		ptr.s = s;
		ptr.option = name;
		ptr.value = val;
		ret = uci_set(uci_ctx, &ptr);
		if(ret == 0) {
			if(dump) printf("%s=%s\n", name, val);
			uci_change++;
		}
	}
	return (ret == 0) ? 1:0;
}

int mib_chain_get_uci(const char *name, int index, char *val, int size) 
{
	return 0;
}

int mib_chain_set_uci(const char *name, int index, const char *val, int size)
{
	return 0;
}

int mib_dump_all_uci(const char *type)
{
	mib_dump_uci_package(mib_page);
	
	return 0;
}

int mib_commit_uci(void)
{
	if(!uci_ctx || !mib_page) return -1;
	if(uci_change) uci_commit(uci_ctx, &mib_page, false);
	return 0;
}

int mib_config_dump_uci(int set_dump)
{
	dump = set_dump;
	return 0;
}

int mib_init_uci(void)
{
	load_uci();
	return 0;
}

int mib_deinit_uci(void)
{
	unload_uci();
	return 0;
}

int mib_default_uci(int type)
{
	file_copy(UCI_CONFIG_DEFAULT, UCI_CONFIG);
	
	return 0;
}

