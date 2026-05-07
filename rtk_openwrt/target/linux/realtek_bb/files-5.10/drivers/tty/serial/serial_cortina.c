/* Copyright, Cortina Access 2015
 * drivers/serial/serial_cortina.c
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/sysrq.h>
#include <linux/console.h>
#include <linux/serial_core.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/io.h>
#ifdef	CONFIG_CONTROL_SERIAL_INPUT
#include <linux/proc_fs.h>
#endif
#ifdef CONFIG_OF
#include "serial_cortina.h"
#else /* pre DT, legacy platform support */
#include <mach/platform.h>
#include <mach/hardware.h>
#endif /*  CONFIG_OF */

#include <asm/io.h>

#ifdef CONFIG_OF /* DT support */
#include <linux/of.h>
#include <linux/of_device.h>

#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#endif /* DT support */

#if defined(CONFIG_KGDB_SERIAL_CONSOLE) || \
    defined(CONFIG_KGDB_SERIAL_CONSOLE_MODULE)
#include <linux/kgdb.h>
#include <asm/irq_regs.h>
#endif

#ifndef CONFIG_OF
#include <mach/cs_clk_change.h>
#endif /* CONFIG_OF */

#ifdef CONFIG_OF
/* forward declr */
static struct of_device_id cortina_uart_of_match[];
#endif /* CONFIG_OF */

/***************************************
 *	UART Related registers
 ****************************************/
/* register definitions */
#define	 CFG		UCFG
#define	 FC		UFC
#define	 RX_SAMPLE	URX_SAMPLE
#define	 TX_DAT		UTX_DATA
#define	 RX_DAT		URX_DATA
#define	 INFO		UINFO
#define	 IE		UINT_EN
#define	 INT	 	UINT_CLR
#define	 STATUS		UINT_STAT

/* CFG */
#define	 CFG_STOP_2BIT   (1<<2)
#define	 CFG_PARITY_EVEN (1<<3)
#define	 CFG_PARITY_EN   (1<<4)
#define	 CFG_TX_EN		(1<<5)
#define	 CFG_RX_EN   	(1<<6)
#define	 CFG_UART_EN 	(1<<7)
#define	 CFG_BAUD_SART   8

/* INFO */
#define		INFO_TX_EMPTY   (1<<3)
#define	 INFO_TX_FULL		(1<<2)
#define	 INFO_RX_EMPTY   	(1<<1)
#define	 INFO_RX_FULL		(1<<0)

/* Interrupt */
#define RX_BREAK			(1<<7)
#define RX_FIFO_NONEMPTYE	(1<<6)
#define TX_FIFO_EMPTYE		(1<<5)
#define RX_FIFO_UNDERRUNE	(1<<4)
#define RX_FIFO_OVERRUNE	(1<<3)
#define RX_PARITY_ERRE		(1<<2)
#define RX_STOP_ERRE		(1<<1)
#define TX_FIFO_OVERRUNE	(1<<0)

#define CONFIG_SERIAL_CS_CORTINA_CONSOLE

#ifdef CONFIG_CORTINA_ENGINEERING
#define UART_NR 4
#else
#define UART_NR 2
#endif

struct cortina_uart_port {
	struct uart_port	uart;
	char			name[16];
	char	break_indicator;
	unsigned int may_wakeup;
};
#ifdef	CONFIG_CONTROL_SERIAL_INPUT
static int ca_uart_input = 1; /* 0: disable input, 1: enable input */
#endif
static struct cortina_uart_port *cortina_uart_ports[UART_NR]={0};

static bool sysrq = true; /* sysrq workaround */

/* Forward decl.s */
static irqreturn_t cortina_uart_interrupt(int irq, void *dev_id);

/* uart_ops functions */
static unsigned int cortina_uart_tx_empty(struct uart_port *port)
{
	/* Return 0 on FIXO condition, TIOCSER_TEMT otherwise */

	return (readl(port->membase + INFO) & 0x8) ?  TIOCSER_TEMT : 0;

}

static void cortina_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	/* This is your basic 3-wire serial port. None of these signals exist. */
}

