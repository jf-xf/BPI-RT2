#ifndef __RT_EDP_DEBUG_H__
#define __RT_EDP_DEBUG_H__

#include <linux/kernel.h>
#include <rt_edp_struct.h>

char *proc_printf(struct seq_file *s, char *fmt, ...);
#define PROC_PRINTF( comment ,arg...)	do { proc_printf(s,comment,##arg); }while(0)

void _rt_edp_common_dump(rt_edp_debug_level_t level,const char *funcs,int line, char *comment,...);


#ifndef WARNING
#define WARNING( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_WARN) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_WARN,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP
#define IGMP( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_IGMP) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_IGMP,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef DEBUG
#define DEBUG( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_DEBUG) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_DEBUG,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef CMD
#define CMD( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_CMD) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_CMD,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ACL_EDP
#define ACL_EDP( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_ACL) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_ACL,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ACL_CTRL_EDP
#define ACL_CTRL_EDP( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_ACL_CONTROL_PATH) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_ACL_CONTROL_PATH,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TRACE_PREROUTING
#define TRACE_PREROUTING( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TRACE_POSTROUTING
#define TRACE_POSTROUTING( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TRACE_FORWARD
#define TRACE_FORWARD( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_FORWARD) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_TRACE_FORWARD,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TRACE_LOCALIN
#define TRACE_LOCALIN( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TRACE_LOCALOUT
#define TRACE_LOCALOUT( comment ,arg...)	do { if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT) _rt_edp_common_dump(RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif



#ifndef TRACE_X
#define TRACE_X( X,comment ,arg...)	do { if(rt_edp_kernel.debug_level&X) _rt_edp_common_dump(X,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif



void rt_edp_memDump (void *start, u32 size, char * strHeader);

#endif
