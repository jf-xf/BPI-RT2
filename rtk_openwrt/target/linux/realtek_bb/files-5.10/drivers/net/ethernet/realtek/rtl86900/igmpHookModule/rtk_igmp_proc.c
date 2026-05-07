#include <br_private.h>
#include <linux/proc_fs.h>
// #include <linux/seq_file.h>
// #include <linux/inet.h>
#include <fs/proc/internal.h>

#include <rtk_igmp_struct.h>
#include <rtk_igmp_proc.h>
#include <rtk_igmp_hook.h>
#include <rtk_igmp_debug.h>
#include <rtk_igmp_snooping.h>

#define CPY_BUF_SIZE_MAX (256)


struct proc_dir_entry *rtk_igmp_proc_dir=NULL;
struct proc_dir_entry *rtk_igmp_dump_proc_dir=NULL;
struct proc_dir_entry *rtk_igmp_ctrl_proc_dir=NULL;

int rtk_igmp6_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp6_proc_read(struct seq_file *s, void *v);
int rtk_igmp_proc_read(struct seq_file *s, void *v);
int rtk_igmp_mrouteMapping_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_mrouteMapping_read(struct seq_file *s, void *v);
int rtk_igmp_debug_read(struct seq_file *s, void *v);
int rtk_igmp_debug_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_br_proc_read(struct seq_file *s, void *v);
int rtk_igmp_br_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_whiteList_proc_read(struct seq_file *s, void *v);
int rtk_igmp_whiteList_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_blackList_proc_read(struct seq_file *s, void *v);
int rtk_igmp_blackList_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_ignoreGroup_proc_read(struct seq_file *s, void *v);
int rtk_igmp_ignoreGroup_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_igmphook_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_igmphook_proc_read(struct seq_file *s, void *v);
int rtk_igmp_igmphwctrl_proc_read(struct seq_file *s, void *v);
int rtk_igmp_igmphwctrl_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_igmphwcbMode_proc_read(struct seq_file *s, void *v);
int rtk_igmp_igmphwcbMode_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_procInternalCtrl_proc_read(struct seq_file *s, void *v);
int rtk_igmp_procInternalCtrl_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_groupWeight_proc_read(struct seq_file *s, void *v);
int rtk_igmp_groupWeight_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_devStatistic_read(struct seq_file *s, void *v);
int rtk_igmp_devStatistic_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_nextGroupStatistic_read(struct seq_file *s, void *v);
int rtk_igmp_nextGroupStatistic_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_unknownFlood_proc_read(struct seq_file *s, void *v);
int rtk_igmp_unknownFlood_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
int rtk_igmp_trapWlanToPS_proc_read(struct seq_file *s, void *v);
int rtk_igmp_trapWlanToPS_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
int rtk_igmp_skbMarkExtraSwitchPortId_proc_read(struct seq_file *s, void *v);
int rtk_igmp_skbMarkExtraSwitchPortId_proc_write(struct file *file,  char *buffer, unsigned long count, void *data);
#endif