static unsigned int cortina_uart_get_mctrl(struct uart_port *port)
{
	/* Claim unimplemented signals asserted, as per Documentation/serial/driver */
	return TIOCM_CTS | TIOCM_DSR | TIOCM_CD;
}

static void cortina_uart_stop_tx(struct uart_port *port)
{
	/* Turn off Tx interrupts. The port lock is held at this point */
	unsigned int temp;
	temp = readl(port->membase + IE);
	writel(temp & ~CFG_TX_EN, port->membase + IE);
}

static inline void cortina_transmit_buffer(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;

	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		cortina_uart_stop_tx(port);
		return;
	}

	while (!(readl(port->membase + INFO) & INFO_TX_FULL)) {
		/* send xmit->buf[xmit->tail]
		 * out the port here */
		writel(xmit->buf[xmit->tail], port->membase + TX_DAT);
		xmit->tail = (xmit->tail + 1) &
				 (UART_XMIT_SIZE - 1);
		port->icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	}

	if (uart_circ_empty(xmit))
		cortina_uart_stop_tx(port);
}
static void cortina_uart_start_tx(struct uart_port *port)
{
	/* Turn on Tx interrupts. The port lock is held at this point */
	unsigned long temp;

	temp = readl(port->membase + IE);
	writel((temp | CFG_TX_EN), port->membase + IE);

	if (readl(port->membase + INFO) & INFO_TX_EMPTY)
		cortina_transmit_buffer(port);
}

static void cortina_uart_stop_rx(struct uart_port *port)
{
	/* Turn off Rx interrupts. The port lock is held at this point */
	unsigned long temp;

	temp = readl(port->membase + IE);
	writel(temp & ~CFG_RX_EN, port->membase + IE);
}
#ifdef CONFIG_CORTINA_PM_CALLBACK
static void cortina_uart_start_rx(struct uart_port *port)
{
	/* Turn on Rx interrupts. The port lock is held at this point */
	unsigned long temp;

	temp = readl(port->membase + IE);
	writel(temp | CFG_RX_EN, port->membase + IE);
}
#endif /* CONFIG_CORTINA_PM_CALLBACK */

static void cortina_uart_enable_ms(struct uart_port *port)
{
	/* Nope, you really can't hope to attach a modem to this */
}
/* FIXME no such utility found in cortina hardware */
static void cortina_uart_break_ctl(struct uart_port *port, int ctl)
{
	/* N.A */
}

static u8 ca_uart_irq_cpu = 0xff;
static  int __init ca_uart_irq_cpu_setup(char *val)
{
	return kstrtou8(val, 10, &ca_uart_irq_cpu);
}

__setup("ca_uart_cpu=", ca_uart_irq_cpu_setup);

static int cortina_uart_startup(struct uart_port *port)
{
	unsigned long temp;
	int retval;
	unsigned long flags;
#ifndef CONFIG_OF
	struct platform_clk clk;
#endif /* CONFIG_OF */

	temp = readl(port->membase + IE);
	writel(temp & 0, port->membase + IE);

	retval = request_irq(port->irq, cortina_uart_interrupt, 0, "cortina_uart", port);
	if (retval)
		return retval;

	if((ca_uart_irq_cpu < num_possible_cpus()) && cpu_online(ca_uart_irq_cpu)){
			if(irq_set_affinity(port->irq, cpumask_of(ca_uart_irq_cpu)) == 0){
				printk("UART: IRQ%d bond to CPU%d\n", port->irq, ca_uart_irq_cpu);
			}
	}

	/* The serial core uses this as a cookie, so we should set it even though
		* it doesn't mean much here. */
/*	sport->mapbase =  FIXME check what value should go here */


#ifndef CONFIG_OF /* this was already assigned at driver probe */
	/* this may need to be changed Keeps the serial_core calculations happy... */
	get_platform_clk(&clk);
	port->uartclk = clk.apb_clk;	//APB_CLOCK;
#endif /* CONFIG_OF */

	spin_lock_irqsave(&port->lock, flags);

	temp = readl(port->membase + CFG);
	temp |= (CFG_UART_EN | CFG_TX_EN| CFG_RX_EN | 0x3 /* data 8 */ );
	writel(temp, port->membase + CFG);
	temp = readl(port->membase + IE);
	writel(temp | CFG_TX_EN | CFG_RX_EN, port->membase + IE);

	spin_unlock_irqrestore(&port->lock, flags);
	return 0;
}

