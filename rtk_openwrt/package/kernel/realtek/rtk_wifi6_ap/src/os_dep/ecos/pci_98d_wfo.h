#ifndef _PCI_98D_WFO_H_
#define _PCI_98D_WFO_H_

#include <linux/pci.h>

#define PCI_VENDOR_ID_REALTEK	0x10EC

extern int ecos_interrupt_in, ecos_interrupt_out;
extern void ecos_dsr_raise_softirq(cyg_vector_t vector,
		cyg_ucount32 count, cyg_addrword_t data);

int rtk_wfo_pcie_init(void);
void rtk_wfo_get_mac(u8 *mac, u8 idx);
void *rtk_wfo_get_net_dev(u8 idx);
void rtk_wfo_get_irq(u32 *irq);
void rtk_wfo_poll_irq(void);

#endif /* _PCI_98D_WFO_H_ */
