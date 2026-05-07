#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlsave.h>
#include "common.h"
#include "mib_xml.h"

#define W_LOG(LV, fmt, arg...)

static int dump = 0;

#define CS_CONFIG_XML "/config/config.xml"
#define CS_CONFIG_XML_BAK "/config/config_bak.xml"
#define CS_CONFIG_DEFAULT_XML "/etc/config_default.xml"
static xmlDocPtr xml_doc_cs = NULL;
static xmlXPathContextPtr doc_cs_context = NULL;
static unsigned int cs_change = 0;

#define HS_CONFIG_XML "/config/config_hs.xml"
#define HS_CONFIG_XML_BAK "/config/config_hs_bak.xml"
#define HS_CONFIG_DEFAULT_XML "/etc/config_default_hs.xml"
static xmlDocPtr xml_doc_hs = NULL;
static xmlXPathContextPtr doc_hs_context = NULL;
static unsigned int hs_change = 0;

const char CONFIG_HEADER[] = "<Config_Information_File_8671>";
const char CONFIG_TRAILER[] = "</Config_Information_File_8671>";
const char CONFIG_HEADER_HS[] = "<Config_Information_File_HS>";
const char CONFIG_TRAILER_HS[] = "</Config_Information_File_HS>";

#define TAB_BLANK_COUNT 4

#define CHK_IS_DIGIT(p, end) ((end) ? (strspn(p, "0123456789") == (end-p)):(strspn(p, "0123456789") == strlen(p)))

xmlXPathObjectPtr getNodeSelect(xmlXPathContextPtr context, const char *xpath)
{
	xmlXPathObjectPtr result;
    if(context == NULL)
    {
        W_LOG(LOG_ERR, "context is NULL\n");
        return NULL;
    }

    result = xmlXPathEvalExpression((const xmlChar*)xpath, context);

    if(result && xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
		W_LOG(LOG_ERR, "Node is empty!!\n");
        xmlXPathFreeObject(result);
        return NULL;
    }

    return result;
}

xmlAttrPtr getNodeAttr(xmlNodePtr cur, const char *name)
{
	xmlAttr *attr = NULL;
    for (attr = cur->properties; attr; attr = attr->next) {
        if(!strcmp((const char*)attr->name, name)) { 
			return attr;
		}
    }
	return NULL;
}

static int loadXML(const char *filename, xmlDocPtr *pdoc, xmlXPathContextPtr *pcontext)
{
	xmlDocPtr doc = NULL;
	xmlXPathContextPtr context = NULL;
	
	if(!filename || !pdoc || !pcontext)
		return 1;

    doc = xmlReadFile(filename, NULL, XML_PARSE_NOWARNING);
	if(doc == NULL) {
		W_LOG(LOG_ERR, "Error to load %s\n", filename);
		return 1;
	} else {
		context = xmlXPathNewContext(doc);
	}
	
	if(pdoc) *pdoc = doc;
	if(pcontext) *pcontext = context;
	
	return 0;
}

static void saveXML(const char *filename, xmlDocPtr doc)
{
	xmlSaveCtxtPtr save;
	int saveopts = 0;
	
	saveopts |= XML_SAVE_FORMAT;
	saveopts |= XML_SAVE_NO_DECL;
	
	save = xmlSaveToFilename(filename, NULL, saveopts);
	
	xmlSaveDoc(save, doc);
	xmlSaveClose(save);
}

static void dump_mib_value(xmlNodePtr cur, int _blank)
{
	xmlAttrPtr nameAttr, valueAttr;
	if(cur && (nameAttr = getNodeAttr(cur, "Name"))) {
		valueAttr = getNodeAttr(cur, "Value");
		printf("%*s%s=%s\n", _blank, "", nameAttr->children->content, valueAttr->children->content);
	}
}