static void cortina_uart_shutdown(struct uart_port *port)
{
	cortina_uart_stop_tx(port);
	cortina_uart_stop_rx(port);
	free_irq(port->irq,port);
}

#ifdef CONFIG_CORTINA_PM_CALLBACK
static void cortina_uart_set_clock(struct uart_port *port, unsigned int freq)
{
	struct tty_struct *tty = port->state->port.tty;
	unsigned int cfg;

	port->uartclk = freq;

	if (tty == NULL)
		return;
/* this no longer works in 3.18 because termios member is
   no longer a pointer */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
	if (!tty->termios) {
		dev_warn(port->dev, "no set termios\n");
		return;
	}
#endif

/* kernel 3.4 has tty->termios as pointer to struct
    but kernel 3.18 has tty->termios as the struct itself */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
	freq /= uart_get_baud_rate(port, &tty->termios, NULL, 0, 115200);
#else
	freq /= uart_get_baud_rate(port, tty->termios, NULL, 0, 115200);
#endif

	cfg = readl(port->membase + CFG);
	cfg &= (1 << CFG_BAUD_SART) - 1;
	cfg |= freq << CFG_BAUD_SART;

	writel(cfg, port->membase + CFG);
	writel(freq / 2, port->membase + RX_SAMPLE);
}
#endif /* CONFIG_CORTINA_PM_CALLBACK */

static void cortina_uart_set_termios(struct uart_port *port, struct ktermios *termios,
								  struct ktermios *old)
{
	unsigned long flags,temp;
	int baud;
	unsigned sample_fre = 0;
#ifndef CONFIG_OF /* this was already assigned at driver probe */
	struct platform_clk clk;

	get_platform_clk(&clk);
	port->uartclk = clk.apb_clk;
#endif /* CONFIG_OF */
	baud = uart_get_baud_rate(port, termios, old, 0, 115200);
	temp = readl(port->membase + CFG);
	/* mask off the baud settings */
	temp &= 0xff;
	switch (baud) {
		case 9600:
			temp |= (port->uartclk / 9600) << CFG_BAUD_SART ;
			break;
		case 19200:
			temp |= (port->uartclk / 19200) << CFG_BAUD_SART ;
			break;
		case 38400:
			temp |= (port->uartclk / 38400) << CFG_BAUD_SART ;
			break;
		case 57600:
			temp |= (port->uartclk / 57600) << CFG_BAUD_SART ;
			break;
		case 115200:
			temp |= (port->uartclk / 115200) << CFG_BAUD_SART ;
			break;
		default:
			temp |= (port->uartclk / 38400) << CFG_BAUD_SART ;
			break;
	}

	/* Sampling rate should be half of baud count */
	sample_fre = (temp >> CFG_BAUD_SART) / 2;
	/* mask off the data width */
	temp &= 0xfffffffc;
	switch(termios->c_cflag & CSIZE) {
		case CS5:
			temp |= 0x0;
			break;
		case CS6:
			temp |= 0x1;
			break;
		case CS7:
			temp |= 0x2;
			break;
		case CS8:
			default:
			temp |= 0x3;
			break;
	}

	/* mask off Stop bits */
	temp &= ~(CFG_STOP_2BIT);
	if (termios->c_cflag & CSTOPB) {
		temp |= CFG_STOP_2BIT;
	}
	/* Parity */
	temp &= ~(CFG_PARITY_EN);
	temp |= CFG_PARITY_EVEN;
	if (termios->c_cflag & PARENB) {
		temp |= CFG_PARITY_EN;
		if (termios->c_cflag & PARODD)
			temp &= ~(CFG_PARITY_EVEN);
	}

