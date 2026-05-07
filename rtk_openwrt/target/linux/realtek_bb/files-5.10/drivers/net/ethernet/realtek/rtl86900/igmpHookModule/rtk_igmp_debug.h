#ifndef _RTK_IGMP_CTRL_H
#define _RTK_IGMP_CTRL_H


#include <rtk_igmp_struct.h>
extern rtk_igmp_Sysdb_t  igmpSysdb;


char *proc_igmp_printf(struct seq_file *s, char *fmt, ...);
#define PROC_PRINTF( comment ,arg...)	do { proc_igmp_printf(s,comment,##arg); }while(0)
void igmp_common_dump(rtk_igmp_debug_level_t level,const char *funcs,int line, char *comment,...);
void igmp_dump_packet(u8 *pkt,u32 size,char *memo);


#ifndef IGMP_CTRL
#define IGMP_CTRL( comment ,arg...)	do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_IGMPCTRL) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_IGMPCTRL,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP_DATA
#define IGMP_DATA( comment ,arg...)	do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_FLOWDATA) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_FLOWDATA,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP_WARNING
#define IGMP_WARNING( comment ,arg...)	do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_WARNING) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_WARNING,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP_HWCB
#define IGMP_HWCB( comment ,arg...)	do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_HWCB) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_HWCB,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP_PARSER
#define IGMP_PARSER( comment ,arg...)	do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_PARSER) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_PARSER,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP_RATE_LIMIT_DROP
#define IGMP_RATE_LIMIT_DROP( comment ,arg...)    do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_RATE_LIMIT_DROP) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_RATE_LIMIT_DROP,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP_HOOK_POINT
#define IGMP_HOOK_POINT( comment ,arg...)	do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_HOOK_POINT) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_HOOK_POINT,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP_API
#define IGMP_API( comment ,arg...)	do {if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_API) igmp_common_dump(RTK_IGMP_DEBUG_LEVEL_API,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif


#endif

