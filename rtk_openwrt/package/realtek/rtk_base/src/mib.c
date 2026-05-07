#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mib_uci.h"
#include "mib_xml.h"
#include "common.h"

static unsigned int mib_type = MIB_DATABASE_XML | MIB_DATABASE_UCI;
static unsigned char __mib_init = 0;

const char *_mib_format_translate(const char *name, MIB_ENTRY *e, const char *val, int size, char **output, int *osize)
{
	const char *v = val;
	if(output == NULL || osize == NULL || name == NULL || size <= 0 ||
		e == NULL || e->name == NULL || e->type == MIB_TYPE_STR || e->type == MIB_TYPE_NONE)
		return v;
	if( e->type == MIB_TYPE_INT )
	{
		*output = malloc(sizeof(int));
		*((int*)(*output)) = atoi(val);
		v = *output;
		*osize = sizeof(int);
	}
	else if( e->type == MIB_TYPE_INT8 || e->type == MIB_TYPE_BOOLEAN)
	{
		*output = malloc(sizeof(char));
		*((char*)(*output)) = (char)atoi(val);
		v = *output;
		*osize = sizeof(char);
	}
	else if( e->type == MIB_TYPE_DOUBLE )
	{
		*output = malloc(sizeof(double));
		*((double*)(*output)) = (double)atof(val);
		v = *output;
		*osize = sizeof(double);
	}
	else if( e->type == MIB_TYPE_BINSTR )
	{
		*osize = size+(size/2)+2;
		*output = (char*)calloc(*osize, 1);
		binStr2Str(val, size, *output, *osize, ',');
		v = *output;
	}

	return v;
}

#define NUM_SIZE 33
const char *_mib_format_translate_r(const char *name, MIB_ENTRY *e, const char *val, int size, char **output, int *osize)
{
	const char *v = val;
	if(output == NULL || osize == NULL || name == NULL || size <= 0 ||
		e == NULL || e->name == NULL || e->type == MIB_TYPE_STR || e->type == MIB_TYPE_NONE)
		return v;
	if( e->type == MIB_TYPE_INT )
	{
		*output = calloc(1, NUM_SIZE);
		snprintf(*output, NUM_SIZE-1, "%d", *((int*)val));
		v = *output;
		*osize = 33;
	}
	else if( e->type == MIB_TYPE_INT8 || e->type == MIB_TYPE_BOOLEAN)
	{
		*output = calloc(1, NUM_SIZE);
		snprintf(*output, NUM_SIZE-1, "%hhu", *((char*)val));
		v = *output;
		*osize = 33;
	}
	else if( e->type == MIB_TYPE_DOUBLE )
	{
		*output = calloc(1, NUM_SIZE);
		snprintf(*output, NUM_SIZE-1, "%f", *((double*)val));
		v = *output;
		*osize = 33;
	}
	else if( e->type == MIB_TYPE_BINSTR )
	{
		*osize = size+(size/2)+2;
		*output = (char*)calloc(*osize, 1);
		binStr2Str(val, size, *output, *osize, ',');
		v = *output;
	}

	return v;
}

int mib_init(void)
{
	mib_get_type(NULL, 0);
	
	if((mib_type & MIB_DATABASE_XML))
		mib_init_xml();
	
	if((mib_type & MIB_DATABASE_UCI))
		mib_init_uci();
	
	__mib_init = 1;
	
	return 0;
}

int mib_deinit(void)
{
	if((mib_type & MIB_DATABASE_XML))
		mib_deinit_xml();
	
	if((mib_type & MIB_DATABASE_UCI))
		mib_deinit_uci();
	
	__mib_init = 0;
	return 0;
}

int mib_commit(void)
{
	if((mib_type & MIB_DATABASE_XML))
		mib_commit_xml();
	
	if((mib_type & MIB_DATABASE_UCI))
		mib_commit_uci();
	
	sync();
	
	return 0;
}

int mib_config_dump(int dump)
{
	if((mib_type & MIB_DATABASE_XML))
		mib_config_dump_xml(dump);
	
	if((mib_type & MIB_DATABASE_UCI))
		mib_config_dump_uci(dump);
	
	return 0;
}

int mib_dump_all(const char *type)
{
	if((mib_type & MIB_DATABASE_XML)) {
		printf("### RTKMIB XML ###\n");
		mib_dump_all_xml(type);
	}
	
	if((mib_type & MIB_DATABASE_UCI)) {
		printf("### RTKMIB UCI ###\n");
		mib_dump_all_uci(type);
	}
	
	printf("\n");
	
	return 0;
}

int _mib_get_s(const char *name, MIB_ENTRY *e, char *val, int size)
{
	int ret = 0, csize;
	char buf[64], *pval;
	
	if(e && (e->type == MIB_TYPE_INT || e->type == MIB_TYPE_INT8 || 
		e->type == MIB_TYPE_DOUBLE || e->type == MIB_TYPE_BOOLEAN)) {
		pval = &buf[0];
		csize = sizeof(buf);
	} else {
		pval = val;
		csize = size;
	}

	if(ret != 1 && e && (e->data == MIB_DATA_HS || e->data == MIB_DATA_CS))
		ret = mib_get_xml(name, (void*)e, pval, csize);
	
	if(ret != 1 && (!e || e->data == MIB_DATA_UCI))
		ret = mib_get_uci(name, (void*)e, pval, csize);
	
	if(ret != 1 && e && e->def_val) {
		pval = (char*)e->def_val;
		ret = 1;
	}

	if(ret == 1) {
		int s = csize;
		char *v = NULL;
		_mib_format_translate(name, e, pval, csize, &v, &s);
		if(v != NULL) {
			memcpy(val, v, size);
			free(v);
		}
	}
	
	return ret;
}