	spin_lock_irqsave(&port->lock, flags);
	writel(temp, port->membase + CFG);
	writel(sample_fre, port->membase + RX_SAMPLE);
	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *cortina_uart_type(struct uart_port *port)
{
	if (port->type != PORT_CORTINA)
		return NULL;

	return container_of(port, struct cortina_uart_port, uart)->name;
}

static void cortina_uart_release_port(struct uart_port *port)
{
	/* Easy enough */
}

static int cortina_uart_request_port(struct uart_port *port)
{
	return 0;			/* How can we fail? */
}

static void cortina_uart_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_CORTINA;
}

static int cortina_uart_verify_port(struct uart_port *port,
				    struct serial_struct *ser)
{
	if (ser->type != PORT_UNKNOWN && ser->type != PORT_CORTINA)
		return -EINVAL;
	return 0;
}

#if defined(CONFIG_KGDB_SERIAL_CONSOLE) || \
    defined(CONFIG_KGDB_SERIAL_CONSOLE_MODULE)

static int cortina_poll_get_char(struct uart_port *port)
{
        unsigned int rx;

        while(readl(port->membase + INFO) & INFO_RX_EMPTY)
                ;
        rx = readl(port->membase + RX_DAT);

        return(rx);
}

static void cortina_poll_put_char(struct uart_port *port, unsigned char c)
{
#define wait_tx_done()  while (!(readl(port->membase + INFO) & INFO_TX_EMPTY));
        unsigned int ie = readl(port->membase + IE);

        // disable the interrupts
        writel(0, port->membase + IE);
        wait_tx_done();

        writel(c, port->membase + TX_DAT);
        wait_tx_done();

        //enable the interrupt
        writel(ie, port->membase + IE);
        return;
}

#endif

static struct uart_ops cortina_uart_ops = {
	.tx_empty = cortina_uart_tx_empty,
	.set_mctrl = cortina_uart_set_mctrl,
	.get_mctrl = cortina_uart_get_mctrl,
	.stop_tx = cortina_uart_stop_tx,
	.start_tx = cortina_uart_start_tx,
	.stop_rx = cortina_uart_stop_rx,
	.enable_ms = cortina_uart_enable_ms,
	.break_ctl = cortina_uart_break_ctl,
	.startup = cortina_uart_startup,
	.shutdown = cortina_uart_shutdown,
	.set_termios = cortina_uart_set_termios,
	.type = cortina_uart_type,
	.release_port = cortina_uart_release_port,
	.request_port = cortina_uart_request_port,
	.config_port = cortina_uart_config_port,
	.verify_port = cortina_uart_verify_port,
#if defined(CONFIG_KGDB_SERIAL_CONSOLE) || \
    defined(CONFIG_KGDB_SERIAL_CONSOLE_MODULE)
        .poll_get_char = cortina_poll_get_char,
        .poll_put_char = cortina_poll_put_char,
#endif
};

static inline void cortina_uart_interrupt_rx_chars(struct uart_port *port,
					     unsigned long status)
{
/* 3.18 arguement changed from struct * tty to  struct * tty_port */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
	struct tty_struct *tty = port->state->port.tty;
#else
	struct tty_port *ttyport = &port->state->port;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0) */
	unsigned int ch;
	unsigned int rx,flg;

	rx = readl(port->membase + INFO);
	if(INFO_RX_EMPTY & rx)
		return ;

	/* Read the character while FIFO is not empty */
	do{
		flg = TTY_NORMAL;
		port->icount.rx++;
		ch = readl(port->membase + RX_DAT);
#ifdef	CONFIG_CONTROL_SERIAL_INPUT
		if(!ca_uart_input)
			goto ignore;
#endif
		if(cortina_uart_ports[port->line]->break_indicator){
			if (sysrq && (status & RX_BREAK)) {/* RX BI is supported after Venus */
				if (uart_handle_break(port))
					goto ignore;
			}
		} else {
			if (sysrq && (status & RX_STOP_ERRE)) {/* stop err as BI */
				if (uart_handle_break(port))
					goto ignore;
			}
		}
		if(!(ch & 0x100)) { /* RX char is not valid */
			goto ignore;
		}
		if (uart_handle_sysrq_char(port, (unsigned char)ch)){
			goto ignore;
		}
/* 3.18 arguement changed from struct * tty to  struct * tty_port */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
		tty_insert_flip_char(tty, ch, flg);
#else
		tty_insert_flip_char(ttyport, ch, flg);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0) */
