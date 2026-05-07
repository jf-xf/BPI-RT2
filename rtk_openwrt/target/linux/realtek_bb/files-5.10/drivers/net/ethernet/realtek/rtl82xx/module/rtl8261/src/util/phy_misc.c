


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>

uint32 log_initMsgEn = TRUE;
uint32 log_initErrEn = TRUE;

/* Function Name:
 *      rt_log
 * Description:
 *      Public API for all layers logging
 * Input:
 *      level   - log level
 *      module  - module name
 *      format - pointer buffer of the logging information
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_ALLOWED - actions not allowed by the function
 * Note:
 *      None
 */
int32
rt_log(const int32 level, const int64 module, const char *format, ...)
{
    return RT_ERR_OK;
}