int mib_get_s(const char *name, char *val, int size)
{
	MIB_ENTRY *e;
	e = mib_lookup_entry(name);
	return _mib_get_s(name, e, val, size);
}

int mib_get_s2(int mibid, char *val, int size)
{
	MIB_ENTRY *e;
	e = mib_lookup_entry2(mibid);
	return (e) ? _mib_get_s(e->name, e, val, size) : 0;
}

int _mib_set_s(const char *name, MIB_ENTRY *e, const char *val, int size, char trans)
{
	int ret = 0;
	char *fix_val = NULL;
	const char *cval;
	int csize;
	
	csize = size;
	if(trans)
		cval = _mib_format_translate_r(name, e, val, size, &fix_val, &csize);
	else
		cval = val;

	if(cval == NULL)
		return -1;
	
	if(ret != 1 && e && (e->data == MIB_DATA_HS || e->data == MIB_DATA_CS))
		ret = mib_set_xml(name, (void*)e, cval, csize);
	
	if(ret != 1 && (!e || e->data == MIB_DATA_UCI))
		ret = mib_set_uci(name, (void*)e, cval, csize);
	
	if(fix_val) free(fix_val);
	
	return ret;
}

int mib_set_s(const char *name, const char *val, int size)
{
	MIB_ENTRY *e;
	e = mib_lookup_entry(name);
	return _mib_set_s(name, e, val, size,1);
}

int mib_set_s2(int mibid, const char *val, int size)
{
	MIB_ENTRY *e;
	e = mib_lookup_entry2(mibid);
	return (e) ? _mib_set_s(e->name, e, val, size, 1) : 0;
}

int mib_chain_get(const char *name, int index, char *val, int size) 
{
	int ret = 0;
	if(ret != 1 && (mib_type & MIB_DATABASE_XML))
		ret = mib_chain_get_xml(name, index, val, size);
	
	if(ret != 1 && (mib_type & MIB_DATABASE_UCI))
		ret = mib_chain_get_uci(name, index, val, size);
	
	return 0;
}

int mib_chain_update(const char *name, int index, const char *val, int size) 
{
	int ret = 0;
	if(ret != 1 && (mib_type & MIB_DATABASE_XML))
		ret = mib_chain_set_xml(name, index, val, size);
	
	if(ret != 1 && (mib_type & MIB_DATABASE_UCI))
		ret = mib_chain_set_uci(name, index, val, size);
	
	return 0;
}

int mib_default(int type) 
{
	int ret = 0, i;
	extern MIB_ENTRY mib_tree[];
	char buf[512];
	const char *v;

	if(__mib_init == 1) {
		mib_deinit();
	}

	if(type == MIB_DATA_ALL || type == MIB_DATA_HS) {
		mib_default_xml(MIB_DATA_HS);
	}
	/* don't default cs because use UCI instead of CS setting
	if(type == MIB_DATA_ALL || type == MIB_DATA_CS) {
		mib_default_xml(MIB_DATA_CS);
	}
	*/
	if(type == MIB_DATA_ALL || type == MIB_DATA_UCI) {
		mib_default_uci(MIB_DATA_UCI);
	}

	mib_init();
	for(i=0; i<MIB_MAX; i++)
	{
		MIB_ENTRY *e = &mib_tree[i];
		if(e->name && (type == MIB_DATA_ALL || e->data == type)) {
			ret = mib_get_s2(e->id, buf, sizeof(buf));
			if(ret == 1) v = buf;
			else if(e->def_val) v = e->def_val;
			else v = "";
			mib_set_s2(e->id, v, strlen(v));
		}
	}
	mib_commit();
	mib_deinit();

	return 0;
}

int mib_set_type(const char *type) 
{
	FILE *fp;
	if(type) {
		if(!strcmp(type, "xml"))
			mib_type = MIB_DATABASE_XML;
		else if(!strcmp(type, "uci"))
			mib_type = MIB_DATABASE_UCI;
		else if(!strcmp(type, "auto"))
			mib_type = MIB_DATABASE_UCI | MIB_DATABASE_XML;
		else 
			return 0;

		if((fp = fopen(MIB_DATA_TYPE, "w"))) {
			fprintf(fp, "%u", mib_type);
			fclose(fp);
		}
		return 1;
	}
	return 0;
}

int mib_get_type(char *type, int size) 
{
	FILE *fp;
	unsigned int val = 0;
	if((fp = fopen(MIB_DATA_TYPE, "r"))) {
		fscanf(fp, "%u", &val);
		fclose(fp);
	}

	if((val & (MIB_DATABASE_XML|MIB_DATABASE_UCI)) != 0)
		mib_type = (val & (MIB_DATABASE_XML|MIB_DATABASE_UCI));

	if(type) {
		if(mib_type == MIB_DATABASE_UCI)
			snprintf(type, size, "uci");
		else if(mib_type == MIB_DATABASE_XML)
			snprintf(type, size, "xml");
		else
			snprintf(type, size, "auto");
	}
	
	return 1;
}