static void dump_mib_chain(xmlNodePtr cur, int num, int _blank, const char *_prefix)
{
	int chainNum=0;
	char path[256]={0};
	xmlAttrPtr nameAttr;
	xmlNodePtr node, lastChain=NULL;
	
	if(cur && (nameAttr = getNodeAttr(cur, "chainName"))) {
		if(_prefix && *_prefix && *_prefix!=' ')
			snprintf(path, sizeof(path), "%s.%s.%d", _prefix, nameAttr->children->content, num);
		else
			snprintf(path, sizeof(path), "%s.%d", nameAttr->children->content, num);
		
		printf("# %*s[%s]:\n", _blank, "", path);
		
		for (node = cur->children; node; node = node->next) {
			if(!strcmp((const char*)node->name, "Value")) {
				dump_mib_value(node, _blank+TAB_BLANK_COUNT);
			}
			else if(!strcmp((const char*)node->name, "chain")) {
				if(lastChain) {
					xmlAttrPtr n1, n2;
					n1 =  getNodeAttr(lastChain, "chainName");
					n2 =  getNodeAttr(node, "chainName");
					if(n1 && n2 && strcmp((const char*)n1->children->content, (const char*)n2->children->content))
						chainNum = 0;
				}
				dump_mib_chain(node, chainNum, _blank+TAB_BLANK_COUNT, "");
				lastChain = node;
				chainNum++;
			}
		}
	}
}

static void dump_mib(xmlNodePtr cur)
{
	int chainNum=0;
	xmlNodePtr node, lastChain=NULL;
	if(cur) {
		for (node = cur->children; node; node = node->next) {
			if(!strcmp((const char*)node->name, "Value")) {
				dump_mib_value(node, 0);
			}
			else if(!strcmp((const char*)node->name, "chain")) {
				if(lastChain) {
					xmlAttrPtr n1, n2;
					n1 =  getNodeAttr(lastChain, "chainName");
					n2 =  getNodeAttr(node, "chainName");
					if(n1 && n2 && strcmp((const char*)n1->children->content, (const char*)n2->children->content))
						chainNum = 0;
				}
				dump_mib_chain(node, chainNum, 0, NULL);
				lastChain = node;
				chainNum++;
			}
		}
	}
}

xmlXPathObjectPtr _mib_get_xml(xmlXPathContextPtr context, const char *name, char isChain, int *id) 
{
	const char *p, *p2, *p3;
	char path[256], *ppath; 
	xmlXPathObjectPtr result = NULL;
	xmlNodePtr cur = NULL;
	int i, len, cop;
	unsigned int last_id = 0;
	
	if(context && name) {
		if(isChain || strchr(name, '.')) {
			ppath = &path[0];
			len = sizeof(path) - 1;
			p = strchr(name, '.');
			if(p) { 
				cop = p - name; 
				p++; 
			} else cop = strlen(name);
			ppath += snprintf(ppath, len - (ppath - path), "//*/chain[@chainName=\"%.*s\"]", cop, name);
			while(p && *p && (len - (ppath - path)))
			{
				p2 = strchr(p, '.');
				if (CHK_IS_DIGIT(p, p2)) {
					last_id = strtoul(p, NULL, 10);
					ppath += snprintf(ppath, len - (ppath - path), "[%d]", last_id+1);
					if(p2) p2++;
				} else {
					if(p2) {
						cop = p2 - p;
						p2++;
						p3 = strchr(p2, '.');
					} else {
						cop = strlen(p);
					}
					if(p2 && CHK_IS_DIGIT(p2, p3)) {
						ppath += snprintf(ppath, len - (ppath - path), "/chain[@chainName=\"%.*s\"]", cop, p);
					} else {
						ppath += snprintf(ppath, len - (ppath - path), "/Value[@Name=\"%.*s\"]", cop, p);
					}
				}
				p = p2;
			}
		}
		else {
			snprintf(path, sizeof(path), "//*/Value[@Name=\"%s\"]", name);
		}
		result = getNodeSelect(context, path);
		if(result && id && *id < 0) {
			*id = last_id;
		}
	}
	return result;
}