rtk_igmp_proc_t igmpProc[]=
{
	//dump
	{
		.name="devConfig" ,
		.get = rtk_igmp_dump_allDevConfig ,
		.set = NULL ,
		.dir = PROC_DIR_IGMP_DUMP,
	},
	{
		.name="memUsed" ,
		.get = rtk_igmp_dump_memoryUsed ,
		.set = NULL ,
		.dir = PROC_DIR_IGMP_DUMP,
	},
	{
		.name="igmpInfo" ,
		.get = rtk_igmp_allDump ,
		.set = NULL ,
		.dir = PROC_DIR_IGMP_DUMP,
	},
	{
		.name="igmpDebug" ,
		.get = rtk_igmp_debug_read,
		.set = rtk_igmp_debug_write ,
		.dir = PROC_DIR_IGMP_DUMP,
	},
	{
		.name="devStatistic" ,
		.get = rtk_igmp_devStatistic_read,
		.set = rtk_igmp_devStatistic_write ,
		.dir = PROC_DIR_IGMP_DUMP,
	},
	{
		.name="nextGroupStatistic" ,
		.get = rtk_igmp_nextGroupStatistic_read,
		.set = rtk_igmp_nextGroupStatistic_write ,
		.dir = PROC_DIR_IGMP_DUMP,
	},


	//control
	{
		.name="igmpConfig" ,
		.get = rtk_igmp_proc_read ,
		.set = rtk_igmp_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="igmp6Config" ,
		.get = rtk_igmp6_proc_read ,
		.set = rtk_igmp6_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},

	{
		.name="brConfig" ,
		.get = rtk_igmp_br_proc_read ,
		.set = rtk_igmp_br_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="whiteList" ,
		.get = rtk_igmp_whiteList_proc_read ,
		.set = rtk_igmp_whiteList_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="blackList" ,
		.get = rtk_igmp_blackList_proc_read ,
		.set = rtk_igmp_blackList_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="ignoreGroup" ,
		.get = rtk_igmp_ignoreGroup_proc_read ,
		.set = rtk_igmp_ignoreGroup_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},	
	{
		.name="groupWeight" ,
		.get = rtk_igmp_groupWeight_proc_read ,
		.set = rtk_igmp_groupWeight_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},		
	{
		.name="igmphook" ,
		.get = rtk_igmp_igmphook_proc_read ,
		.set = rtk_igmp_igmphook_proc_write ,
		.unlockBefortWrite=1,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="hwControl" ,
		.get = rtk_igmp_igmphwctrl_proc_read ,
		.set = rtk_igmp_igmphwctrl_proc_write ,
		.unlockBefortWrite=1,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="hwCbMode" ,
		.get = rtk_igmp_igmphwcbMode_proc_read ,
		.set = rtk_igmp_igmphwcbMode_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="internalCtrlProc" ,
		.get = rtk_igmp_procInternalCtrl_proc_read ,
		.set = rtk_igmp_procInternalCtrl_proc_write ,
		.unlockBefortWrite=1,
		.dir = PROC_DIR_IGMP_CTRL,
	},		
	{
		.name="unknownFlood" ,
		.get = rtk_igmp_unknownFlood_proc_read ,
		.set = rtk_igmp_unknownFlood_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
	{
		.name="trapWlanToPS" ,
		.get = rtk_igmp_trapWlanToPS_proc_read ,
		.set = rtk_igmp_trapWlanToPS_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
	{
		.name="skbMarkExtraSwitchPortId" ,
		.get = rtk_igmp_skbMarkExtraSwitchPortId_proc_read ,
		.set = rtk_igmp_skbMarkExtraSwitchPortId_proc_write ,
		.dir = PROC_DIR_IGMP_CTRL,
	},
#endif


#if 0
	{
		.name="mrouteMapping" ,
		.get = rtk_igmp_mrouteMapping_read ,
		.set = rtk_igmp_mrouteMapping_write,
		.dir = PROC_DIR_IGMP_CTRL,
	},
#endif

};



static void *rtk_igmp_single_start(struct seq_file *p, loff_t *pos)
{
	return NULL + (*pos == 0);
}

static void *rtk_igmp_single_next(struct seq_file *p, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void rtk_igmp_single_stop(struct seq_file *p, void *v)
{
}



int rtk_igmp_single_open(struct file *file, int (*show)(struct seq_file *, void *),void *data)
{
	struct seq_operations *op = kmalloc(sizeof(*op), GFP_ATOMIC);
	int res = -ENOMEM;

	if (op) {
		op->start = rtk_igmp_single_start;
		op->next = rtk_igmp_single_next;
		op->stop = rtk_igmp_single_stop;
		op->show = show;
		res = seq_open(file, op);
		if (!res)
			((struct seq_file *)file->private_data)->private = data;
		else
			kfree(op);
	}
	return res;
}


static int rtk_igmp_nullDebugGet(struct seq_file *s, void *v)
{
       return 0;
}

static int rtk_igmp_nullDebugSingleOpen(struct inode *inode, struct file *file)
{
       return(single_open(file, rtk_igmp_nullDebugGet, NULL));
}

static int rtk_igmp_commonDebugSingleRead(struct seq_file *s, void *v)
{
	int i, ret = -1;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	//========================= Critical Section Start =========================//
	for( i=0; i< (sizeof(igmpProc)/sizeof(rtk_igmp_proc_t)) ;i++)
	{
		if(igmpProc[i].inode_id==(unsigned int)s->file->f_inode->i_ino)
		{
			ret = igmpProc[i].get(s, v);
			break;
		}
	}
	//========================= Critical Section End ===========================//
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);

	return ret;
}


static int rtk_igmp_commonDebugSingleOpen(struct inode *inode, struct file *file)
{
    int i, ret = -1;

	//========================= Critical Section Start =========================//
    for( i=0; i< (sizeof(igmpProc)/sizeof(rtk_igmp_proc_t)) ;i++)
    {
		//printk("common_single_open inode_id=%u i_ino=%u\n",igmpProc[i].inode_id,(unsigned int)inode->i_ino);
        if(igmpProc[i].inode_id==(unsigned int)inode->i_ino)
        {
			ret = rtk_igmp_single_open(file, rtk_igmp_commonDebugSingleRead, NULL);
			break;
        }
    }
	//========================= Critical Section End =========================//

    return ret;
}

static ssize_t rtk_igmp_commonDebugSingleWrite(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
    int i, ret = -1;
	char procBuffer[CPY_BUF_SIZE_MAX] = {0};
	int len = (count >= (CPY_BUF_SIZE_MAX) ) ? (CPY_BUF_SIZE_MAX) : count;

	/* write data to the buffer */
	if ( copy_from_user(procBuffer, userbuf, len) ) {
		return -EFAULT;
	}
	procBuffer[len-1] = '\0';	//echo string from user is  abc	=> 'a''b''c''\n' len=4 and no char '\0' , replace '\n' to '\0'

	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	//========================= Critical Section Start =========================//
    for( i=0; i< (sizeof(igmpProc)/sizeof(rtk_igmp_proc_t)) ;i++)
    {

		//printk("common_single_write inode_id=%u i_ino=%u\n",igmpProc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);
        if(igmpProc[i].inode_id==(unsigned int)file->f_inode->i_ino)
        {
			if(igmpProc[i].unlockBefortWrite)
				igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
			ret = igmpProc[i].set(file,procBuffer,len,off);
			break;
        }
    }
	//========================= Critical Section End =========================//
	if((i!=(sizeof(igmpProc)/sizeof(rtk_igmp_proc_t))) && !igmpProc[i].unlockBefortWrite)
		igmp_spin_unlock_bh(igmpSysdb.lock_igmp);

    return count;
}


extern int in4_pton(const char *src, int srclen, u8 *dst, int delim, const char **end);
extern int in6_pton(const char *src, int srclen, u8 *dst, int delim, const char **end);
extern char * strstr(const char *,const char *);

enum rtk_igmp_parseDataType
{
   MAC=0,
   V4IP,
   V6IP,
   STRING_EN_DIS, //warning : only use for uint32* parameterGetAddr
   UNSIGNED_LONG, //uint32
   SINGED_LONG,	  //int32
   STRING,		  //warning : should sure string non-overflow
};

void _rtk_igmp_str2mac(unsigned char *mac_string,rtk_mac_t *pMacEntry)
{
	int i,j=0,k=0;
	memset(pMacEntry,0,sizeof(rtk_mac_t));
	for(i=0;i<strlen(mac_string);i++)
	{
		if(mac_string[i]==':')
		{
			j=0;
			continue;
		}
		else if((mac_string[i]>='A')&&(mac_string[i]<='F'))
			pMacEntry->octet[k]+=(mac_string[i]-'A'+10);
		else if((mac_string[i]>='a')&&(mac_string[i]<='f'))
			pMacEntry->octet[k]+=(mac_string[i]-'a'+10);
		else if((mac_string[i]>='0')&&(mac_string[i]<='9'))
			pMacEntry->octet[k]+=(mac_string[i]-'0');
		else
			IGMP_CTRL("str2mac MAC string parsing error!");
		if(j==0) pMacEntry->octet[k]<<=4;
		if(j==1) k++;
		j++;
	}
}


int32 _rtk_igmp_parseGetValue(char* stringBuff,void *parameterGetAddr,char* parameterName,enum rtk_igmp_parseDataType dataType)
{
	char* strPtr;
	uint32 stringSize=0;
	uint32 netv4ip;

	strPtr = stringBuff;

	while(1)
	{
		strPtr=strstr(strPtr,parameterName);
		if(strPtr==NULL) //string not find
		{
			//printk("stringBuff %s \n%s not find\n",stringBuff,parameterName);
			return FAILED;
		}
		//DEBUG("%s type=%d posDif=%d  sizeof:%d   [%d][%d]\n", parameterName,dataType, (int)(strPtr-stringBuff),(int)strlen(parameterName),((strPtr-stringBuff)!=0  && ((*(strPtr-1)!=' ') ||(*(strPtr-1)!=',') )  ),( (*(strPtr+strlen(parameterName)) != ' ') || (*(strPtr+strlen(parameterName)) != ',')));
		if(((strPtr-stringBuff)!=0  && ((*(strPtr-1)!=' ') && (*(strPtr-1)!=',') )  ) ||
			( (*(strPtr+strlen(parameterName)) != ' ') && (*(strPtr+strlen(parameterName)) != ',')))
		{
			//not this string maybe a sub-string goto find next
			strPtr+=strlen(parameterName);
			while((*strPtr==' ') || (*strPtr==',')) //ingore " " and ","
				strPtr++;
		}
		if( (*(strPtr+strlen(parameterName)) == ' ') || (*(strPtr+strlen(parameterName)) == ','))
			break;
	}
	strPtr+=strlen(parameterName);
	while((*strPtr==' ') || (*strPtr==',')) //ingore " " and ","
		strPtr++;


	if(*strPtr<0x20 ||  *strPtr==0x7f)//out of ascii char range
		return FAILED;

//	printk("%s type=%d posDif=%d  sizeof:%d\n",parameterName,dataType,strPtr-stringBuff,strlen(parameterName));

	switch(dataType)
	{
		case MAC:
			_rtk_igmp_str2mac(strPtr,(rtk_mac_t*)(parameterGetAddr));
			break;
		case V4IP:
			//*((uint32 *)parameterGetAddr) = (uint32)in_aton(strPtr);
			in4_pton(strPtr,-1,(uint8*)&netv4ip,-1,NULL);
			*(uint32*)parameterGetAddr= ntohl(netv4ip);
			break;
		case V6IP:
			in6_pton(strPtr,-1,(uint8*)parameterGetAddr,-1,NULL);
			break;
		case UNSIGNED_LONG:
			*((uint32*)parameterGetAddr)= simple_strtoul(strPtr,NULL,0);
			break;
		case SINGED_LONG:
			*((int32*)parameterGetAddr)= simple_strtol(strPtr,NULL,0);
			break;
		case STRING_EN_DIS:
			if(memcmp(strPtr,"enable",sizeof("enable")-1)==0){
				*((uint32*)parameterGetAddr)=1;
			}else if(memcmp(strPtr,"disable",sizeof("disable")-1)==0){
				*((uint32*)parameterGetAddr)=0;
			}else
				return FAILED;
			break;
		case STRING:
			while(1){
				if(*(strPtr+stringSize)<=0x20 ||  *(strPtr+stringSize)==0x7f)//out of ascii char range (include space)
					break;
				stringSize++;
			}
			memcpy(parameterGetAddr,strPtr,stringSize);
			break;
		default:
			return FAILED;

	}

	return SUCCESS;
}


int rtk_igmp_igmphook_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo IGMPHOOK enable > proc/igmp/ctrl/igmphook\n");
	PROC_PRINTF("echo IGMPHOOK disable > proc/igmp/ctrl/igmphook\n");
	PROC_PRINTF("status:%s\n",igmpSysdb.disHook==0?"enable\n":"disable");
	return SUCCESS;

}
int rtk_igmp_igmphook_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	uint32 value;
	if (buffer)
	{
		if(_rtk_igmp_parseGetValue(buffer,&value,"IGMPHOOK",STRING_EN_DIS) ==SUCCESS)
		{
			if(value)//enable ==1 disable==0
				igmpSysdb.disHook=0;
			else
				igmpSysdb.disHook=1;
		}
	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_igmphook_proc_read(NULL,NULL);

	return count;

}

int rtk_igmp_igmphwctrl_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo HWCTRL enable > proc/igmp/ctrl/hwControl\n");
	PROC_PRINTF("echo HWCTRL disable > proc/igmp/ctrl/hwControl\n");
	PROC_PRINTF("status:%s\n",igmpSysdb.disHwctrl==0?"enable":"disable");
	return SUCCESS;

}
int rtk_igmp_igmphwctrl_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{

	uint32 value;

	if (buffer)
	{
		if(_rtk_igmp_parseGetValue(buffer,&value,"HWCTRL",STRING_EN_DIS) ==SUCCESS)
		{
			if(value)//enable ==1 disable==0
			{
				if(igmpSysdb.disHwctrl!=0)
				{
					igmpSysdb.disHwctrl=0;
					rtk_igmp_hwCbReg();
				}
			}
			else
			{
				if(igmpSysdb.disHwctrl!=1)
				{
					igmpSysdb.disHwctrl=1;
					rtk_igmp_hwCbUnReg();
				}
			}
		}
	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_igmphwctrl_proc_read(NULL,NULL);

	return count;

}

int rtk_igmp_igmphwcbMode_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo HWCTRL_MODE mode > proc/igmp/ctrl/hwCbMode\n");
	PROC_PRINTF("mode [0:RTK_IGMP_CONFIG_BY_IFIDX] [1:RTK_IGMP_CONFIG_BY_PORT] [2:RTK_IGMP_CONFIG_BY_PORTMASK]\n");
	if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX)
		PROC_PRINTF("status:RTK_IGMP_CONFIG_BY_IFIDX\n");
	else if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORT)
		PROC_PRINTF("status:RTK_IGMP_CONFIG_BY_IFIDX\n");
	else if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORTMASK)
		PROC_PRINTF("status:RTK_IGMP_CONFIG_BY_PORTMASK\n");
	return SUCCESS;

}
int rtk_igmp_igmphwcbMode_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	uint32 value;

	if (buffer)
	{
		if(_rtk_igmp_parseGetValue(buffer,&value,"HWCTRL_MODE",UNSIGNED_LONG) ==SUCCESS)
		{
			if( value < RTK_IGMP_CONFIG_MAX)
			{
#if 1
				igmpSysdb.hwCbMode = value;
#else
				if(igmpSysdb.hwCbMode!=RTK_IGMP_CONFIG_BY_PORTMASK)
					printk("NOT SUPPORT PLEASE ENABLE CONFIG_RTK_FC_ABSTR_SW!\n");
				else
					igmpSysdb.hwCbMode = value;
#endif
			}
		}
	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_igmphwcbMode_proc_read(NULL,NULL);

	return count;

}


