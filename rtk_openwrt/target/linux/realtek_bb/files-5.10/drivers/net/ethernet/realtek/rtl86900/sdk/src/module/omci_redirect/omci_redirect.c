#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/kfifo.h>

#include <linux/sched.h>
#include <linux/slab.h>
#include "../pkt_redirect/pkt_redirect.h"
#include "omci_redirect.h"

/*rt api*/
#include "../../../include/rtk/rt/rt_gpon.h"

#define RTK_GPON_OMCI_MSG_LEN   1980
#define DEBUG_LOGS              0

static void omci_send_to_user(rtk_gpon_pkt_t* omci)
{
    int ret;
    
    if((ret = pkt_redirect_kernelApp_sendPkt(PR_USER_UID_GPONOMCI,1, sizeof(rtk_gpon_pkt_t), (unsigned char *)omci))!=RT_ERR_OK)
    {
        printk(KERN_INFO "%s() ret:%d",__FUNCTION__,ret);
    }
    return;
}

static unsigned int omci_rx_callback(unsigned int msgLen,unsigned char *pMsg)
{
    rtk_gpon_pkt_t pkt;
    
    if (DEBUG_LOGS)
    {
        printk(KERN_INFO "======%s %d : OMCI RX  len %u ======",__FUNCTION__,__LINE__, msgLen);
        printk(KERN_INFO "%s %d : RX OMCI content",__FUNCTION__,__LINE__);
        print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 16, 1, pMsg, msgLen, true);
    }
    
    memset(&pkt,0,sizeof(rtk_gpon_pkt_t));
    pkt.type = RTK_GPON_MSG_OMCI;
    memcpy(&pkt.msg.omci.msg,pMsg,msgLen);
    pkt.msg.omci.len = msgLen;

    omci_send_to_user(&pkt);

    return 0;


}

static void omci_send_to_nic(unsigned short len,unsigned char *ptr) 
{

    rtk_gpon_omci_msg_t *pkt_ptr = (rtk_gpon_omci_msg_t *)ptr;
    uint8_t *omci_ptr = pkt_ptr->msg;
    uint32_t omci_len = pkt_ptr->len;
    
    if(len != sizeof(rtk_gpon_omci_msg_t))
    {
        printk(KERN_INFO "%s %d : tx wrong length Error!!",__FUNCTION__,__LINE__);
        return;
    }
    
    if (DEBUG_LOGS)
    {
        printk(KERN_INFO "======%s %d : TX OMCI len %u ======",__FUNCTION__,__LINE__,omci_len);
        printk(KERN_INFO "%s %d : TX OMCI content",__FUNCTION__,__LINE__);
        print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS, 16, 1, omci_ptr, omci_len, true);
    }

    if (RT_ERR_OK != rt_gpon_omci_tx(omci_len, omci_ptr))
    {
        printk(KERN_INFO "%s %d : TX OMCI Error!!",__FUNCTION__,__LINE__);
    }

    return;
}


static int omci_init(void)
{
    int32 ret = RT_ERR_OK;

    if((ret = pkt_redirect_kernelApp_reg(PR_KERNEL_UID_GPONOMCI,omci_send_to_nic))!=RT_ERR_OK)
    {
        printk(KERN_INFO "OMCI pkt_redirect_kernelApp_reg Error!!\n");
        return ret;
    }

    if((ret = rt_gpon_omci_rx_callback_register(&omci_rx_callback))!=RT_ERR_OK)
    {
        printk(KERN_INFO "OMCI reg rt gpon rx callback fail ret:%d !!\n", ret);
        return ret;
    }  

    return ret;
}


static int omci_exit(void)
{
    int32 ret = RT_ERR_OK;
    /*remove receive GPON OMCI callback for send packet to NIC*/
    if((ret = pkt_redirect_kernelApp_dereg(PR_KERNEL_UID_GPONOMCI))!=RT_ERR_OK)
    {
        return ret;
    }

    return ret;
}

int __init rtk_omci_redirect_init(void)
{

    if(RT_ERR_OK != omci_init())
        return RT_ERR_OK;

    return RT_ERR_OK;
}

void __exit rtk_omci_redirect_exit(void)
{
    omci_exit();
}


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek OMCI Redirect kernel module");
MODULE_AUTHOR("RealTek");

module_init(rtk_omci_redirect_init);
module_exit(rtk_omci_redirect_exit);
