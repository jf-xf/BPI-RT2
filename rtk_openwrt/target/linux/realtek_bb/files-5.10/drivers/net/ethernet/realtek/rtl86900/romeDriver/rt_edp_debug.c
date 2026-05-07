#include <rt_edp_debug.h>

char rt_edp_str_buf[512];



void rt_edp_memDump (void *start, u32 size, char * strHeader)
{
	int row, column, index, index2, max;
//	uint32 buffer[5];
	u8 *buf, *line, ascii[17];
	char empty = '.';

	if(!start ||(size==0))
		return;
	line = (u8*)start;

	/*
	16 bytes per line
	*/
	if (strHeader)
		printk("%s", strHeader);
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16)
	{
		buf = line;

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */

		printk("\n%08x ", (u32) line);

		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			printk("  ");
			printk("%02x ", (u8) buf[index2]);
			ascii[index2] = (((u8) buf[index2] < 32) || ((u8) buf[index2] >= 128)) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				printk("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printk("   ");
		}

		//ASCII
		printk("  %s", ascii);
	}
	printk("\n");
	return;
}

void _rt_edp_common_dump(rt_edp_debug_level_t level,const char *funcs,int line, char *comment,...)
{
	int show=1;
	char string[16]={0};
	int color, bgcolor;
	va_list a_list;
	va_start(a_list,comment);

/*
	if(rt_edp_db.filter_level&level)
	{
		//show=_rtk_rg_trace_filter_compare(rg_db.pktHdr->skb,rg_db.pktHdr);
		show=rt_edp_db.tracefilterShow;
	}
*/
	if(show==1)
	{
		int mt_trace_i;

		vsprintf( rt_edp_str_buf, comment,a_list);
		for(mt_trace_i=1;mt_trace_i<512;mt_trace_i++)
		{
			if(rt_edp_str_buf[mt_trace_i]==0)
			{
				if(rt_edp_str_buf[mt_trace_i-1]=='\n') rt_edp_str_buf[mt_trace_i-1]=' ';
				else break;
			}
		}

		switch(level)
		{

			case RT_EDP_DEBUG_LEVEL_WARN:
				sprintf(string,"WARN"); color=33; bgcolor=41;
				break;
			case RT_EDP_DEBUG_LEVEL_IGMP:
				sprintf(string,"IGMP"); color=31; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_DEBUG:
				sprintf(string,"DEBUG"); color=33; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_CMD:
				sprintf(string,"CMD"); color=36; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_ACL:
				sprintf(string,"ACL"); color=36; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_ACL_CONTROL_PATH:
				sprintf(string,"ACL_CTRL"); color=36; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING:
				sprintf(string,"PREROUTING"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_FORWARD:
				sprintf(string,"FORWARD"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING:
				sprintf(string,"POSTROUTING"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN:
				sprintf(string,"LOCAL_IN"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT:
				sprintf(string,"LOCAL_OUT"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING_DUMP:
				sprintf(string,"PREROUTING_DUMP"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_FORWARD_DUMP:
				sprintf(string,"FORWARD_DUMP"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING_DUMP:
				sprintf(string,"POSTROUTING_DUMP"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN_DUMP:
				sprintf(string,"LOCAL_IN_DUMP"); color=35; bgcolor=40;
				break;
			case RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT_DUMP:
				sprintf(string,"LOCAL_OUT_DUMP");color=35; bgcolor=40;
				break;

			default:
				sprintf(string,"XXX");color=37; bgcolor=40;
				break;

		}
		printk("\033[1;%d;%dm[%s] %s\033[1;30;40m @ %s(%d)\033[0m\n",color,bgcolor,string,rt_edp_str_buf,funcs,line);
	}

	va_end (a_list);
}