int mib_get_xml(const char *name, void *e, char *val, int size) 
{
	xmlXPathObjectPtr result = NULL;
	xmlNodePtr cur = NULL;
	xmlAttrPtr attr = NULL;
	MIB_ENTRY *mib_e = (MIB_ENTRY *)e;
	
	if(!mib_e) return 0;
	
	if(mib_e->data == MIB_DATA_HS)
		result = _mib_get_xml(doc_hs_context, name, 0, NULL);
	else if(mib_e->data == MIB_DATA_CS)
		result = _mib_get_xml(doc_cs_context, name, 0, NULL);
	
	if(result) 
	{
		xmlNodeSetPtr nodeset = result->nodesetval;
		cur = nodeset->nodeTab[0];
		if(dump) dump_mib_value(cur, 0);
		if(val && size && (attr = getNodeAttr(cur, "Value"))) {
			memcpy(val, attr->children->content, size-1);
			*((char*)(val+size-1)) = '\0';
		}
		xmlXPathFreeObject(result);
		return 1;
	}
	else if(mib_e && mib_e->def_val)
	{
		if(dump){
			printf("%s=%s\n", name, mib_e->def_val);
		}
		if(val && size) {
			memcpy(val, mib_e->def_val, size-1);
			*((char*)(val+size-1)) = '\0';
		}
		return 1;
	}
	
	return 0;
}

int mib_set_xml(const char *name, void *e, const char *val, int size) 
{
	xmlXPathObjectPtr result = NULL;
	xmlNodePtr cur = NULL, root = NULL;
	xmlAttrPtr attr = NULL;
	MIB_ENTRY *mib_e = (MIB_ENTRY *)e;
	
	if(!val || size <=0) return 0;
	if(!mib_e) return 0;
	
	if(mib_e->data == MIB_DATA_HS)
		result = _mib_get_xml(doc_hs_context, name, 0, NULL);
	else if(mib_e->data == MIB_DATA_CS) {
		result = _mib_get_xml(doc_cs_context, name, 0, NULL);
	}
		
	if(result) 
	{
		xmlNodeSetPtr nodeset = result->nodesetval;
		cur = nodeset->nodeTab[0];
		if((attr = getNodeAttr(cur, "Value"))) {
			xmlNodeSetContent(attr->children, (const xmlChar*)val);
			if(dump) { dump_mib_value(cur, 0); }
			if(mib_e->data == MIB_DATA_HS) hs_change++;
			else cs_change++;
		}
		xmlXPathFreeObject(result);
		return 1;
	}
	else 
	{
		if(mib_e->data == MIB_DATA_HS)
			root = xmlDocGetRootElement(xml_doc_hs);
		else if(mib_e->data == MIB_DATA_CS)
			root = xmlDocGetRootElement(xml_doc_cs);
		if(root) {
			cur = xmlNewNode(NULL, (const xmlChar *)"Value");
			if(cur) {
				 xmlNewProp(cur, (const xmlChar *)"Name", (const xmlChar *)name);
				 xmlNewProp(cur, (const xmlChar *)"Value", (const xmlChar *)val);
				 xmlAddChild(root, cur);
				 xmlAddChild(root, xmlNewText((const xmlChar *)"\n"));
				 if(dump) { dump_mib_value(cur, 0); }
				 if(mib_e->data == MIB_DATA_HS) hs_change++;
				 else cs_change++;
				 return 1;
			}
		}
	}
	return 0;
}

int mib_chain_get_xml(const char *name, int index, char *val, int size) 
{
	xmlXPathObjectPtr result = NULL;
	xmlNodePtr cur = NULL;
	xmlAttrPtr attr = NULL;
	int i, len, len1, last_id = index;
	
	result = _mib_get_xml(doc_hs_context, name, 1, &last_id);
	if(result == NULL)
		result = _mib_get_xml(doc_cs_context, name, 1, &last_id);
	
	if(result) 
	{
		xmlNodeSetPtr nodeset = result->nodesetval;
		if(val && size) {
			cur = nodeset->nodeTab[0];
			if(!strcmp((const char*)cur->name, "Value")) {
				if((attr = getNodeAttr(cur, "Value"))) {
					len1 = strlen((const char*)attr->children->content);
					len = ((size-1) < len1) ? (size-1) : len1;
					memcpy(val, (const void*)attr->children->content, len);
					*((char*)(val+len)) = '\0';
				}
			}
		}
		
		if(dump) {
			for(i=0; i<nodeset->nodeNr; i++)
			{
				cur = nodeset->nodeTab[i];
				if(!strcmp((const char*)cur->name, "Value")) {
					dump_mib_value(cur, 0);
				} else {
					dump_mib_chain(cur, last_id+i, 0, NULL);
				}
			}
		}
		
		xmlXPathFreeObject(result);
		return 1;
	}
	return 0;
}