int rtk_igmp_procInternalCtrl_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support Patten: [PROC_CONF_SHOW] [REG_MDB_CB][MC2UC_COPYBY_IGMP]\n");

	PROC_PRINTF("procConfigShow status:%d %s\n",igmpSysdb.procConfigShow,igmpSysdb.procConfigShow==0?"(disable)DO NOT PRINT AFTER CONFIG":"(enable)PRINT AFTER CONFIG");
	PROC_PRINTF("REG_MDB_CB:%d\n",igmpSysdb.regMdbCb);
	PROC_PRINTF("MC2UC_COPYBY_IGMP:%d\n",igmpSysdb.mc2ucCopyByIGMP);
	return SUCCESS;

}
int rtk_igmp_procInternalCtrl_proc_write(struct file *file, char *buffer, unsigned long count, void *data)
{
	int32 value;

	if (buffer)
	{
		if(_rtk_igmp_parseGetValue(buffer,&value,"PROC_CONF_SHOW",SINGED_LONG) ==SUCCESS)
		{
			if(value)
				igmpSysdb.procConfigShow=1;
			else
				igmpSysdb.procConfigShow=0;
		}


		if(_rtk_igmp_parseGetValue(buffer,&value,"REG_MDB_CB",SINGED_LONG) ==SUCCESS)
		{
			if(value)
				igmpSysdb.regMdbCb=1;
			else
				igmpSysdb.regMdbCb=0;

			//apply config
			if(igmpSysdb.disHwctrl==0)
				rtk_igmp_hwCbReg();
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"MC2UC_COPYBY_IGMP",SINGED_LONG) ==SUCCESS)
		{
			if(value)
				igmpSysdb.mc2ucCopyByIGMP=1;
			else
				igmpSysdb.mc2ucCopyByIGMP=0;
		}

		
	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_procInternalCtrl_proc_read(NULL,NULL);

	return count;

}


int _rtk_igmp_proc_pasring_string_to_integer( char *buff,unsigned long len)
{
	int ret;

	ret=simple_strtol(buff, NULL, 0);

	return ret;
}

int rtk_igmp_unknownFlood_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Enable unknown multicast data flooding:\n");
	PROC_PRINTF("echo 1 > proc/igmp/ctrl/unkownFlood\n");
	PROC_PRINTF("Disable unknown multicast data flooding:\n");
	PROC_PRINTF("echo 0 > proc/igmp/ctrl/unkownFlood\n");
	PROC_PRINTF("status:%s\n",igmpSysdb.unknownFlood==0?"disable":"enable");
	return SUCCESS;

}

extern rtk_igmp_hwCbEvt_t hwCb;

int rtk_igmp_unknownFlood_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	igmpSysdb.unknownFlood = _rtk_igmp_proc_pasring_string_to_integer(buffer,count);

	if(igmpSysdb.unknownFlood)
	{
		if(hwCb.hwUnknownIpMcAction)
		{
			rtk_igmp_unknownMcCbEvt_t unknownCtrl;
			unknownCtrl.isIpv6=0;
			unknownCtrl.unknownAct=UNKNOWN_MC_TRAP;
			hwCb.hwUnknownIpMcAction(&unknownCtrl);
			unknownCtrl.isIpv6=1;
			hwCb.hwUnknownIpMcAction(&unknownCtrl);			
		}

	}
	else
	{
		//restore default action
		if(rtk_igmp_anyDevDisSnooping(0))
		{
			if(hwCb.hwUnknownIpMcAction)
			{
				rtk_igmp_unknownMcCbEvt_t unknownCtrl;
				unknownCtrl.isIpv6=0;
				unknownCtrl.unknownAct=UNKNOWN_MC_TRAP;
				hwCb.hwUnknownIpMcAction(&unknownCtrl);
			}
		}
		else
		{
			if(hwCb.hwUnknownIpMcAction)
			{
				rtk_igmp_unknownMcCbEvt_t unknownCtrl;
				unknownCtrl.isIpv6=0;
				unknownCtrl.unknownAct=UNKNOWN_MC_DROP;
				hwCb.hwUnknownIpMcAction(&unknownCtrl);
			}
		}
		if(rtk_igmp_anyDevDisSnooping(1))
		{
			if(hwCb.hwUnknownIpMcAction)
			{
				rtk_igmp_unknownMcCbEvt_t unknownCtrl;
				unknownCtrl.isIpv6=1;
				unknownCtrl.unknownAct=UNKNOWN_MC_TRAP;
				hwCb.hwUnknownIpMcAction(&unknownCtrl);
			}
		}
		else
		{
			if(hwCb.hwUnknownIpMcAction)
			{
				rtk_igmp_unknownMcCbEvt_t unknownCtrl;
				unknownCtrl.isIpv6=1;
				unknownCtrl.unknownAct=UNKNOWN_MC_DROP;
				hwCb.hwUnknownIpMcAction(&unknownCtrl);
			}
		}		
	}

	
	rtk_igmp_unknownFlood_proc_read(NULL, NULL);

	return count;

}

int rtk_igmp_trapWlanToPS_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Used to realize that packets of user group address forwarded to wireless devices will be sent to the protocol stack.\n");
	PROC_PRINTF("And multicast packets forwarded to wired devices will still be forwarded by hardware\n");
	PROC_PRINTF("Please refer to /proc/fc/ctrl/user_group for user group address.\n");
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Enable trap user group address multicast to PS:\n");
	PROC_PRINTF("echo 1 > proc/igmp/ctrl/trapWlanToPS\n");
	PROC_PRINTF("Disable trap user group address multicast to PS:\n");
	PROC_PRINTF("echo 0 > proc/igmp/ctrl/trapWlanToPS\n");
	PROC_PRINTF("status:%s\n",igmpSysdb.trapWlanToPS==0?"disable":"enable");
	return SUCCESS;

}

int rtk_igmp_trapWlanToPS_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	igmpSysdb.trapWlanToPS = _rtk_igmp_proc_pasring_string_to_integer(buffer,count);;
	rtk_igmp_trapWlanToPS_proc_read(NULL, NULL);

	return count;

}

#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
int rtk_igmp_skbMarkExtraSwitchPortId_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("MRRK2_EXTPORT_EN:%d MRRK2_EXTPORTID_START:%d MRRK2_EXTPORTID_LEN:%d\n",igmpSysdb.skbMark2_ExtPortEnStartbit,igmpSysdb.skbMark2_ExtPortIDStartbit,igmpSysdb.skbMark2_ExtPortIDLen);
	PROC_PRINTF("echo MRRK2_EXTPORT_EN [startbit] MRRK2_EXTPORTID_START [startbit] MRRK2_EXTPORTID_LEN [portid bitLen] > proc/igmp/ctrl/skbMarkExtraSwitchPortId\n");

	return SUCCESS;
}

