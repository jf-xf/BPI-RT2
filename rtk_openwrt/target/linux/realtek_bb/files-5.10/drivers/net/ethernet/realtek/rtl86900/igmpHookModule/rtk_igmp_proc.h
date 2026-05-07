
#ifndef _RTK_IGMP_PROC_H
#define _RTK_IGMP_PROC_H


#include <linux/seq_file.h>
#include <linux/proc_fs.h>


typedef enum rtk_igmp_procDir_e
{
	PROC_DIR_IGMP_DUMP,
	PROC_DIR_IGMP_CTRL,
	PROC_DIR_MAX //this field must put at last.
} rtk_igmp_procDir_t;



typedef struct rtk_igmp_proc_s
{
	char *name;
	int (*get) (struct seq_file *s, void *v);
	int (*set) ( struct file *, char *,unsigned long, void *);
	unsigned int inode_id;
	unsigned char unlockBefortWrite;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	struct proc_ops proc_fops;
#else
	struct file_operations proc_fops;
#endif
	rtk_igmp_procDir_t dir;
}rtk_igmp_proc_t;

int rtk_igmp_proc_init(void);
void rtk_igmp_proc_exit(void);
//int32 _rtk_igmp_parseGetValue(char* stringBuff,void *parameterGetAddr,char* parameterName,enum rtk_rg_parseDataType dataType);


#endif