int mib_chain_set_xml(const char *name, int index, const char *val, int size)
{
	xmlXPathObjectPtr result = NULL;
	xmlNodePtr cur = NULL;
	xmlAttrPtr attr = NULL;
	int last_id = index;
	int type = 0;

	if(!val || size <=0) return 0;
	
	result = _mib_get_xml(doc_hs_context, name, 1, &last_id);
	if(result == NULL) {
		result = _mib_get_xml(doc_cs_context, name, 1, &last_id);
		type = 1;
	}

	if(result) 
	{
		xmlNodeSetPtr nodeset = result->nodesetval;
		cur = nodeset->nodeTab[0];
		if(!strcmp((const char*)cur->name, "Value")) {
			if((attr = getNodeAttr(cur, "Value"))) {
				xmlNodeSetContent(attr->children, (const xmlChar*)val);
				if(dump) { dump_mib_value(cur, 0); }
				if(type == 0) hs_change++;
				else cs_change++;
				return 1;
			}
		} else if(!strcmp((const char*)cur->name, "chain")) {
			// no support;
		}
	}
	return 0;
}

int mib_dump_all_xml(const char *type)
{
	xmlNodePtr cur = NULL;
	
	if((type == NULL || !strcmp(type, "hs")) && xml_doc_hs) {
		cur = xmlDocGetRootElement(xml_doc_hs);
		if(cur) {
			printf("@@@@@@ Hardware Config @@@@@@\n");
			dump_mib(cur);
			printf("\n");
		}
	}
	
	if((type == NULL || !strcmp(type, "cs")) && xml_doc_cs) {
		cur = xmlDocGetRootElement(xml_doc_cs);
		if(cur) {
			printf("\n@@@@@@ Current Config @@@@@@\n");
			dump_mib(cur);
			printf("\n");
		}
	}

	return 1;
}

int mib_commit_xml(void)
{
	if(cs_change > 0 && xml_doc_cs) {
		saveXML(CS_CONFIG_XML, xml_doc_cs);
	}
	
	if(hs_change > 0 && xml_doc_hs) {
		saveXML(HS_CONFIG_XML, xml_doc_hs);
	}
	
	return 0;
}

int mib_config_dump_xml(int set_dump)
{
	dump = set_dump;
	return 0;
}

int check_xor_encrypt(const char *inputfile);
void xor_decrypt(const char *inputfile, char outputfile[]);
int check_and_loadxml(const char *infile, xmlDocPtr *xml, xmlXPathContextPtr *doc)
{
	struct stat st;
	char tempFile[128]={0}, bak_name[128]={0};
	if(stat(infile, &st) == 0) {
		if(check_xor_encrypt(infile) == 1) {
			xor_decrypt(infile, tempFile);
			if(tempFile[0]) {
				snprintf(bak_name, sizeof(bak_name), "%s.enc_bak", infile);
				va_cmd("mv", 3, 1, "-f", infile, bak_name);//rename(infile, bak_name);
				va_cmd("mv", 3, 1, "-f", tempFile, infile);//rename(tempFile, infile);
			}
		}
		if(loadXML(infile, xml, doc) == 0)
			return 0;
	}
	return -1;
}

int mib_init_xml(void)
{	
	xmlSetGenericErrorFunc(NULL, NULL);
	xmlSetStructuredErrorFunc(NULL, NULL);
	
	if(check_and_loadxml(CS_CONFIG_XML, &xml_doc_cs, &doc_cs_context) != 0)
		check_and_loadxml(CS_CONFIG_XML_BAK, &xml_doc_cs, &doc_cs_context);
	
	if(check_and_loadxml(HS_CONFIG_XML, &xml_doc_hs, &doc_hs_context) != 0)
		check_and_loadxml(HS_CONFIG_XML_BAK, &xml_doc_hs, &doc_hs_context);
	
	return 0;
}

int mib_deinit_xml(void)
{
	/*free the document */
	if(doc_cs_context) { xmlXPathFreeContext(doc_cs_context); doc_cs_context = NULL; }
    if(xml_doc_cs) { xmlFreeDoc(xml_doc_cs); xml_doc_cs = NULL; }
	
	if(doc_hs_context) { xmlXPathFreeContext(doc_hs_context); doc_hs_context = NULL; }
    if(xml_doc_hs) { xmlFreeDoc(xml_doc_hs); xml_doc_hs = NULL; }

    xmlCleanupParser();
	
	return 0;
}