ignore:
		rx = readl(port->membase + INFO);
   } while(!(INFO_RX_EMPTY & rx)) ;

	spin_unlock(&port->lock);
/* 3.18 arguement changed from struct * tty to  struct * tty_port */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
	tty_flip_buffer_push(tty);
#else
	tty_flip_buffer_push(ttyport);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0) */
	spin_lock(&port->lock);
}


static inline void cortina_uart_interrupt_tx_chars(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;

	/* Process out of band chars */
	if (port->x_char) {
		/* Send next char */
		writel(port->x_char, port->membase + TX_DAT);
		goto done;
	}

	/* Nothing to do ? */
	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		cortina_uart_stop_tx(port);
		goto done;
	}

	cortina_transmit_buffer(port);

	/* Wake up */
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	/* Maybe we're done after all */
	if (uart_circ_empty(xmit)) {
		cortina_uart_stop_tx(port);
		goto done;
	}
	/* Ack the interrupt */
done:
	return;
}

irqreturn_t cortina_uart_interrupt(int irq, void *dev_id)
{
	struct uart_port *port = (struct uart_port *)dev_id;
	unsigned long temp;

	spin_lock(&port->lock);

	/* Must clear interrupt first! */
	temp = readl(port->membase + INT);
	writel(temp , port->membase + INT);

	/* Process any Rx chars first */
	cortina_uart_interrupt_rx_chars(port, temp);
	/* Then use any Tx space */
	cortina_uart_interrupt_tx_chars(port);

	spin_unlock(&port->lock);

	return IRQ_HANDLED;
}

#ifdef CONFIG_SERIAL_CS_CORTINA_CONSOLE

void cortina_console_write(struct console *co, const char *s,
			   unsigned int count)
{
	struct uart_port *port = &cortina_uart_ports[co->index]->uart;
	unsigned long previous;
	int i;
	unsigned long flags;
	int locked;

	local_irq_save(flags);
	if (port->sysrq) {
		locked = 0;
	} else if (oops_in_progress) {
		locked = spin_trylock(&port->lock);
	} else {
		spin_lock(&port->lock);
		locked = 1;
	}

	/* Save current state */
	previous = readl(port->membase + IE);
	/* Disable Tx interrupts so this all goes out in one go */
	cortina_uart_stop_tx(port);

	/* Write all the chars */
	for (i = 0; i < count; i++) {

		/* Wait the TX buffer to be empty, which can't take forever:
			* there's no flow control on the UART */
		while (!(readl(port->membase + INFO) & INFO_TX_EMPTY))
			udelay(1);

		/* Send the char */
		writel(*s, port->membase + TX_DAT);

		/* CR/LF stuff */
		if (*s++ == '\n') {
			/* Wait the TX buffer to be empty */
			while (!(readl(port->membase + INFO) & INFO_TX_EMPTY))
					udelay(1);
			writel('\r', port->membase + TX_DAT);
		}
	}

	writel(previous, port->membase + IE); /* Put it all back */

	if (locked)
		spin_unlock(&port->lock);
	local_irq_restore(flags);
}

static int __init cortina_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int ret;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (co->index < 0 || co->index >= UART_NR)
		return -ENODEV;

	if (!cortina_uart_ports[co->index])
		return -ENODEV;

	port = &cortina_uart_ports[co->index]->uart;

	/* This isn't going to do much, but it might change the baud rate. */
	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	ret = uart_set_options(port, co, baud, parity, bits, flow);

	return 0;
}

static struct uart_driver cortina_uart_driver;	/* Forward decl. */

static struct console cortina_console = {
	.name = "ttyS",
	.write = cortina_console_write,
	.device = uart_console_device,
	.setup = cortina_console_setup,
	.flags = CON_PRINTBUFFER,
	.index = -1,			/* Only possible option. */
	.data = &cortina_uart_driver,
};

