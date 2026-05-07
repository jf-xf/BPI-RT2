/* Copyright, Cortina Access 2015
 *
 * drivers/serial/serial_cortina.c
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

/* Register definitions */
#if 0
#define UCFG                    (PER_UART0_CFG - PER_UART0_CFG) /* UART config register */
#define UFC                     (PER_UART0_FC - PER_UART0_CFG)  /* flow control */
#define URX_SAMPLE              (PER_UART0_RX_SAMPLE - PER_UART0_CFG)   /* UART RX Sample register */
#define URT_TUNE                (PER_UART0_TUN - PER_UART0_CFG) /* Fine tune of UART clk */
#define UTX_DATA                (PER_UART0_TXDAT - PER_UART0_CFG)       /* UART TX Character data */
#define URX_DATA                (PER_UART0_RXDAT - PER_UART0_CFG)       /* UART RX Character data */
#define UINFO                   (PER_UART0_INFO - PER_UART0_CFG)        /* UART Info */
#define UINT_EN                 (PER_UART0_IE_0 - PER_UART0_CFG)        /* UART Interrupt enable */
#define UINT_CLR                (PER_UART0_INT_0 - PER_UART0_CFG)       /* UART Interrupt setting/clearing */
#define UINT_STAT               (PER_UART0_STAT - PER_UART0_CFG)        /* UART Interrupt Status */
#else
#define UCFG                    0x00 	/* UART config register */
#define UFC                     0x04	/* flow control */
#define URX_SAMPLE              0x08 	/* UART RX Sample register */
#define URT_TUNE                0x0C 	/* Fine tune of UART clk */
#define UTX_DATA                0x10	/* UART TX Character data */
#define URX_DATA                0x14	/* UART RX Character data */
#define UINFO                   0x18	/* UART Info */
#define UINT_EN                 0x1C	/* UART Interrupt enable */
#define UINT_CLR                0x24	/* UART Interrupt setting/clearing */
#define UINT_STAT               0x2C	/* UART Interrupt Status */
#endif

/* UART Control Register Bit Fields.*/
#define UCFG_BAUD_COUNT                 (8)
#define UCFG_EN                         (1 << 7)
#define UCFG_RX_EN                      (1 << 6)
#define UCFG_TX_EN                      (1 << 5)
#define UCFG_PARITY_EN                  (1 << 4)
#define UCFG_PARITY_SEL                 (1 << 3)
#define UCFG_2STOP_BIT                  (1 << 2)
#define UCFG_CNT1                       (1 << 1)
#define UCFG_CNT0                       (1 << 0)
#define UCFG_CHAR_5                     0
#define UCFG_CHAR_6                     1
#define UCFG_CHAR_7                     2
#define UCFG_CHAR_8                     3

#define  UINFO_TX_FIFO_EMPTY            (1<<3)
#define  UINFO_TX_FIFO_FULL             (1 << 2)
#define  UINFO_RX_FIFO_EMPTY            (1<<1)
#define  UINFO_RX_FIFO_FULL             (1 << 0)

#define UINT_RX_NON_EMPTY               (1 << 6)
#define UINT_TX_EMPTY                   (1 << 5)
#define UINT_RX_UNDERRUN                (1 << 4)
#define UINT_RX_OVERRUN                 (1 << 3)
#define UINT_RX_PARITY_ERR              (1 << 2)
#define UINT_RX_STOP_ERR                (1 << 1)
#define UINT_TX_OVERRUN                 (1 << 0)
#define UINT_MASK_ALL                   0x7F

// UART CONF bits
#define SHF_UCONF_WL       0
#define MSK_UCONF_WL       (0x3 << SHF_UCONF_WL)
#define VAL_UCONF_WL_5     (0x0 << SHF_UCONF_WL)
#define VAL_UCONF_WL_6     (0x1 << SHF_UCONF_WL)
#define VAL_UCONF_WL_7     (0x2 << SHF_UCONF_WL)
#define VAL_UCONF_WL_8     (0x3 << SHF_UCONF_WL)

#define SHF_UCONF_SB       2
#define MSK_UCONF_SB       (0x1 << SHF_UCONF_SB)
#define VAL_UCONF_SB_1     (0x0 << SHF_UCONF_SB)
#define VAL_UCONF_SB_2     (0x1 << SHF_UCONF_SB)

#define SHF_UCONF_PM       3
#define MSK_UCONF_PM       (0x3 << SHF_UCONF_PM)
#define VAL_UCONF_PM_N     (0x0 << SHF_UCONF_PM)
#define VAL_UCONF_PM_O     (0x2 << SHF_UCONF_PM)
#define VAL_UCONF_PM_E     (0x3 << SHF_UCONF_PM)