int rtk_igmp_skbMarkExtraSwitchPortId_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	uint32 enStartbit=0,portStartbit=0,portLen=0;


	if (buffer)
	{
		
		if(_rtk_igmp_parseGetValue(buffer,&enStartbit,"MRRK2_EXTPORT_EN",UNSIGNED_LONG) ==FAILED)
		{
			printk("MRRK2_EXTPORT_EN parser error\n");
		}
		if(_rtk_igmp_parseGetValue(buffer,&portStartbit,"MRRK2_EXTPORTID_START",UNSIGNED_LONG) ==FAILED)
		{
			printk("MRRK2_EXTPORTID_START parser error\n");
		}
		if(_rtk_igmp_parseGetValue(buffer,&portLen,"MRRK2_EXTPORTID_LEN",UNSIGNED_LONG) ==FAILED)
		{
			printk("MRRK2_EXTPORTID_LEN parser error\n");
		}
	}
	igmpSysdb.skbMark2_ExtPortEnStartbit		= enStartbit;
	igmpSysdb.skbMark2_ExtPortIDStartbit 		= portStartbit;
	igmpSysdb.skbMark2_ExtPortIDLen				= portLen;

	return count;
}

#endif

int rtk_igmp_whiteList_proc_read(struct seq_file *s, void *v)
{

	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support ADD Patten: [DEVIFID/DEVIFNAME] [ISIP6] [SMAC] [SMACMSK(SMAC option)] \n");
	PROC_PRINTF("					 [GIP4_START/GIP4_END] [GIP6_START/GIP6_END]\n");
	PROC_PRINTF("					 [SIP4_START/SIP4_END] [SIP6_START/SIP6_END]\n");
	PROC_PRINTF("					 [MCSIP4_START/MCSIP4_END] [MCSIP6_START/MCSIP6_END]\n");
	PROC_PRINTF("					 [MCSIP_IGNORE_NON_V3]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2.0 GIP4_START 224.1.2.3 GIP4_END 224.1.2.3 SIP4_START 192.168.1.7  SIP4_END 192.168.1.7 > proc/igmp/ctrl/whiteList\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2.0 ISIP6 1 SMAC 00:8a:23:45:67:89 > proc/igmp/ctrl/whiteList\n");
	PROC_PRINTF("Support DEL Patten: [DELINDEX] [DELBY_PATTEN]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2.0 DELINDEX 0 > proc/igmp/ctrl/whiteList\n");
	PROC_PRINTF("echo DELBY_PATTEN 1 DEVIFNAME eth0.2.0 GIP4_START 224.2.3.4 GIP4_END 224.2.3.4 > proc/igmp/ctrl/whiteList\n");
	PROC_PRINTF("echo DELBY_PATTEN 1 DEVIFNAME eth0.2.0 ISIP6 1 SMAC 00:8a:23:45:67:89 > proc/igmp/ctrl/whiteList\n");
	PROC_PRINTF("DUMP:\n");
	PROC_PRINTF("echo DUMP 1 > proc/igmp/ctrl/whiteList\n");
	rtk_igmp_dump_devConfig(FAIL,NULL,NULL);
	return SUCCESS;
}

int rtk_igmp_whiteList_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	rt_igmpHook_whiteList_t patten;
	int index;
	int isipv6=FAIL;
	uint32 dump=0;
	uint32 v4ip=0;
	uint32 v6ip[4]={0};
	uint8  macaddr[6]={0};
	int32 devifid=FAIL;
	int32 whiteListIdx;
	char ifname[32]={0};
	int32 mcSipIgnoreNonV3;


	if (buffer)
	{

		if(_rtk_igmp_parseGetValue(buffer,&dump,"DUMP",UNSIGNED_LONG) ==SUCCESS)
		{
			show_igmp_whiteList_all(NULL,NULL);
			return count;
		}


		if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
		{
			if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
			{
				printk("DEVIFNAME parser error\n");
				goto DEV_ERROR;
			}
			else
			{
				devifid = rtk_igmp_devNameToDevIfidx(ifname);
			}
			if(devifid == FAIL)
			{
				printk("DEVIFNAME devifid FAIL\n");
				goto DEV_ERROR;
			}
		}

		if(_rtk_igmp_parseGetValue(buffer,&whiteListIdx,"DELINDEX",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_whiteListDel(NULL,devifid,whiteListIdx);
			if(igmpSysdb.procConfigShow)
				rtk_igmp_dump_devConfig(devifid,NULL,NULL);
			return count;
		}


		memset(&patten,0,sizeof(patten));


		if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4_START",V4IP) ==SUCCESS)
		{
			isipv6=0;
			patten.gipChk=1;
			patten.wlGroupIpStart[0]=v4ip;
			patten.wlGroupIpEnd[0]=v4ip;
			if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4_END",V4IP) ==SUCCESS)
			{
				patten.wlGroupIpEnd[0]=v4ip;
			}
		}

		if(_rtk_igmp_parseGetValue(buffer,v6ip,"GIP6_START",V6IP) ==SUCCESS)
		{
			isipv6=1;
			patten.gipChk=1;
			memcpy(patten.wlGroupIpStart,v6ip,sizeof(patten.wlGroupIpStart));
			memcpy(patten.wlGroupIpEnd,v6ip,sizeof(patten.wlGroupIpEnd));
			if(_rtk_igmp_parseGetValue(buffer,v6ip,"GIP6_END",V6IP) ==SUCCESS)
			{
				memcpy(patten.wlGroupIpEnd,v6ip,sizeof(patten.wlGroupIpEnd));
			}
		}


		if(_rtk_igmp_parseGetValue(buffer,&v4ip,"SIP4_START",V4IP) ==SUCCESS)
		{
			isipv6=0;
			patten.sipChk=1;
			patten.wlSipStart[0]=v4ip;
			patten.wlSipEnd[0]=v4ip;
			if(_rtk_igmp_parseGetValue(buffer,&v4ip,"SIP4_END",V4IP) ==SUCCESS)
			{
				patten.wlSipEnd[0]=v4ip;
			}
		}


		if(_rtk_igmp_parseGetValue(buffer,v6ip,"SIP6_START",V6IP) ==SUCCESS)
		{
			isipv6=1;
			patten.sipChk=1;
			memcpy(patten.wlSipStart,v6ip,sizeof(patten.wlSipStart));
			memcpy(patten.wlSipEnd,v6ip,sizeof(patten.wlSipEnd));
			if(_rtk_igmp_parseGetValue(buffer,v6ip,"SIP6_END",V6IP) ==SUCCESS)
			{
				memcpy(patten.wlSipEnd,v6ip,sizeof(patten.wlSipEnd));
			}
		}

		if(_rtk_igmp_parseGetValue(buffer,macaddr,"SMAC",MAC) ==SUCCESS)
		{
			patten.smacChk=1;
			memcpy(patten.smac,macaddr,sizeof(patten.smac));
			memset(patten.smacMask,0xff,sizeof(patten.smacMask));
			if(_rtk_igmp_parseGetValue(buffer,macaddr,"SMACMSK",MAC) ==SUCCESS)
			{
				memcpy(patten.smacMask,macaddr,sizeof(patten.smacMask));
			}
		}



		if(_rtk_igmp_parseGetValue(buffer,&v4ip,"MCSIP4_START",V4IP) ==SUCCESS)
		{
			isipv6=0;
			patten.mcSipChk=1;
			patten.wlMcSip[0]=v4ip;
			patten.wlMcSipEnd[0]=v4ip;
			if(_rtk_igmp_parseGetValue(buffer,&v4ip,"MCSIP4_END",V4IP) ==SUCCESS)
			{
				patten.wlMcSipEnd[0]=v4ip;
			}
		}

		if(_rtk_igmp_parseGetValue(buffer,v6ip,"MCSIP6_START",V6IP) ==SUCCESS)
		{
			isipv6=1;
			patten.mcSipChk=1;
			memcpy(patten.wlMcSip,v6ip,sizeof(patten.wlMcSip));
			memcpy(patten.wlMcSipEnd,v6ip,sizeof(patten.wlMcSipEnd));
			if(_rtk_igmp_parseGetValue(buffer,v6ip,"MCSIP6_END",V6IP) ==SUCCESS)
			{
				memcpy(patten.wlMcSipEnd,v6ip,sizeof(patten.wlMcSipEnd));
			}
			
		}

		if(_rtk_igmp_parseGetValue(buffer,&mcSipIgnoreNonV3,"MCSIP_IGNORE_NON_V3",UNSIGNED_LONG) ==SUCCESS)
		{
			patten.mcSipChkIgnoreNonIGMPv3MLDv2=1;
		}

		if(isipv6==FAIL)
		{
			if(_rtk_igmp_parseGetValue(buffer,&isipv6,"ISIP6",UNSIGNED_LONG) ==SUCCESS)
			{
				if(isipv6)
					patten.isIpv6=1;
			}
			else
				printk("ISIP6 parser error\n");
		}
		else
			patten.isIpv6=isipv6;


		if(_rtk_igmp_parseGetValue(buffer,&whiteListIdx,"DELBY_PATTEN",UNSIGNED_LONG) ==SUCCESS)
		{
			int ret;
			ret=rtk_igmp_whiteListDel(&patten,devifid , 0);
			if(ret ==FAIL)
				printk("Error can't find the entry \n");
			else
			{
				if(igmpSysdb.procConfigShow)
					rtk_igmp_dump_devConfig(devifid,NULL,NULL);
			}
			return count;
		}

		if(rtk_igmp_whiteListAdd( &patten,devifid , &index)!=SUCCESS)
			printk("Error\n");
		else
			printk("whiteList Add Success at %d\n",index);


	}

	
	if(igmpSysdb.procConfigShow)
		rtk_igmp_dump_devConfig(devifid,NULL,NULL);

DEV_ERROR:

	return count;

}