/* Support EARLYCON */
static void cortina_putc(struct uart_port *port, int c)
{
	while (!(readl(port->membase + INFO) & INFO_TX_EMPTY))
			udelay(10);

	/* Send the char */
	writel(c, port->membase + TX_DAT);

}

static void cortina_early_write(struct console *con, const char *s, unsigned n)
{
	struct earlycon_device *dev = con->data;

	uart_console_write(&dev->port, s, n, cortina_putc);
}

static int __init cortina_early_console_setup(struct earlycon_device *device,
					    const char *opt)
{
	if (!device->port.membase)
		return -ENODEV;

	device->con->write = cortina_early_write;
	return 0;
}
EARLYCON_DECLARE(serial, cortina_early_console_setup);
OF_EARLYCON_DECLARE(serial, "cortina,serial", cortina_early_console_setup);


#endif

static struct uart_driver cortina_uart_driver = {
	.owner = THIS_MODULE,
	.driver_name = "cortina_uart",
	.dev_name = "ttyS",
	.major = TTY_MAJOR,
	.minor = 64,
	.nr = UART_NR,
	.cons = &cortina_console,
};

#ifdef CONFIG_CORTINA_PM_CALLBACK
static cs_status_t cortina_uart_freq_callback(cs_pm_freq_notifier_data_t *data)
{
	struct cortina_uart_port *up = (struct cortina_uart_port *)data->data;
	struct uart_port *port = &up->uart;
	struct tty_struct *tty = port->state->port.tty;
	unsigned int tries, freq;

	/* Check if the port is registered */
	if (!port || port->type != PORT_CORTINA)
		return CS_E_OK;

	if (data->event == CS_PM_FREQ_PRECHANGE) {
		dev_dbg(port->dev, "pre-suspend, freq %d\n", port->uartclk);

		if (port->cons)
			console_stop(port->cons);

		if (tty)
			tty_wait_until_sent(tty, port->timeout);
		for (tries = 20000; !cortina_uart_tx_empty(port) && tries; tries--)
			udelay(1);
		if (!tries)
			dev_err(port->dev, "unable to drain transmitter\n");

		spin_lock_irq(&port->lock);
		disable_irq_nosync(port->irq);
		cortina_uart_stop_tx(port);
		cortina_uart_stop_rx(port);
		spin_unlock_irq(&port->lock);
	} else if (data->event == CS_PM_FREQ_POSTCHANGE) {
		switch (data->new_peripheral_clk) {
		case CS_PERIPHERAL_FREQUENCY_150:
			freq = 150000000;
			break;
		case CS_PERIPHERAL_FREQUENCY_170:
			freq = 170000000;
			break;
		default:
			freq = 100000000;
		}

		spin_lock_irq(&port->lock);
		cortina_uart_set_clock(port, freq);
		cortina_uart_start_tx(port);
		cortina_uart_start_rx(port);
		enable_irq(port->irq);
		spin_unlock_irq(&port->lock);

		if (port->cons)
			console_start(port->cons);

		dev_dbg(port->dev, "resumed, freq %d\n", port->uartclk);
	}

	return CS_E_OK;
}

static cs_pm_freq_notifier_t cortina_uart_freq_notifier = {
	.notifier = cortina_uart_freq_callback,
};
#endif /* CONFIG_CORTINA_PM_CALLBACK */

