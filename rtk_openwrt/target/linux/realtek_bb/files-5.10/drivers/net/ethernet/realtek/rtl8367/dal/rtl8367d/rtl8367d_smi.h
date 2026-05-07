
#ifndef __RTL8367D_SMI_H__
#define __RTL8367D_SMI_H__

#include <rtk_types.h>
#include "rtk_error.h"

rtksw_int32 rtl8367d_smi_read(rtksw_uint32 mAddrs, rtksw_uint32 *rData);
rtksw_int32 rtl8367d_smi_write(rtksw_uint32 mAddrs, rtksw_uint32 rData);

#endif /* __RTL8367D_SMI_H__ */