int rtk_igmp_blackList_proc_read(struct seq_file *s, void *v)
{

	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support ADD Patten: [DEVIFID/DEVIFNAME] [SMAC] [SMACMSK(SMAC option)] \n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2 ISIP6 1 SMAC 00:8a:23:45:67:89 > proc/igmp/ctrl/blackList\n");
	PROC_PRINTF("Support DEL Patten: [DELINDEX] [DELBY_PATTEN]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2 DELINDEX 0 > proc/igmp/ctrl/blackList\n");
	PROC_PRINTF("echo DELBY_PATTEN 1 DEVIFNAME eth0.2 ISIP6 1 SMAC 00:8a:23:45:67:89 > proc/igmp/ctrl/blackList\n");
	PROC_PRINTF("DUMP:\n");
	PROC_PRINTF("echo DUMP 1 > proc/igmp/ctrl/blackList\n");	
	rtk_igmp_dump_devConfig(FAIL,NULL,NULL);
	return SUCCESS;
}


int rtk_igmp_blackList_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	rt_igmpHook_blackList_t patten;
	int blackListIdx;
	uint8  macaddr[6]={0};
	int32 devifid=FAIL;
	char ifname[32]={0};
	int32 isipv6=FAIL;
	uint32 dump=0;

	if (buffer)
	{

		if(_rtk_igmp_parseGetValue(buffer,&dump,"DUMP",UNSIGNED_LONG) ==SUCCESS)
		{
			show_igmp_blackList_all(NULL,NULL);
			return count;
		}


		if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
		{
			if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
			{
				printk("DEVIFNAME parser error\n");
				goto DEV_ERROR;
			}
			else
			{
				devifid = rtk_igmp_devNameToDevIfidx(ifname);
			}
			if(devifid == FAIL)
			{
				printk("DEVIFNAME devifid FAIL\n");
				goto DEV_ERROR;
			}
		}

		if(_rtk_igmp_parseGetValue(buffer,&blackListIdx,"DELINDEX",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_blackListDel(NULL,devifid,blackListIdx);
			if(igmpSysdb.procConfigShow)
				rtk_igmp_dump_devConfig(devifid,NULL,NULL);
			return count;
		}


		memset(&patten,0,sizeof(patten));


		if(_rtk_igmp_parseGetValue(buffer,macaddr,"SMAC",MAC) ==SUCCESS)
		{
			patten.smacChk=1;
			memcpy(patten.smac,macaddr,sizeof(patten.smac));
			memset(patten.smacMask,0xff,sizeof(patten.smacMask));
			if(_rtk_igmp_parseGetValue(buffer,macaddr,"SMACMSK",MAC) ==SUCCESS)
			{
				memcpy(patten.smacMask,macaddr,sizeof(patten.smacMask));
			}
		}


		if(_rtk_igmp_parseGetValue(buffer,&isipv6,"ISIP6",UNSIGNED_LONG) ==SUCCESS)
		{
			if(isipv6)
				patten.isIpv6=1;
		}
		else
		{
			printk("ISIP6 parser error using ISIP6=0\n");
		}


		if(_rtk_igmp_parseGetValue(buffer,&blackListIdx,"DELBY_PATTEN",UNSIGNED_LONG) ==SUCCESS)
		{
			int ret;
			ret=rtk_igmp_blackListDel(&patten,devifid , 0);
			if(ret ==FAIL)
				printk("Error can't find the entry \n");
			else
			{
				if(igmpSysdb.procConfigShow)
					rtk_igmp_dump_devConfig(devifid,NULL,NULL);
			}
			return count;
		}



		if(rtk_igmp_blackListAdd( &patten,devifid , &blackListIdx)!=SUCCESS)
			printk("\n blackListAdd Error\n");
		else
		{
			if(igmpSysdb.procConfigShow)
				printk("\nblackList Add Success at %d\n",blackListIdx);
		}

	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_dump_devConfig(devifid,NULL,NULL);

DEV_ERROR:

	return count;

}




int rtk_igmp_ignoreGroup_proc_read(struct seq_file *s, void *v)
{
	rtk_igmp_groupToPsDump(s,v);
	PROC_PRINTF("=================================================\n");
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support ADD Patten:[GIP4/GIP6] [GIPMSK4/GIPMSK6(GIP option)] [GIP4_START/GIP6_START] [GIP4_END/GIP6_END][COPY2CPU]  \n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo  GIP4 224.2.3.4 > proc/igmp/ctrl/ignoreGroup\n");
	PROC_PRINTF("Support DEL Patten: [DELINDEX] [DELBY_PATTEN]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DELINDEX 0 > proc/igmp/ctrl/ignoreGroup\n");
	PROC_PRINTF("echo DELBY_PATTEN 1 GIP4 224.2.3.4 > proc/igmp/ctrl/ignoreGroup\n");
	return SUCCESS;
}

int rtk_igmp_ignoreGroup_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	rt_igmpHook_ignoreGroup_t patten;
	int index;
	uint32 v4ip=0;
	uint32 v6ip[4]={0};

	if (buffer)
	{
		if(_rtk_igmp_parseGetValue(buffer,&index,"DELINDEX",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_groupToPsTblDel(NULL,index);
			return count;
		}

		memset(&patten,0,sizeof(patten));

		if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4",V4IP) ==SUCCESS)
		{
			patten.ignGroupIpStart[0]=v4ip;
			patten.ignGroupIpEnd[0]=patten.ignGroupIpStart[0];
		}
		
		if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4_START",V4IP) ==SUCCESS)
		{
			patten.ignGroupIpStart[0]=v4ip;
			patten.ignGroupIpEnd[0]=patten.ignGroupIpStart[0];
			if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4_END",V4IP) ==SUCCESS)
			{
				patten.ignGroupIpEnd[0]=v4ip;
			}

		}
		if(_rtk_igmp_parseGetValue(buffer,v6ip,"COPY2CPU",UNSIGNED_LONG) ==SUCCESS)
		{
			patten.isCopy2cpu=1;
		}


		if(_rtk_igmp_parseGetValue(buffer,v6ip,"GIP6",V6IP) ==SUCCESS)
		{
			memcpy(patten.ignGroupIpStart,v6ip,sizeof(patten.ignGroupIpStart));
			memcpy(patten.ignGroupIpEnd,v6ip,sizeof(patten.ignGroupIpEnd));
			patten.isIpv6=1;
		}
		if(_rtk_igmp_parseGetValue(buffer,&v6ip,"GIP6_START",V6IP) ==SUCCESS)
		{
			memcpy(patten.ignGroupIpStart,v6ip,sizeof(patten.ignGroupIpStart));
			memcpy(patten.ignGroupIpEnd,v6ip,sizeof(patten.ignGroupIpEnd));
			patten.isIpv6=1;
			if(_rtk_igmp_parseGetValue(buffer,&v6ip,"GIP6_END",V6IP) ==SUCCESS)
			{
				memcpy(patten.ignGroupIpEnd,v6ip,sizeof(patten.ignGroupIpEnd));
			}
		}


		if(_rtk_igmp_parseGetValue(buffer,&index,"DELBY_PATTEN",UNSIGNED_LONG) ==SUCCESS)
		{
			int ret;
			ret=rtk_igmp_groupToPsTblDel(&patten,-1);
			if(ret ==FAIL)
				printk("Error can't find the entry \n");
			else
			{
				//rtk_igmp_dump_devConfig(devifid,NULL,NULL);
			}
			return count;
		}


		if(rtk_igmp_groupToPsTblAdd( &patten, &index)!=SUCCESS)
			printk("Error\n");
		else
			printk("groupToPs Add Success at %d\n",index);


	}

	return count;

}



int rtk_igmp_groupWeight_proc_read(struct seq_file *s, void *v)
{
	rtk_igmp_groupWeightDump(s,v);
	PROC_PRINTF("=================================================\n");
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support ADD Patten:[GIP4/GIP6] [GIPMSK4/GIPMSK6(GIP option)] [GIP4_START/GIP6_START] [GIP4_END/GIP6_END][WEIGHT]  \n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo  DEVIFNAME eth0.2.0 GIP4 224.2.3.4 WEIGHT 10 > proc/igmp/ctrl/groupWeight\n");
	PROC_PRINTF("Support DEL Patten: [DELINDEX] [DELBY_PATTEN]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DELINDEX 0 > proc/igmp/ctrl/ignoreGroup\n");
	PROC_PRINTF("echo DELBY_PATTEN 1 GIP4 224.2.3.4 > proc/igmp/ctrl/groupWeight\n");
	return SUCCESS;
}

int rtk_igmp_groupWeight_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	rt_igmpHook_groupWeight_t patten;
	int index;
	uint32 v4ip=0;
	int32 weight=FAIL;
	uint32 v6ip[4]={0};
	rt_igmpHook_devInfo_t devInfo={{0}};
	int32 devifid=FAIL;
	char ifname[32]={0};

	if (buffer)
	{
		if(_rtk_igmp_parseGetValue(buffer,&index,"DELINDEX",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_groupWeightTblDel(devInfo,NULL,index);
			return count;
		}

		memset(&patten,0,sizeof(patten));

		if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
		{
			if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
			{
				printk("DEVIFNAME parser error\n");
				return count;
			}
			else
			{
				memcpy(devInfo.devIfname,ifname,sizeof(devInfo.devIfname));
			}
		
		}
		else
			devInfo.devIfidx=devifid;



		if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4",V4IP) ==SUCCESS)
		{
			patten.weightGroupIpStart[0]=v4ip;
			patten.weightGroupIpEnd[0]=patten.weightGroupIpStart[0];
		}
		
		if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4_START",V4IP) ==SUCCESS)
		{
			patten.weightGroupIpStart[0]=v4ip;
			patten.weightGroupIpEnd[0]=patten.weightGroupIpStart[0];
			if(_rtk_igmp_parseGetValue(buffer,&v4ip,"GIP4_END",V4IP) ==SUCCESS)
			{
				patten.weightGroupIpEnd[0]=v4ip;
			}

		}


		if(_rtk_igmp_parseGetValue(buffer,v6ip,"GIP6",V6IP) ==SUCCESS)
		{
			memcpy(patten.weightGroupIpStart,v6ip,sizeof(patten.weightGroupIpStart));
			memcpy(patten.weightGroupIpEnd,v6ip,sizeof(patten.weightGroupIpEnd));
			patten.isIpv6=1;
		}
		if(_rtk_igmp_parseGetValue(buffer,&v6ip,"GIP6_START",V6IP) ==SUCCESS)
		{
			memcpy(patten.weightGroupIpStart,v6ip,sizeof(patten.weightGroupIpStart));
			memcpy(patten.weightGroupIpEnd,v6ip,sizeof(patten.weightGroupIpEnd));
			patten.isIpv6=1;
			if(_rtk_igmp_parseGetValue(buffer,&v6ip,"GIP6_END",V6IP) ==SUCCESS)
			{
				memcpy(patten.weightGroupIpEnd,v6ip,sizeof(patten.weightGroupIpEnd));
			}
		}

		if(_rtk_igmp_parseGetValue(buffer,&weight,"WEIGHT",SINGED_LONG) ==SUCCESS)
		{
			patten.weight=weight;
		}




		if(_rtk_igmp_parseGetValue(buffer,&index,"DELBY_PATTEN",UNSIGNED_LONG) ==SUCCESS)
		{
			int ret;
			ret=rtk_igmp_groupWeightTblDel(devInfo,&patten,-1);
			if(ret ==FAIL)
				printk("Error can't find the entry \n");
			else
			{
				//rtk_igmp_dump_devConfig(devifid,NULL,NULL);
			}
			return count;
		}


		if(weight==FAIL || rtk_igmp_groupWeightTblAdd( devInfo,&patten, &index)!=SUCCESS)
			printk("Error\n");
		else
			printk("groupWeight Add Success at %d\n",index);


	}


	return count;

}





int rtk_igmp_br_proc_read(struct seq_file *s, void *v)
{

	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support Patten: [DEVIFID/DEVIFNAME] [BR_MAX_GROUP_SIZE] [BR_MAX_IP4_GROUP_SIZE] [BR_MAX_IP6_GROUP_SIZE] [BR_MAX_CLIENT_SIZE] [BR_MAX_IP4_CLIENT_SIZE]\n");
	PROC_PRINTF("                [BR_MAX_IP6_CLIENT_SIZE] [BR_PER_IP4_CLIENT_MAX_JOIN_GROUP] [BR_PER_IP6_CLIENT_MAX_JOIN_GROUP]\n");
	PROC_PRINTF("                [DROP_IGMPV1] [DROP_IGMPV2] [DROP_IGMPV3] [DROP_MLDV1] [DROP_MLDV2] [SNOOPING_DISABLE] [RATE_LIMIT] [FAST_LEAVE]\n");
	PROC_PRINTF("                [BR_FLUSH_CONF] [BR_FLUSH_MDB] [BR_FLUSH_MDBv4] [BR_FLUSH_MDBv6] \n");
	PROC_PRINTF("                [MAX_IP4_WEIGHT] [MAX_IP6_WEIGHT] [DROP_IP4_OVER_WEIGHT] [DROP_IP6_OVER_WEIGHT][COMBINE_ID]\n");
	PROC_PRINTF("Example:\n");

	PROC_PRINTF("echo DEVIFNAME br0 BR_MAX_GROUP_SIZE 1 > proc/igmp/ctrl/brConfig\n");

	return SUCCESS;
}

int rtk_igmp_br_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	uint32 value;
	int32 devifid=FAIL;
	char ifname[32]={0};

	if (buffer)
	{
		if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
		{
			if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
			{
				printk("DEVIFNAME parser error\n");
				goto DEV_ERROR;
			}
			else
			{
				devifid = rtk_igmp_devNameToBrDevIfidx(ifname);
			}
			if(devifid == FAIL)
			{
				printk("DEVIFNAME devifid FAIL\n");
				goto DEV_ERROR;
			}
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_MAX_GROUP_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_GROUP_SIZE,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_MAX_IP4_GROUP_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_IP4_GROUP_SIZE,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_MAX_IP6_GROUP_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_IP6_GROUP_SIZE,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_MAX_CLIENT_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_CLIENT_SIZE,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_MAX_IP4_CLIENT_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_IP4_CLIENT_SIZE,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_MAX_IP6_CLIENT_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_IP6_CLIENT_SIZE,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_PER_IP4_CLIENT_MAX_JOIN_GROUP",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_PER_IP4_CLIENT_MAX_JOIN_GROUP,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_PER_IP6_CLIENT_MAX_JOIN_GROUP",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_PER_IP6_CLIENT_MAX_JOIN_GROUP,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IGMPV1",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_DROP_IGMPV1,value);
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IGMPV2",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_DROP_IGMPV2,value);
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IGMPV3",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_DROP_IGMPV3,value);
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_MLDV1",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_DROP_MLDV1,value);
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_MLDV2",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_DROP_MLDV2,value);
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"SNOOPING_DISABLE",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_SNOOPING_DISABLE,value);
		}
		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_FLUSH_CONF",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_FLUSH_CONF,value);
		}
		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_FLUSH_MDB",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_FLUSH_MDB,value);
		}
		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_FLUSH_MDBv4",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_FLUSH_MDBv4,value);
		}
		if(_rtk_igmp_parseGetValue(buffer,&value,"BR_FLUSH_MDBv6",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_FLUSH_MDBv6,value);
		}
		if(_rtk_igmp_parseGetValue(buffer,&value,"RATE_LIMIT",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_RATE_LIMIT,value);
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"FAST_LEAVE",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_FAST_LEAVE,value);
		}

		if(_rtk_igmp_parseGetValue(buffer,&value,"MAX_IP4_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_IP4_WEIGHT,value);
		}		

		if(_rtk_igmp_parseGetValue(buffer,&value,"MAX_IP6_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_MAX_IP6_WEIGHT,value);
		}	

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IP4_OVER_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_DROP_IP4_OVER_WEIGHT,value);
		}	
		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IP6_OVER_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_DROP_IP6_OVER_WEIGHT,value);
		}	
		if(_rtk_igmp_parseGetValue(buffer,&value,"COMBINE_ID",UNSIGNED_LONG) ==SUCCESS)
		{
			rtk_igmp_BrDevConfig_set(devifid,RT_BR_COMBINE_ID,value);
		}	

	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_dump_devConfig(devifid,NULL,NULL);