static int serial_cortina_probe(struct platform_device *pdev)
{
	int ret;
	void __iomem *base;
#ifndef CONFIG_OF /* pre DT */
	struct resource *mem, *irq;
	struct platform_clk clk;
#endif

	struct cortina_uart_port *port;

#ifdef CONFIG_OF /* DT support */
	const struct of_device_id *match;

	/* assign DT node pointer */
        struct device_node *np = pdev->dev.of_node;
        struct resource mem_resource;
	u32 of_clock_frequency;
	struct clk * pclk_info;
	int uart_idx;

	/* search DT for a match */
        match = of_match_device(cortina_uart_of_match, &pdev->dev);
        if (!match)
                return -EINVAL;
#endif /* DT support */

	for( uart_idx=0; uart_idx < UART_NR; ++uart_idx)
		if ( cortina_uart_ports[uart_idx] == NULL )
			break;

	if ( uart_idx >= UART_NR)
		return -EINVAL;

	port = devm_kzalloc(&pdev->dev,
			    sizeof(struct cortina_uart_port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;
	snprintf(port->name, sizeof(port->name), "Cortina UART%d", uart_idx);

	/* get "reg" property from DT and convert to platform mem address resource */
	ret = of_address_to_resource(np, 0, &mem_resource);
        if (ret) {
                dev_warn(&pdev->dev, "invalid address %d\n", ret);
                return ret;
        }


	base = devm_ioremap(&pdev->dev, mem_resource.start,
			    resource_size(&mem_resource));
	if (!base) {
		devm_kfree(&pdev->dev, port);
		return -ENOMEM;
	}

#ifndef CONFIG_OF /* pre DT */
	/* get register base */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!mem) {
                dev_err(&pdev->dev, "no memory resource\n");
                return -ENODEV;
        }

        irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
        if (!irq) {
                dev_err(&pdev->dev, "no irq resource\n");
                return -ENODEV;
        }

	get_platform_clk(&clk);
#endif /* pre DT */

#ifdef CONFIG_OF /* DT support */
	/* assign reg base and irq from DT */
	port->uart.irq = irq_of_parse_and_map(np, 0);
	port->uart.membase =  base;
	port->uart.mapbase =  mem_resource.start;
#else   /* pre DT */
	port->uart.irq = irq->start;
	port->uart.membase = (void *)mem->start;
	port->uart.mapbase = mem->start;
#endif /* DT support */

	port->uart.ops = &cortina_uart_ops;
	port->uart.dev = &pdev->dev;
	port->uart.line = uart_idx;
	port->uart.has_sysrq = IS_ENABLED(CONFIG_SERIAL_CORTINA_CONSOLE);

#ifdef CONFIG_OF   /* DT support */
	/* get clock-freqency tuple from DT and store value */
	if (of_property_read_u32(np, "clock-frequency", &of_clock_frequency))
	{
	  /* if we are here, it means DT node did not contain clock-frequency tuple */

          /* therefore, instead try to get clk rate through
             the clk driver that DT has stated we are consuming */

          pclk_info = clk_get(&pdev->dev, NULL);
          if (IS_ERR(pclk_info)) {
             dev_warn(&pdev->dev,
                                "clk or clock-frequency not defined\n");
             return PTR_ERR(pclk_info);
          }

          clk_prepare_enable(pclk_info);
          of_clock_frequency = clk_get_rate(pclk_info);
        }
	port->uart.uartclk = of_clock_frequency;

	if(of_property_read_bool(np, "wakeup-source"))
		port->may_wakeup = true;
	if(of_property_read_bool(np, "break-indicator"))
		port->break_indicator = true;
#else   /* pre DT */
	port->uart.uartclk = clk.apb_clk;
#endif /* DT support */

	port->uart.type = PORT_CORTINA;
	cortina_uart_ports[port->uart.line] = port;

	if(port->may_wakeup)
		device_init_wakeup(&pdev->dev, true);

	ret = uart_add_one_port(&cortina_uart_driver, &port->uart);
	if (ret)
		return ret;

#ifdef CONFIG_CORTINA_PM_CALLBACK
	cortina_uart_freq_notifier.data = (void *)port;
	cs_pm_freq_register_notifier(&cortina_uart_freq_notifier, 0);
#endif /* CONFIG_CORTINA_PM_CALLBACK */

	platform_set_drvdata(pdev, port);

	return 0;
}

static int serial_cortina_remove(struct platform_device *pdev)
{
	struct uart_port *port = platform_get_drvdata(pdev);

	if (port) {
#ifdef CONFIG_CORTINA_PM_CALLBACK
		cs_pm_freq_unregister_notifier(&cortina_uart_freq_notifier);
#endif /* CONFIG_CORTINA_PM_CALLBACK */
		uart_remove_one_port(&cortina_uart_driver, port);
	}

	platform_set_drvdata(pdev, NULL);
	return 0;
}

#ifdef CONFIG_PM
static int serial_cortina_suspend(struct platform_device *pdev,
				pm_message_t state)
{
	struct cortina_uart_port *port=(struct cortina_uart_port*)pdev->dev.driver_data;

	if(port->may_wakeup){
		device_set_wakeup_enable(&pdev->dev, 1);
		enable_irq_wake(port->uart.irq);
	}
	return 0;
}

static int serial_cortina_resume(struct platform_device *pdev)
{
	struct cortina_uart_port *port=(struct cortina_uart_port*)pdev->dev.driver_data;

	if(port->may_wakeup)
		device_set_wakeup_enable(&pdev->dev, 0);

	return 0;
}
#else
#define serial_cortina_suspend NULL
#define serial_cortina_resume NULL
#endif

/* Match table for of_platform binding */
static struct of_device_id cortina_uart_of_match[] = {
        { .compatible = "cortina,serial", },
        {}
};
MODULE_DEVICE_TABLE(of, cortina_uart_of_match);

static struct platform_driver serial_cortina_driver = {
	.probe          = serial_cortina_probe,
	.remove         = serial_cortina_remove,
#ifdef CONFIG_PM
	.suspend		= serial_cortina_suspend,
	.resume			= serial_cortina_resume,
#endif
	.driver         = {
		.name   = "cortina_serial",
		.owner  = THIS_MODULE,
		.of_match_table = cortina_uart_of_match,
        },
};
#ifdef	CONFIG_CONTROL_SERIAL_INPUT
static char *cortina_uart_proc_name = "ca_uart";
static struct proc_dir_entry *procfs = NULL;

static int ca_uart_input_read(struct seq_file *seq, void *v)
{
	seq_printf(seq, "serial input: %s (0:disable 1:enable)\n", ca_uart_input ? "enable":"disable");
	return 0;
}

static int ca_uart_input_open(struct inode *inode, struct file *file)
{
	return single_open(file, ca_uart_input_read, inode->i_private);
}

static ssize_t	 ca_uart_input_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	char buffer[64] = {0};
	if (buf && !copy_from_user(buffer, buf, count))
	{
	    ca_uart_input = simple_strtol(buffer, NULL, 10);
		    printk("%s serial input\n", ca_uart_input ? "enable":"disable");
	}
	return count;
}