int mib_default_xml(int type)
{
	if(type == MIB_DATA_CS)
		file_copy(CS_CONFIG_DEFAULT_XML, CS_CONFIG_XML);
	
	if(type == MIB_DATA_HS)
		file_copy(HS_CONFIG_DEFAULT_XML, HS_CONFIG_XML);
	
	return 0;
}

const char XOR_KEY[] = "tecomtec";
/* used by mktemp to generate a unique temporary filename. The last six
   characters of template must be XXXXXX. */
#define MK_TEMPLATE "/tmp/cnftmp_XXXXXX"
void xor_decrypt(const char *inputfile, char outputfile[])
{
	FILE *input  = fopen(inputfile, "rb");
	FILE *output = NULL;
	unsigned char buffer[1024];
	int fd = -1;

	strcpy(outputfile, MK_TEMPLATE);
	if(input != NULL && (fd = mkstemp(outputfile)) >= 0 
		&& (output = fdopen(fd, "wb"))) 
	{
		
		size_t count, i, j = 0;
		do {
			count = fread(buffer, 1, sizeof(buffer), input);
			for(i = 0; i<count; ++i) {
				buffer[i] ^= XOR_KEY[j++];
				if(XOR_KEY[j] == '\0')
					j = 0; /* restart at the beginning of the key */
			}
			fwrite(buffer, 1, count, output);
		} while(!feof(input));
	}
	else outputfile[0] = 0;
	
	if(input) fclose(input);
	if(output) fclose(output);
	else if(fd >= 0) close(fd);
}

void xor_encrypt2(char *buf, int len, int *index)
{
	int i;
	int idx;
	if (NULL == buf || 0 == len) {
		return;
	}
	idx = *index;
	for (i = 0; i < len; ++i) {
		buf[i] ^= XOR_KEY[idx];
		idx++;
		if (XOR_KEY[idx] == '\0'){
			idx = 0;
		}
	}
	*index = idx;
}

void xor_encrypt(const char *inputfile, char outputfile[])
{
	FILE *input  = fopen(inputfile, "rb");
	FILE *output = NULL;
	char *buf = NULL;
	int fd = -1;
	unsigned long len;
	int i, j;

	strcpy(outputfile, MK_TEMPLATE);
	if(input != NULL && (fd = mkstemp(outputfile)) >= 0 
		&& (output = fdopen(fd, "wb"))) 
	{
		fseek(input, 0, SEEK_END);
		len = ftell(input);
		fseek(input, 0, SEEK_SET);
		buf = (char*)malloc(len);
		if(buf) {
			fread(buf, 1, len, input);
			j = 0;
			for(i = 0; i<len; i++) {
				buf[i] ^= XOR_KEY[j++];
				if(XOR_KEY[j] == '\0')
					j = 0; 
			}
			fwrite(buf, 1, len, output);
		} else outputfile[0] = 0;
	}
	else outputfile[0] = 0;
	
	if(input) fclose(input);
	if(output) fclose(output);
	else if(fd >= 0) close(fd);
}

static char *get_line(FILE *fp, char *s, int size)
{
	char *pstr;

	while (1) {
		if (!fgets(s, size, fp))
			return NULL;
		pstr = trim_white_space(s);
		if (strlen(pstr))
			break;
	}

	return pstr;
}

int check_xor_encrypt(const char *inputfile)
{
	FILE *input  = NULL;
	char xml_line[1024];
	char *pstr;
	
	if(inputfile != NULL){
		input  = fopen(inputfile, "rb");
		if(input){
			while(!feof(input)){
				pstr = get_line(input, xml_line, sizeof(xml_line));
				if(pstr == NULL) continue;
				if (!strcmp(pstr, CONFIG_HEADER) || !strcmp(pstr, CONFIG_HEADER_HS)){
					fclose(input);
					return 0; ////file not encrypted
				}
				break;
			}
			fclose(input);
		}
		else
			return -1; //file doesn't exist
	}
	else
		return -1; //file doesn't exist
	return 1;//file encrypted
}