DEV_ERROR:

	return count;

}


int rtk_igmp6_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	uint32 value;
	int32 devifid=FAIL;
	char ifname[32]={0};

	if (buffer)
	{


		if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
		{
			if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
			{
				printk("DEVIFNAME parser error\n");
				goto DEV_ERROR;
			}
			else
			{
				devifid = rtk_igmp_devNameToDevIfidx(ifname);
			}
			if(devifid == FAIL)
			{
				printk("DEVIFNAME devifid FAIL\n");
				goto DEV_ERROR;
			}
		}


		if(_rtk_igmp_parseGetValue(buffer,&value,"SNOOPING_DISABLE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_SNOOPING_DISABLE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_MLDV1",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_DROP_MLDV1 ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_MLDV2",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_DROP_MLDV2 ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"IGNORE_MLDV1",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_IGNORE_MLDV1 ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"IGNORE_MLDV2",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_IGNORE_MLDV2 ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"REPORT_DROP",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_REPORT_DROP ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"FAST_LEAVE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_FAST_LEAVE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"CARE_SOURCE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_CARE_SOURCE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"MAX_GROUP_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_MAX_GROUP_SIZE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"MC_MEMBER_AGING_INTERVAL",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_MC_MEMBER_AGING_INTERVAL ,value);
		
		if(_rtk_igmp_parseGetValue(buffer,&value,"MC_LAST_MEMBER_AGING_INTERVAL",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_MC_LAST_MEMBER_AGING_INTERVAL ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "RATE_LIMIT", UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1,RT_RATE_LIMIT, value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "FLUSH_CONF",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1, RT_FLUSH_CONF, value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "MAX_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1, RT_MAX_WEIGHT, value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "DROP_OVER_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1, RT_DROP_OVER_WEIGHT, value);		

		if(_rtk_igmp_parseGetValue(buffer,&value, "FORWARD_DISABLE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1, RT_FORWARD_DISABLE, value);		

		if(_rtk_igmp_parseGetValue(buffer,&value, "DMAC_MC_TO_UC",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 1, RT_DMAC_MC_TO_UC, value);		

	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_dump_devConfig(devifid,NULL,NULL);

DEV_ERROR:
	return count;
}

int rtk_igmp6_proc_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support Patten: [DEVIFID/DEVIFNAME] [SNOOPING_DISABLE] [DROP_MLDV1] [DROP_MLDV2] [IGNORE_MLDV1] [IGNORE_MLDV2]\n");
	PROC_PRINTF("                [REPORT_DROP] [FAST_LEAVE] [CARE_SOURCE] [DISABLE_CTAG_AUTO_LEARNING] [MAX_GROUP_SIZE] [MC_MEMBER_AGING_INTERVAL] [RATE_LIMIT] [FLUSH_CONF]\n");
	PROC_PRINTF("                [MAX_WEIGHT] [DROP_OVER_WEIGHT] [MC_LAST_MEMBER_AGING_INTERVAL] [FORWARD_DISABLE][DMAC_MC_TO_UC]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2 SNOOPING_DISABLE 1 > proc/igmp/ctrl/igmp6Config\n");

	return SUCCESS;
}

int rtk_igmp_proc_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	uint32 value;
	int32 devifid=FAIL;
	char ifname[32]={0};

	if (buffer)
	{

		if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
		{
			if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
			{
				printk("DEVIFNAME parser error\n");
				goto DEV_ERROR;
			}
			else
			{
				devifid = rtk_igmp_devNameToDevIfidx(ifname);
			}
			if(devifid == FAIL)
			{
				printk("DEVIFNAME devifid FAIL\n");
				goto DEV_ERROR;
			}
		}


		if(_rtk_igmp_parseGetValue(buffer,&value,"SNOOPING_DISABLE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_SNOOPING_DISABLE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IGMPV1",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_DROP_IGMPV1 ,value);
		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IGMPV2",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_DROP_IGMPV2 ,value);
		if(_rtk_igmp_parseGetValue(buffer,&value,"DROP_IGMPV3",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_DROP_IGMPV3 ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"IGNORE_IGMPV1",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_IGNORE_IGMPV1 ,value);
		if(_rtk_igmp_parseGetValue(buffer,&value,"IGNORE_IGMPV2",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_IGNORE_IGMPV2 ,value);
		if(_rtk_igmp_parseGetValue(buffer,&value,"IGNORE_IGMPV3",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_IGNORE_IGMPV3 ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"REPORT_DROP",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_REPORT_DROP ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"FAST_LEAVE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_FAST_LEAVE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"CARE_SOURCE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_CARE_SOURCE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"MAX_GROUP_SIZE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_MAX_GROUP_SIZE ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"MC_MEMBER_AGING_INTERVAL",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_MC_MEMBER_AGING_INTERVAL ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value,"MC_LAST_MEMBER_AGING_INTERVAL",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0,RT_MC_LAST_MEMBER_AGING_INTERVAL ,value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "RATE_LIMIT",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0, RT_RATE_LIMIT, value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "FLUSH_CONF",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0, RT_FLUSH_CONF, value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "MAX_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0, RT_MAX_WEIGHT, value);

		if(_rtk_igmp_parseGetValue(buffer,&value, "DROP_OVER_WEIGHT",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0, RT_DROP_OVER_WEIGHT, value);		

		if(_rtk_igmp_parseGetValue(buffer,&value, "FORWARD_DISABLE",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0, RT_FORWARD_DISABLE, value);		

		if(_rtk_igmp_parseGetValue(buffer,&value, "DMAC_MC_TO_UC",UNSIGNED_LONG) ==SUCCESS)
			rtk_igmp_devConfig_set(devifid, 0, RT_DMAC_MC_TO_UC, value);		
		

	}
	if(igmpSysdb.procConfigShow)
		rtk_igmp_dump_devConfig(devifid,NULL,NULL);

DEV_ERROR:
	return count;

}

int rtk_igmp_proc_read(struct seq_file *s, void *v)
{

	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support Patten: [DEVIFID/DEVIFNAME] [SNOOPING_DISABLE] [DROP_IGMPV1] [DROP_IGMPV2] [DROP_IGMPV3] [IGNORE_IGMPV1] [IGNORE_IGMPV2] [IGNORE_IGMPV3]\n");
	PROC_PRINTF("                [REPORT_DROP] [FAST_LEAVE] [CARE_SOURCE] [DISABLE_CTAG_AUTO_LEARNING] [MAX_GROUP_SIZE] [MC_MEMBER_AGING_INTERVAL] [RATE_LIMIT] [FLUSH_CONF]\n");
	PROC_PRINTF("                [MAX_WEIGHT] [DROP_OVER_WEIGHT] [MC_LAST_MEMBER_AGING_INTERVAL][FORWARD_DISABLE][DMAC_MC_TO_UC]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2 SNOOPING_DISABLE 1 > proc/igmp/ctrl/igmpConfig\n");


	return SUCCESS;
}


int rtk_igmp_nextGroupStatistic_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support Patten: [DEVIFID/DEVIFNAME]  [ISIP6]\n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2 ISIP6 1 > proc/igmp/dump/nextGroupStatistic\n");

	return SUCCESS;

}


int rtk_igmp_nextGroupStatistic_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	struct seq_file *s=NULL;

	uint32 isipv6=0;
	int32 getIdx=0;
	int32 devifid=FAIL;
	char ifname[32]={0};
	rt_igmpHook_devInfo_t devInfo={{0}};
	rt_igmpHook_groupStatistic_t statisticInfo;

	if(!buffer)
		return count;

	if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
	{
		if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
		{
			printk("DEVIFNAME parser error\n");
			return count;
		}
		else
		{
			memcpy(devInfo.devIfname,ifname,sizeof(devInfo.devIfname));
		}

	}
	else
		devInfo.devIfidx=devifid;


	if(_rtk_igmp_parseGetValue(buffer,&isipv6,"ISIP6",SINGED_LONG) ==SUCCESS)
		isipv6=1;
	else
		isipv6=0;
		

	while(1)
	{
		int i=0;
		if(rtk_igmp_nextValidGroupStatistic_get(devInfo,&getIdx,isipv6,&statisticInfo)==SUCCESS &&  getIdx>=0)
		{
			if(isipv6)
			{
				PROC_PRINTF("[%d]Group=%pI6 SrcAddr=%pI6 vlan=%d svlan=%d curClient=%pI6(ExistTimesSec:%u)\n",i,statisticInfo.groupAddr,statisticInfo.sourceAddr,statisticInfo.vlanId,statisticInfo.svlanId,statisticInfo.curClient,statisticInfo.curClientExistTimesSec);
			}
			else
			{
				PROC_PRINTF("[%d]Group=%pI4h SrcAddr=%pI4h vlan=%d svlan=%d curClient=%pI4h(ExistTimesSec:%u)\n",i,statisticInfo.groupAddr,statisticInfo.sourceAddr,statisticInfo.vlanId,statisticInfo.svlanId,statisticInfo.curClient,statisticInfo.curClientExistTimesSec);

			}
			getIdx++;
			i++;
		}
		else
			break;
	}
	

	return count;

}

int rtk_igmp_devStatistic_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("Support Patten: [DEVIFID/DEVIFNAME][ISIP6] \n");
	PROC_PRINTF("Example:\n");
	PROC_PRINTF("echo DEVIFNAME eth0.2 ISIP6 1 > proc/igmp/dump/devStatistic\n");

	return SUCCESS;
}


int rtk_igmp_devStatistic_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	struct seq_file *s=NULL;
	uint32 isipv6=0;
	int32 devifid=FAIL;
	char ifname[32]={0};
	rt_igmpHook_devInfo_t devInfo={{0}};
	rt_igmpHook_devStatistic_t statisticInfo;

	if(!buffer)
		return count;
	/* copy data to the buffer */


	if(_rtk_igmp_parseGetValue(buffer,&devifid,"DEVIFID",UNSIGNED_LONG) ==FAILED)
	{
		if(_rtk_igmp_parseGetValue(buffer,ifname,"DEVIFNAME",STRING) ==FAILED)
		{
			printk("DEVIFNAME parser error\n");
			return count;
		}
		else
		{
			memcpy(devInfo.devIfname,ifname,sizeof(devInfo.devIfname));
		}

	}
	else
		devInfo.devIfidx=devifid;


	if(_rtk_igmp_parseGetValue(buffer,&isipv6,"ISIP6",SINGED_LONG) !=SUCCESS)
	{
		isipv6=0;
	}

	rtk_igmp_devStatistic_get(devInfo,isipv6,&statisticInfo);
	
	PROC_PRINTF("activeGroupCnt=%d bandwidthExceedCnt=%d curWeight=%d joinCnt=%d\n",statisticInfo.activeGroupCnt,statisticInfo.bandwidthExceedCnt,statisticInfo.curWeight,statisticInfo.joinCnt);


	return count;

}


int rtk_igmp_debug_read(struct seq_file *s, void *v)
{
	PROC_PRINTF("DEBUG_LEVEL:%03x\n",igmpSysdb.igmp_debug_level);
	PROC_PRINTF("[WARNING]:          :0x001   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_WARNING?"ON":"");
	PROC_PRINTF("[HOOK_POINT]:       :0x002   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_HOOK_POINT?"ON":"");
	PROC_PRINTF("[PARSER]:           :0x004   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_PARSER?"ON":"");	
	PROC_PRINTF("[IGMPCTRL]:         :0x008   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_IGMPCTRL?"ON":"");
	PROC_PRINTF("[FLOWDATA]:         :0x010   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_FLOWDATA?"ON":"");
	PROC_PRINTF("[HWCB]:             :0x020   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_HWCB?"ON":"");
	PROC_PRINTF("[DUMP]:             :0x040   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_DUMP?"ON":"");
	PROC_PRINTF("[RATE_LIMIT_DROP]:  :0x080   %s \n",igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_RATE_LIMIT_DROP?"ON":"");
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
{
	extern atomic_t wqAllocCnt;
	extern atomic_t wqFreeCnt;
	PROC_PRINTF("wqAllocCnt =%d  wqFreeCnt=%d \n",atomic_read(&wqAllocCnt),atomic_read(&wqFreeCnt));
}
#endif	
	return SUCCESS;
}

int rtk_igmp_debug_write(struct file *file,  char *buffer, unsigned long count, void *data)
{

	if (buffer)
	{
		/* copy data to the buffer */
		igmpSysdb.igmp_debug_level=simple_strtoul(buffer, NULL, 16);
		if(igmpSysdb.procConfigShow)
			rtk_igmp_debug_read(NULL,NULL);
		return count;
	}
	return -EFAULT;

}


int rtk_igmp_mrouteMapping_read(struct seq_file *s, void *v)
{

	PROC_PRINTF("Usage:\n");
	PROC_PRINTF("echo UPIFIDX [UP_IF_IDX] DOWNIFIDX [DOWN_IF_IDX] > proc/igmp/ctrl/mrouteMapping\n");
	PROC_PRINTF("echo UPIFNAME [UP_IF_NAME] DOWNIFNAME [DOWN_IF_NAME] > proc/igmp/ctrl/mrouteMapping\n");

	PROC_PRINTF("\n\n");
	rtk_igmp_allDump(s,v);

	return SUCCESS;
}

int rtk_igmp_mrouteMapping_write(struct file *file,  char *buffer, unsigned long count, void *data)
{
	int32 upifidx=FAIL,downifidx=FAIL;
	uint32 value;
	char upIfName[16]={0};
	char downIfName[16]={0};
	int ifNameGet=FAIL;

	if (buffer)
	{

		if(_rtk_igmp_parseGetValue(buffer,&value,"DOWNIFIDX",UNSIGNED_LONG) ==SUCCESS)
			upifidx = value;
		else
			upifidx=FAIL;

		if(_rtk_igmp_parseGetValue(buffer,&value,"UPIFIDX",UNSIGNED_LONG) ==SUCCESS)
			downifidx = value;
		else
			downifidx = FAIL;

		if((_rtk_igmp_parseGetValue(buffer,upIfName,"UPIFNAME",STRING) ==SUCCESS) && (_rtk_igmp_parseGetValue(buffer,downIfName,"DOWNIFNAME",STRING) ==SUCCESS))
			ifNameGet=SUCCESS;
	}

	if(upifidx!=FAIL && downifidx!=FAIL)
	{
		rtk_igmp_wanIdxBindToBrIdx(upifidx,downifidx);
		if(igmpSysdb.procConfigShow)
			rtk_igmp_allDump(NULL,NULL);
	}
	else if(ifNameGet==SUCCESS)
	{
		rtk_igmp_wanIfNameBindToBrName(upIfName,downIfName);
		if(igmpSysdb.procConfigShow)
			rtk_igmp_allDump(NULL,NULL);
	}
	else
		printk("parameter error\n");

	return count;
}


int rtk_igmp_proc_init(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#define proc_open proc_fops.proc_open
#define proc_write proc_fops.proc_write
#define proc_read proc_fops.proc_read
#define proc_lseek proc_fops.proc_lseek
#define proc_release proc_fops.proc_release
#else
#define proc_open proc_fops.open
#define proc_write proc_fops.write
#define proc_read proc_fops.read
#define proc_lseek proc_fops.llseek
#define proc_release proc_fops.release
#endif
	int i = 0;
	struct proc_dir_entry *p=NULL;

	/* create proc directories  */
	if(rtk_igmp_proc_dir==NULL){
		rtk_igmp_proc_dir = proc_mkdir("igmp", NULL);
	}
	if(rtk_igmp_dump_proc_dir==NULL){
		rtk_igmp_dump_proc_dir = proc_mkdir("dump", rtk_igmp_proc_dir);
	}
	if(rtk_igmp_ctrl_proc_dir==NULL){
		rtk_igmp_ctrl_proc_dir = proc_mkdir("ctrl", rtk_igmp_proc_dir);
	}

	for( i=0; i< (sizeof(igmpProc)/sizeof(rtk_igmp_proc_t)) ;i++)
	{
		struct proc_dir_entry *tmpDir=NULL;

		if(igmpProc[i].get==NULL)
			igmpProc[i].proc_open=rtk_igmp_nullDebugSingleOpen;
		else
			igmpProc[i].proc_open=rtk_igmp_commonDebugSingleOpen;

		if(igmpProc[i].set==NULL)
			igmpProc[i].proc_write=NULL;
		else
			igmpProc[i].proc_write=rtk_igmp_commonDebugSingleWrite;

		igmpProc[i].proc_read=seq_read;
		igmpProc[i].proc_lseek=seq_lseek;
		igmpProc[i].proc_release=single_release;

		switch(igmpProc[i].dir)
		{
			case PROC_DIR_IGMP_DUMP:
				tmpDir=rtk_igmp_dump_proc_dir;
				break;
			case PROC_DIR_IGMP_CTRL:
				tmpDir=rtk_igmp_ctrl_proc_dir;
				break;
			default:
				break;
		}


		p = proc_create_data(igmpProc[i].name, S_IRUGO, tmpDir, &(igmpProc[i].proc_fops),NULL);
		if(!p){
			printk("create proc %s failed!\n",igmpProc[i].name);
		}
		else
		{
			igmpProc[i].inode_id = p->low_ino;
		}
	}

	return SUCCESS;

}



void rtk_igmp_proc_exit(void)
{
	int i = 0;

	for( i=0; i< (sizeof(igmpProc)/sizeof(rtk_igmp_proc_t)) ;i++)
	{
		struct proc_dir_entry *tmpDir=NULL;

		switch(igmpProc[i].dir)
		{
			case PROC_DIR_IGMP_DUMP:
				tmpDir=rtk_igmp_dump_proc_dir;
				break;
			case PROC_DIR_IGMP_CTRL:
				tmpDir=rtk_igmp_ctrl_proc_dir;
				break;
			default:
				break;
		}

		remove_proc_entry(igmpProc[i].name, tmpDir);
	}

	proc_remove(rtk_igmp_dump_proc_dir);
	proc_remove(rtk_igmp_ctrl_proc_dir);
	proc_remove(rtk_igmp_proc_dir);
}