static const struct proc_ops ca_uart_input_fops = {
        .proc_open           = ca_uart_input_open,
        .proc_read           = seq_read,
        .proc_write          = ca_uart_input_write,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#endif
static int __init cortina_uart_init(void)
{
	int ret;
#ifdef	CONFIG_CONTROL_SERIAL_INPUT
	struct proc_dir_entry *entry=NULL;
#endif
	ret = uart_register_driver(&cortina_uart_driver);
	if (ret)
		return ret;

	ret = platform_driver_register(&serial_cortina_driver);
	if (ret)
		uart_unregister_driver(&cortina_uart_driver);
#ifdef	CONFIG_CONTROL_SERIAL_INPUT
	/* create a directory */
	procfs = proc_mkdir(cortina_uart_proc_name, NULL);
	if(procfs == NULL)
	{
		printk("Register /proc/%s failed\n", cortina_uart_proc_name);
		return -ENOMEM;
	}
	entry = proc_create("serialInput", 0644, procfs, &ca_uart_input_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/serialInput failed\n", cortina_uart_proc_name);
		return -ENOMEM;
	}
#endif
	return ret;
}

static void __exit cortina_uart_exit(void)
{
#ifdef	CONFIG_CONTROL_SERIAL_INPUT
	remove_proc_entry("serialInput", procfs);
	remove_proc_entry(cortina_uart_proc_name, NULL);
#endif
	platform_driver_unregister(&serial_cortina_driver);
	uart_unregister_driver(&cortina_uart_driver);
}

module_init(cortina_uart_init);
module_exit(cortina_uart_exit);

module_param(sysrq, bool, 0644);
MODULE_PARM_DESC(sysrq, "sysrq break key workarond");

MODULE_AUTHOR("Cortina-Systems");
MODULE_DESCRIPTION(" Cortina UART driver");
MODULE_LICENSE("GPL");

