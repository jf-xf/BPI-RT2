#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>

#include <rt_acl_ext.h>
//#include <rt_sec_ext.h>
#include <rt_misc_ext.h>
#include <rt_port.h>
#include <common/rt_error.h>

void add_test_acl_rule(void)
{
	int32 ret = RT_ERR_FAILED;
	int acl_filter_idx = 0;

	rt_acl_filterAndQos_t rule;

	/* Setting ACL rule sample */
	memset(&rule, 0, sizeof(rt_acl_filterAndQos_t));

	rule.filter_fields |= RT_ACL_INGRESS_L4_UDP_BIT;
	rule.filter_fields |= RT_ACL_INGRESS_PORT_MASK_BIT;
	rule.ingress_port_mask = 0x1f;
	rule.ingress_l4_protocal = 0x11; /* UDP protocal value = 17 */
	rule.ingress_src_l4_port_start = 0;
	rule.ingress_src_l4_port_end = 0xffff;
	rule.ingress_dest_l4_port_start = 67;
	rule.ingress_dest_l4_port_end = 67;
	rule.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT;

	ret = rt_acl_filterAndQos_add(rule, &acl_filter_idx);
	
	if (ret == RT_ERR_OK)
		printk("%s: add test acl entry[%d] success!\n", __func__, acl_filter_idx);
	else
		printk("%s: add test acl entry failed!\n", __func__);
}

void add_mac_learning_limit(void)
{
	int32 ret = RT_ERR_FAILED;
	unsigned int port = 0;

	rt_misc_macAddr_learning_limit_t mac_learning_limit;

	/* learning limit set to -1 means unlimit */
	mac_learning_limit.learningLimitNumber = 777;

	ret = rt_misc_macAddr_learning_limit_set(port, mac_learning_limit);
	if (ret == RT_ERR_OK) {
		printk("%s: set mac learning limit of port[%d] = %d success!\n", __func__,
				port, mac_learning_limit.learningLimitNumber);
	} else
		printk("%s: set mac learning limit of port[%d] failed!\n", __func__, port);
}

void add_mac_filter_by_acl(void)
{
	int32 ret = RT_ERR_FAILED;
	int acl_filter_idx = 0;

	rt_acl_filterAndQos_t rule;

	/* MAC filter sample */
	memset(&rule, 0, sizeof(rt_acl_filterAndQos_t));

	rule.filter_fields |= RT_ACL_INGRESS_DMAC_BIT;
	rule.filter_fields |= RT_ACL_INGRESS_PORT_MASK_BIT;

	rule.ingress_dmac_mask[0] = 0xff;
	rule.ingress_dmac_mask[1] = 0xff;
	rule.ingress_dmac_mask[2] = 0xff;
	rule.ingress_dmac_mask[3] = 0xff;
	rule.ingress_dmac_mask[4] = 0xff;
	rule.ingress_dmac_mask[5] = 0xff;
	rule.ingress_dmac[0] = 0xde;
	rule.ingress_dmac[1] = 0xad;
	rule.ingress_dmac[2] = 0xbe;
	rule.ingress_dmac[3] = 0xef;
	rule.ingress_dmac[4] = 0xfa;
	rule.ingress_dmac[5] = 0xce;
	rule.ingress_port_mask = 0x1f;

	rule.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT;

	ret = rt_acl_filterAndQos_add(rule, &acl_filter_idx);

	if (ret == RT_ERR_OK)
		printk("%s: add mac filter acl entry[%d] success!\n", __func__, acl_filter_idx);
	else
		printk("%s: add mac filter acl entry failed!\n", __func__);
}

void add_port_disable(void)
{
	int32 ret = RT_ERR_FAILED;
	unsigned int port = 0;

	rt_enable_t state = DISABLED;

	/* Port disable sample */
	ret = RT_ERR_FAILED;
	port = 0;

	ret = rt_port_adminEnable_set(port, state);
	if (ret == RT_ERR_OK)
		printk("%s: set port[%d] state to DISABLE\n", __func__, port);
	else
		printk("%s: set port[%d] state failed!\n", __func__, port);
}

static int __init rtk_acl_test_init(void)
{
	add_test_acl_rule();

	add_mac_filter_by_acl();

	add_mac_learning_limit();

	add_port_disable();

	return 0;
}

static void __exit rtk_acl_test_exit(void)
{
	printk(KERN_INFO "Goodbye\n" );
}


MODULE_DESCRIPTION( "Realtek RT API ACL Rule test !!" );
MODULE_LICENSE( "GPL" );
module_init(rtk_acl_test_init);
module_exit(rtk_acl_test_exit);
