/*
 * Pin control driver for Realtek Elnath SoCs
 *
 * Copyright (C) 2022 Realtek, Inc.
 *
 *
 * Based on pinctrl-digicolor.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/gpio/driver.h>
#include <linux/spinlock.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#ifdef CONFIG_ARCH_CORTINA_SMC
#include <soc/cortina/cortina-smc.h>
#endif
#include <soc/cortina/cortina-soc.h>
#include "pinctrl-utils.h"

#define DRIVER_NAME	"pinctrl-rtk-elnath"

#define GPIO0_MUX_OFFSET	0x00
/*** Taurus ****************
0xf43200f0 GLOBAL_PIN_MUX
0xf43200f4 GLOBAL_PIN_MUX_NEW
0xf43200f8 GLOBAL_SD_IO_PULL_CONTROL
0xf43200fc GLOBAL_IO_DRIVE_CONTROL
0xf4320100 GLOBAL_GPIO_MUX_0
0xf4320104 GLOBAL_GPIO_MUX_1
0xf4320108 GLOBAL_GPIO_MUX_2
0xf432010c GLOBAL_GPIO_MUX_3
0xf4320110 GLOBAL_GPIO_MUX_4
**********************************/

enum elnath_pinmux_groups {
	EMPTY_GROUP = -1,

	/*Global Pin Mux 0*/
	LED0 = 0,
	LED1,
	LED2,
	LED3,
	LED4,
	LED5,
	LED6,
	LED7,
	LED8,
	LED9,
	LED10,
	LED11,
	LED12,
	LED13,
	LED14,
	LED15,
	LED0_Q,
	LED1_Q,
	LED2_Q,
	LED3_Q,
	LED4_Q,
	LED5_Q,
	LED6_Q,
	LED7_Q,
	LED8_Q,
	LED9_Q,
	LED10_Q,
	LED11_Q,
	LED12_Q,
	LED13_Q,
	LED14_Q,
	LED15_Q,

	/*Global Pin Mux 1*/
	UART0	= 32,
	UART1 = 34,
	UART1_Q,
	HS_UART2,
	HS_UART2_Q,
	HS_UART2_FC,
	HS_UART2_FC_Q,
	EXT_RESET_OUT,
	REF_CLK,
	JTAG = 44,
	PMD,
	SLED = 48,
	OPT = 52,
	OPT_Q,
	ONU,
	ONU_Q,
	SPIF,
	PWM0 = 60,
	PWM0_Q,
	PWM1,

	/*Global Pin Mux 2*/
	SPI = 64,
	SPI_Q,
	SPI_CS1 = 68,
	SPI_CS2,
	SPI_CS3,
	SPI_CS4,
	SPI_CS5,
	SPI_CS6,
	SPI_CS7,
	SPI_CS1_Q = 76,
	SPI_CS2_Q,
	SPI_CS4_Q,
	I2C0 = 80,
	I2C1,
	MDIO = 84,
	PCM = 88,
	ZSI_ISI,
	SSP_EXTCLK,
	I2S0_M = 92,
	I2S0_S,
	I2S1_M,
	I2S1_S,
};

static const unsigned int led0_pins[] = {0};
static const unsigned int led1_pins[] = {1};
static const unsigned int led2_pins[] = {2};
static const unsigned int led3_pins[] = {3};
static const unsigned int led4_pins[] = {4};
static const unsigned int led5_pins[] = {5};
static const unsigned int led6_pins[] = {6};
static const unsigned int led7_pins[] = {7};
static const unsigned int led8_pins[] = {8};
static const unsigned int led9_pins[] = {9};
static const unsigned int led10_pins[] = {10};
static const unsigned int led11_pins[] = {11};
static const unsigned int led12_pins[] = {12};
static const unsigned int led13_pins[] = {13};
static const unsigned int led14_pins[] = {14};
static const unsigned int led15_pins[] = {15};
static const unsigned int led0_q_pins[] = {96};
static const unsigned int led1_q_pins[] = {97};
static const unsigned int led2_q_pins[] = {98};
static const unsigned int led3_q_pins[] = {99};
static const unsigned int led4_q_pins[] = {100};
static const unsigned int led5_q_pins[] = {101};
static const unsigned int led7_q_pins[] = {102};
static const unsigned int led8_q_pins[] = {103};
static const unsigned int led9_q_pins[] = {108};
static const unsigned int led10_q_pins[] = {109};
static const unsigned int led15_q_pins[] = {113};
static const unsigned int sled_pins[] = {11, 12};
static const unsigned int pmd_pins[] = {136, 137, 138, 139};
static const unsigned int pwm0_pins[] = {15};
static const unsigned int pwm0_q_pins[] = {65};
static const unsigned int pwm1_pins[] = {48};
static const unsigned int i2c0_pins[] = {68, 69};
static const unsigned int i2c1_pins[] = {101, 102};
static const unsigned int zsi_isi_pins[] = {72, 73, 74, 75};
static const unsigned int ext_reset_out_pins[] = {128};
static const unsigned int ref_clk_pins[] = {129};
static const unsigned int mdio_pins[] = {130, 131};
static const unsigned int uart0_pins[] = {132, 133};
static const unsigned int uart1_pins[] = {134, 135};
static const unsigned int uart1_q_pins[] = {6, 98};
static const unsigned int hs_uart2_pins[] = {2, 3};
static const unsigned int hs_uart2_q_pins[] = {112, 113};
static const unsigned int hs_uart2_fc_pins[] = {8, 9};
static const unsigned int hs_uart2_fc_q_pins[] = {109, 111};
static const unsigned int jtag_pins[] = {136, 137, 138, 139, 140};
static const unsigned int onu_pins[] = {142, 143};
static const unsigned int onu_q_pins[] = {111, 112};
static const unsigned int opt_pins[] = {13, 14, 15};
static const unsigned int opt_q_pins[] = {13, 14, 109};
static const unsigned int spif_pins[] = {32, 33, 34, 35, 42, 47, 48};
static const unsigned int spi_pins[] = {64, 65, 66, 67};
static const unsigned int spi_cs1_pins[] = {4};
static const unsigned int spi_cs2_pins[] = {5};
static const unsigned int spi_cs3_pins[] = {6};
static const unsigned int spi_cs4_pins[] = {7};
static const unsigned int spi_cs5_pins[] = {11};
static const unsigned int spi_cs6_pins[] = {98};
static const unsigned int spi_cs7_pins[] = {97};
static const unsigned int spi_q_pins[] = {65, 96, 100, 108};
static const unsigned int spi_cs1_q_pins[] = {103};
static const unsigned int spi_cs2_q_pins[] = {102};
static const unsigned int spi_cs4_q_pins[] = {12};
static const unsigned int ssp_extclk_pins[] = {70, 71};
static const unsigned int pcm_pins[] = {72, 73, 74, 75};
static const unsigned int i2s0_m_pins[] = {136, 137, 138, 139, 140};
static const unsigned int i2s0_s_pins[] = {136, 138, 139, 140};
static const unsigned int i2s1_m_pins[] = {99, 101, 111, 112, 113};
static const unsigned int i2s1_s_pins[] = {101, 111, 112, 113};

static const int led0_conflicts[] = {EMPTY_GROUP};
static const int led1_conflicts[] = {EMPTY_GROUP};
static const int led2_conflicts[] = {HS_UART2};
static const int led3_conflicts[] = {HS_UART2};
static const int led4_conflicts[] = {SPI_CS1};
static const int led5_conflicts[] = {SPI_CS2};
static const int led6_conflicts[] = {UART1_Q, SPI_CS3};
static const int led7_conflicts[] = {SPI_CS4};
static const int led8_conflicts[] = {HS_UART2_FC};
static const int led9_conflicts[] = {HS_UART2_FC};
static const int led10_conflicts[] = {EMPTY_GROUP};
static const int led11_conflicts[] = {SLED, SPI_CS5};
static const int led12_conflicts[] = {SLED, SPI_CS4_Q};
static const int led13_conflicts[] = {OPT, OPT_Q};
static const int led14_conflicts[] = {OPT, OPT_Q};
static const int led15_conflicts[] = {PWM0, OPT};
static const int led0_q_conflicts[] = {SPI_Q};
static const int led1_q_conflicts[] = {SPI_CS7};
static const int led2_q_conflicts[] = {UART1_Q, SPI_CS6};
static const int led3_q_conflicts[] = {I2S1_M};
static const int led4_q_conflicts[] = {SPI_Q};
static const int led5_q_conflicts[] = {I2C1, I2S1_M, I2S1_S};
static const int led7_q_conflicts[] = {I2C1, SPI_CS2_Q};
static const int led8_q_conflicts[] = {SPI_CS1_Q};
static const int led9_q_conflicts[] = {SPI_Q};
static const int led10_q_conflicts[] = {HS_UART2_FC_Q, OPT_Q};
static const int led15_q_conflicts[] = {HS_UART2_Q, I2S1_M, I2S1_S};
static const int sled_conflicts[] = {LED11, LED12, SPI_CS5, SPI_CS4_Q};
static const int pmd_conflicts[] = {JTAG, I2S0_M, I2S0_S};
static const int pwm0_conflicts[] = {LED15, OPT};
static const int pwm0_q_conflicts[] = {SPI, SPI_Q};
static const int pwm1_conflicts[] = {SPIF};
static const int i2c0_conflicts[] = {EMPTY_GROUP};
static const int i2c1_conflicts[] = {LED5_Q, LED7_Q, I2S1_M, I2S1_S, SPI_CS2_Q};
static const int zsi_isi_conflicts[] = {PCM};
static const int ext_reset_out_conflicts[] = {EMPTY_GROUP};
static const int ref_clk_conflicts[] = {EMPTY_GROUP};
static const int mdio_conflicts[] = {EMPTY_GROUP};
static const int uart0_conflicts[] = {EMPTY_GROUP};
static const int uart1_conflicts[] = {EMPTY_GROUP};
static const int uart1_q_conflicts[] = {LED6, SPI_CS3, LED2_Q, SPI_CS6};
static const int hs_uart2_conflicts[] = {LED2, LED3};
static const int hs_uart2_q_conflicts[] = {ONU_Q,  LED15_Q, I2S1_M, I2S1_S};
static const int hs_uart2_fc_conflicts[] = {LED8, LED9};
static const int hs_uart2_fc_q_conflicts[] = {LED10_Q, OPT_Q, ONU_Q, I2S1_M, I2S1_S};
static const int jtag_conflicts[] = {PMD, I2S0_M, I2S0_S};
static const int onu_conflicts[] = {EMPTY_GROUP};
static const int onu_q_conflicts[] = {HS_UART2_FC_Q, I2S1_M, I2S1_S, HS_UART2_Q};
static const int opt_conflicts[] = {LED13, LED14, LED15, PWM0};
static const int opt_q_conflicts[] = {LED13, LED14, LED10_Q, HS_UART2_FC_Q};
static const int spif_conflicts[] = {PWM1};
static const int spi_conflicts[] = {PWM0_Q};
static const int spi_cs1_conflicts[] = {LED4};
static const int spi_cs2_conflicts[] = {LED5};
static const int spi_cs3_conflicts[] = {LED6, UART1_Q};
static const int spi_cs4_conflicts[] = {LED7};
static const int spi_cs5_conflicts[] = {LED11, SLED};
static const int spi_cs6_conflicts[] = {LED2_Q, UART1_Q};
static const int spi_cs7_conflicts[] = {LED1_Q};
static const int spi_q_conflicts[] = {PWM0_Q, LED0_Q, LED4_Q, LED9_Q};
static const int spi_cs1_q_conflicts[] = {LED8_Q};
static const int spi_cs2_q_conflicts[] = {LED7_Q, I2C1};
static const int spi_cs4_q_conflicts[] = {LED12, SLED};
static const int ssp_extclk_conflicts[] = {EMPTY_GROUP};
static const int pcm_conflicts[] = {ZSI_ISI};
static const int i2s0_m_conflicts[] = {PMD, JTAG, I2S0_S};
static const int i2s0_s_conflicts[] = {PMD, JTAG, I2S0_M};
static const int i2s1_m_conflicts[] = {LED3_Q, LED5_Q, LED15_Q, ONU_Q, I2C1, HS_UART2_FC_Q, HS_UART2_Q, I2S1_S};
static const int i2s1_s_conflicts[] = {LED5_Q, LED15_Q, ONU_Q, I2C1, HS_UART2_FC_Q, HS_UART2_Q, I2S1_M};

#define DEFINE_RTK_PIN_GROUP(group, pinmux) \
	{ \
		.name = #group "_group", \
		.pins = group ## _pins, \
		.npins = ARRAY_SIZE(group ## _pins), \
		.pinmux_offset = pinmux, \
		.pinmux_conflicts =group ## _conflicts, \
		.nconflicts = ARRAY_SIZE(group ## _conflicts), \
	}

struct rtk_pin_group {
	const char *name;
	const unsigned int *pins;
	const unsigned int npins;
	const unsigned int pinmux_offset;
	const int *pinmux_conflicts;
	const unsigned int nconflicts;
};

static const struct rtk_pin_group elnath_groups[] = {
	DEFINE_RTK_PIN_GROUP(led0,			LED0),
	DEFINE_RTK_PIN_GROUP(led1,			LED1),
	DEFINE_RTK_PIN_GROUP(led2,			LED2),
	DEFINE_RTK_PIN_GROUP(led3,			LED3),
	DEFINE_RTK_PIN_GROUP(led4,			LED4),
	DEFINE_RTK_PIN_GROUP(led5,			LED5),
	DEFINE_RTK_PIN_GROUP(led6,			LED6),
	DEFINE_RTK_PIN_GROUP(led7,			LED7),
	DEFINE_RTK_PIN_GROUP(led8,			LED8),
	DEFINE_RTK_PIN_GROUP(led9,			LED9),
	DEFINE_RTK_PIN_GROUP(led10,			LED10),
	DEFINE_RTK_PIN_GROUP(led11,			LED11),
	DEFINE_RTK_PIN_GROUP(led12,			LED12),
	DEFINE_RTK_PIN_GROUP(led13,			LED13),
	DEFINE_RTK_PIN_GROUP(led14,			LED14),
	DEFINE_RTK_PIN_GROUP(led15,			LED15),
	DEFINE_RTK_PIN_GROUP(led0_q,			LED0_Q),
	DEFINE_RTK_PIN_GROUP(led1_q, 		LED1_Q),
	DEFINE_RTK_PIN_GROUP(led2_q,			LED2_Q),
	DEFINE_RTK_PIN_GROUP(led3_q,			LED3_Q),
	DEFINE_RTK_PIN_GROUP(led4_q,			LED4_Q),
	DEFINE_RTK_PIN_GROUP(led5_q,			LED5_Q),
	DEFINE_RTK_PIN_GROUP(led7_q,			LED7_Q),
	DEFINE_RTK_PIN_GROUP(led8_q,			LED8_Q),
	DEFINE_RTK_PIN_GROUP(led9_q,			LED9_Q),
	DEFINE_RTK_PIN_GROUP(led10_q,		LED10_Q),
	DEFINE_RTK_PIN_GROUP(led15_q,		LED15_Q),
	DEFINE_RTK_PIN_GROUP(uart0,			UART0),
	DEFINE_RTK_PIN_GROUP(uart1,			UART1),
	DEFINE_RTK_PIN_GROUP(uart1_q,		UART1_Q),
	DEFINE_RTK_PIN_GROUP(hs_uart2,		HS_UART2),
	DEFINE_RTK_PIN_GROUP(hs_uart2_q,		HS_UART2_Q),
	DEFINE_RTK_PIN_GROUP(hs_uart2_fc,	HS_UART2_FC),
	DEFINE_RTK_PIN_GROUP(hs_uart2_fc_q,	HS_UART2_FC_Q),
	DEFINE_RTK_PIN_GROUP(ext_reset_out,	EXT_RESET_OUT),
	DEFINE_RTK_PIN_GROUP(ref_clk,		REF_CLK),
	DEFINE_RTK_PIN_GROUP(jtag,			JTAG),
	DEFINE_RTK_PIN_GROUP(pmd,			PMD),
	DEFINE_RTK_PIN_GROUP(sled,			SLED),
	DEFINE_RTK_PIN_GROUP(opt,			OPT),
	DEFINE_RTK_PIN_GROUP(opt_q,			OPT_Q),
	DEFINE_RTK_PIN_GROUP(onu,			ONU),
	DEFINE_RTK_PIN_GROUP(onu_q,			ONU_Q),
	DEFINE_RTK_PIN_GROUP(spif,			SPIF),
	DEFINE_RTK_PIN_GROUP(pwm0,			PWM0),
	DEFINE_RTK_PIN_GROUP(pwm0_q,			PWM0_Q),
	DEFINE_RTK_PIN_GROUP(pwm1,			PWM1),
	DEFINE_RTK_PIN_GROUP(spi,			SPI),
	DEFINE_RTK_PIN_GROUP(spi_q,			SPI_Q),
	DEFINE_RTK_PIN_GROUP(spi_cs1,		SPI_CS1),
	DEFINE_RTK_PIN_GROUP(spi_cs2,		SPI_CS2),
	DEFINE_RTK_PIN_GROUP(spi_cs3,		SPI_CS3),
	DEFINE_RTK_PIN_GROUP(spi_cs4,		SPI_CS4),
	DEFINE_RTK_PIN_GROUP(spi_cs5,		SPI_CS5),
	DEFINE_RTK_PIN_GROUP(spi_cs6,		SPI_CS6),
	DEFINE_RTK_PIN_GROUP(spi_cs7,		SPI_CS7),
	DEFINE_RTK_PIN_GROUP(spi_cs1_q,		SPI_CS1_Q),
	DEFINE_RTK_PIN_GROUP(spi_cs2_q,		SPI_CS2_Q),
	DEFINE_RTK_PIN_GROUP(spi_cs4_q,		SPI_CS4_Q),
	DEFINE_RTK_PIN_GROUP(i2c0,			I2C0),
	DEFINE_RTK_PIN_GROUP(i2c1,			I2C1),
	DEFINE_RTK_PIN_GROUP(mdio,			MDIO),
	DEFINE_RTK_PIN_GROUP(pcm,			PCM),
	DEFINE_RTK_PIN_GROUP(zsi_isi,		ZSI_ISI),
	DEFINE_RTK_PIN_GROUP(ssp_extclk,		SSP_EXTCLK),
	DEFINE_RTK_PIN_GROUP(i2s0_m,			I2S0_M),
	DEFINE_RTK_PIN_GROUP(i2s0_s,			I2S0_S),
	DEFINE_RTK_PIN_GROUP(i2s1_m,			I2S1_M),
	DEFINE_RTK_PIN_GROUP(i2s1_s,			I2S1_S),
};

static const char * const led0_groups[] = {"led0_group", "led0_q_group"};
static const char * const led1_groups[] = {"led1_group", "led1_q_group"};
static const char * const led2_groups[] = {"led2_group", "led2_q_group"};
static const char * const led3_groups[] = {"led3_group", "led3_q_group"};
static const char * const led4_groups[] = {"led4_group", "led4_q_group"};
static const char * const led5_groups[] = {"led5_group", "led5_q_group"};
static const char * const led6_groups[] = {"led6_group"};
static const char * const led7_groups[] = {"led7_group", "led7_q_group"};
static const char * const led8_groups[] = {"led8_group", "led8_q_group"};
static const char * const led9_groups[] = {"led9_group", "led9_q_group"};
static const char * const led10_groups[] = {"led10_group", "led10_q_group"};
static const char * const led11_groups[] = {"led11_group"};
static const char * const led12_groups[] = {"led12_group"};
static const char * const led13_groups[] = {"led13_group"};
static const char * const led14_groups[] = {"led14_group"};
static const char * const led15_groups[] = {"led15_group", "led15_q_group"};
static const char * const uart0_groups[] = {"uart0_group"};
static const char * const uart1_groups[] = {"uart1_group", "uart1_q_group"};
static const char * const hs_uart2_groups[] = {"hs_uart2_group", "hs_uart2_q_group"};
static const char * const hs_uart2_fc_groups[] = {"hs_uart2_fc_group", "hs_uart2_fc_q_group"};
static const char * const ext_reset_out_groups[] = {"ext_reset_out_group"};
static const char * const ref_clk_groups[] = {"ref_clk_group"};
static const char * const jtag_groups[] = {"jtag_group"};
static const char * const pmd_groups[] = {"pmd_group"};
static const char * const sled_groups[] = {"sled_group"};
static const char * const opt_groups[] = {"opt_group", "opt_q_group"};
static const char * const onu_groups[] = {"onu_group", "onu_q_group"};
static const char * const spif_groups[] = {"spif_group"};
static const char * const pwm0_groups[] = {"pwm0_group", "pwm0_q_group"};
static const char * const pwm1_groups[] = {"pwm1_group"};
static const char * const spi_groups[] = {"spi_group", "spi_q_group"};
static const char * const spi_cs1_groups[] = {"spi_cs1_group", "spi_cs1_q_group"};
static const char * const spi_cs2_groups[] = {"spi_cs2_group", "spi_cs2_q_group"};
static const char * const spi_cs3_groups[] = {"spi_cs3_group"};
static const char * const spi_cs4_groups[] = {"spi_cs4_group", "spi_cs4_q_group"};
static const char * const spi_cs5_groups[] = {"spi_cs5_group"};
static const char * const spi_cs6_groups[] = {"spi_cs6_group"};
static const char * const spi_cs7_groups[] = {"spi_cs7_group"};
static const char * const i2c0_groups[] = {"i2c0_group"};
static const char * const i2c1_groups[] = {"i2c1_group"};
static const char * const mdio_groups[] = {"mdio_group"};
static const char * const pcm_groups[] = {"pcm_group"};
static const char * const zsi_isi_groups[] = {"zsi_isi_group"};
static const char * const ssp_extclk_groups[] = {"ssp_extclk_group"};
static const char * const i2s0_m_groups[] = {"i2s0_m_group"};
static const char * const i2s0_s_groups[] = {"i2s0_s_group"};
static const char * const i2s1_m_groups[] = {"i2s1_m_group"};
static const char * const i2s1_s_groups[] = {"i2s1_s_group"};

#define DEFINE_RTK_PINMUX_FUNCTION(fname)	\
	{ \
		.name = #fname, \
		.groups = fname##_groups,		\
		.ngroups = ARRAY_SIZE(fname##_groups),	\
	}

struct rtk_function {
	const char *name;
	const char * const *groups;
	unsigned int ngroups;
};

static const struct rtk_function elnath_functions[] = {
	DEFINE_RTK_PINMUX_FUNCTION(led0),
	DEFINE_RTK_PINMUX_FUNCTION(led1),
	DEFINE_RTK_PINMUX_FUNCTION(led2),
	DEFINE_RTK_PINMUX_FUNCTION(led3),
	DEFINE_RTK_PINMUX_FUNCTION(led4),
	DEFINE_RTK_PINMUX_FUNCTION(led5),
	DEFINE_RTK_PINMUX_FUNCTION(led6),
	DEFINE_RTK_PINMUX_FUNCTION(led7),
	DEFINE_RTK_PINMUX_FUNCTION(led8),
	DEFINE_RTK_PINMUX_FUNCTION(led9),
	DEFINE_RTK_PINMUX_FUNCTION(led10),
	DEFINE_RTK_PINMUX_FUNCTION(led11),
	DEFINE_RTK_PINMUX_FUNCTION(led12),
	DEFINE_RTK_PINMUX_FUNCTION(led13),
	DEFINE_RTK_PINMUX_FUNCTION(led14),
	DEFINE_RTK_PINMUX_FUNCTION(led15),
	DEFINE_RTK_PINMUX_FUNCTION(uart0),
	DEFINE_RTK_PINMUX_FUNCTION(uart1),
	DEFINE_RTK_PINMUX_FUNCTION(hs_uart2),
	DEFINE_RTK_PINMUX_FUNCTION(hs_uart2_fc),
	DEFINE_RTK_PINMUX_FUNCTION(ext_reset_out),
	DEFINE_RTK_PINMUX_FUNCTION(ref_clk),
	DEFINE_RTK_PINMUX_FUNCTION(jtag),
	DEFINE_RTK_PINMUX_FUNCTION(pmd),
	DEFINE_RTK_PINMUX_FUNCTION(sled),
	DEFINE_RTK_PINMUX_FUNCTION(opt),
	DEFINE_RTK_PINMUX_FUNCTION(onu),
	DEFINE_RTK_PINMUX_FUNCTION(spif),
	DEFINE_RTK_PINMUX_FUNCTION(pwm0),
	DEFINE_RTK_PINMUX_FUNCTION(pwm1),
	DEFINE_RTK_PINMUX_FUNCTION(spi),
	DEFINE_RTK_PINMUX_FUNCTION(spi_cs1),
	DEFINE_RTK_PINMUX_FUNCTION(spi_cs2),
	DEFINE_RTK_PINMUX_FUNCTION(spi_cs3),
	DEFINE_RTK_PINMUX_FUNCTION(spi_cs4),
	DEFINE_RTK_PINMUX_FUNCTION(spi_cs5),
	DEFINE_RTK_PINMUX_FUNCTION(spi_cs6),
	DEFINE_RTK_PINMUX_FUNCTION(spi_cs7),
	DEFINE_RTK_PINMUX_FUNCTION(i2c0),
	DEFINE_RTK_PINMUX_FUNCTION(i2c1),
	DEFINE_RTK_PINMUX_FUNCTION(mdio),
	DEFINE_RTK_PINMUX_FUNCTION(pcm),
	DEFINE_RTK_PINMUX_FUNCTION(zsi_isi),
	DEFINE_RTK_PINMUX_FUNCTION(ssp_extclk),
	DEFINE_RTK_PINMUX_FUNCTION(i2s0_m),
	DEFINE_RTK_PINMUX_FUNCTION(i2s0_s),
	DEFINE_RTK_PINMUX_FUNCTION(i2s1_m),
	DEFINE_RTK_PINMUX_FUNCTION(i2s1_s),
};

#define GPIO_GROUP			5
#define PINS_PER_COLLECTION	32
#define PINS_COUNT		(GPIO_GROUP * PINS_PER_COLLECTION)

struct elnath_pinmap {
	void __iomem		*gpiomux_regs;
	void __iomem		*pinmux_regs;

	struct device		*dev;
	struct pinctrl_dev	*pctl;

	unsigned int groups_count;
	const struct rtk_pin_group *groups;

	unsigned int funcs_count;
	const struct rtk_function *funcs;

	spinlock_t		lock;
};

static int elnath_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	return pmap->groups_count;
}

static const char *elnath_get_group_name(struct pinctrl_dev *pctldev,
				     unsigned selector)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	return pmap->groups[selector].name;
}

static int elnath_get_group_pins(struct pinctrl_dev *pctldev, unsigned selector,
			     const unsigned **pins,
			     unsigned *num_pins)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	*pins = pmap->groups[selector].pins;
	*num_pins = pmap->groups[selector].npins;

	return 0;
}

static struct pinctrl_ops elnath_pinctrl_ops = {
	.get_groups_count	= elnath_get_groups_count,
	.get_group_name		= elnath_get_group_name,
	.get_group_pins		= elnath_get_group_pins,
	.dt_node_to_map		= pinconf_generic_dt_node_to_map_all,
	.dt_free_map		= pinctrl_utils_free_map,
};

static int elnath_get_functions_count(struct pinctrl_dev *pctldev)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	return pmap->funcs_count;
}

static const char *elnath_get_fname(struct pinctrl_dev *pctldev, unsigned selector)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	return pmap->funcs[selector].name;
}

static int elnath_get_groups(struct pinctrl_dev *pctldev, unsigned selector,
			 const char * const **groups,
			 unsigned * const num_groups)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);

	*groups = pmap->funcs[selector].groups;
	*num_groups = pmap->funcs[selector].ngroups;

	return 0;
}

static void elnath_client_sel(int pin_num, int *reg, int *bit)
{
	*bit = (pin_num % PINS_PER_COLLECTION);

	*reg = (pin_num/PINS_PER_COLLECTION)*4;
}

static int elnath_set_mux(struct pinctrl_dev *pctldev, unsigned selector,
		      unsigned group)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pctldev);
	const struct rtk_pin_group *pin_group = &pmap->groups[group];
	u32 reg;
	int bit_off, reg_off;
	int i;
	//get PIN_MUX register and bit offset
	elnath_client_sel(pin_group->pinmux_offset, &reg_off, &bit_off);

	dev_info(pmap->dev,"%s: pinmux group:%s reg_off:0x%x bit_off:%d\n",__func__, pin_group->name, reg_off, bit_off);

	reg = readl_relaxed(pmap->pinmux_regs + reg_off);
	reg |= (1 << bit_off);/* PIN_MUX set 1 as multi-function */
	writel_relaxed(reg, pmap->pinmux_regs + reg_off);

	//Clear conflict PIN_MUX bit
	for(i = 0; i < pin_group->nconflicts; i++)
	{
		if(pin_group->pinmux_conflicts[0] == EMPTY_GROUP)
			break;

		elnath_client_sel(pin_group->pinmux_conflicts[i], &reg_off, &bit_off);

		reg = readl_relaxed(pmap->pinmux_regs + reg_off);
		if(reg & (1 << bit_off))
		{
			reg &= ~(1 << bit_off);
			writel_relaxed(reg, pmap->pinmux_regs + reg_off);
			dev_info(pmap->dev,"%s: conflicts pinmux:%d reg_off:0x%x bit_off:%d\n",__func__, pin_group->pinmux_conflicts[i], reg_off, bit_off);
		}
	}

	//Clear conflict GPIO_MUX bit
	for(i = 0; i < pin_group->npins; i++)
	{
		elnath_client_sel(pin_group->pins[i], &reg_off, &bit_off);

		reg = readl_relaxed(pmap->gpiomux_regs + reg_off + GPIO0_MUX_OFFSET);
		if(reg & (1 << bit_off))
		{
			reg &= ~(1 << bit_off);
			writel_relaxed(reg, pmap->gpiomux_regs + reg_off + GPIO0_MUX_OFFSET);
			dev_info(pmap->dev,"%s: conflicts gpiomux:%d reg_off:0x%x bit_off:%d\n",__func__, pin_group->pins[i], reg_off, bit_off);
		}
	}
	return 0;
}

static int elnath_pmx_request_gpio(struct pinctrl_dev *pcdev,
			       struct pinctrl_gpio_range *range,
			       unsigned pin)
{
	struct elnath_pinmap *pmap = pinctrl_dev_get_drvdata(pcdev);
	int bit_off, reg_off;
	u32 reg;

	elnath_client_sel(pin, &reg_off, &bit_off);

	printk("%s: pin:%d reg_off:0x%x bit_off:%d\n",__func__, pin, reg_off, bit_off);

	reg = readl_relaxed(pmap->gpiomux_regs + reg_off + GPIO0_MUX_OFFSET);
	reg |= (1 << bit_off);
	writel_relaxed(reg, pmap->gpiomux_regs + reg_off + GPIO0_MUX_OFFSET);

	return 0;
}

static struct pinmux_ops elnath_pmxops = {
	.get_functions_count	= elnath_get_functions_count,
	.get_function_name		= elnath_get_fname,
	.get_function_groups	= elnath_get_groups,
	.set_mux				= elnath_set_mux,
	.gpio_request_enable	= elnath_pmx_request_gpio,
};

static int elnath_pinctrl_probe(struct platform_device *pdev)
{
	struct resource *r;
	struct pinctrl_desc *pctl_desc;
	struct pinctrl_pin_desc *pins;
	char *pin_names;
	struct elnath_pinmap *pmap;
	int name_len = strlen("GPIO_xxx") + 1;
	int i, j;

	pmap = devm_kzalloc(&pdev->dev, sizeof(*pmap), GFP_KERNEL);
	if (!pmap)
		return -ENOMEM;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	pmap->pinmux_regs = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(pmap->pinmux_regs))
		return PTR_ERR(pmap->pinmux_regs);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 1);

	pmap->gpiomux_regs = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(pmap->gpiomux_regs))
		return PTR_ERR(pmap->gpiomux_regs);

	pins = devm_kzalloc(&pdev->dev, sizeof(*pins)*PINS_COUNT, GFP_KERNEL);
	if (!pins)
		return -ENOMEM;

	pin_names = devm_kzalloc(&pdev->dev, name_len * PINS_COUNT, GFP_KERNEL);
	if (!pin_names)
		return -ENOMEM;

	for (i = 0; i < GPIO_GROUP; i++) {	/* GPIO_MUX */
		for (j = 0; j < PINS_PER_COLLECTION; j++) {
			int pin_id = i*PINS_PER_COLLECTION + j;
			char *name = &pin_names[pin_id * name_len];

			snprintf(name, name_len, "GPIO_%c%c%c", '0'+(i), '0'+(j/10), '0'+(j%10));
			pins[pin_id].number = pin_id;
			pins[pin_id].name = name;
		}
	}

	pctl_desc = devm_kzalloc(&pdev->dev, sizeof(*pctl_desc), GFP_KERNEL);
	if (!pctl_desc)
		return -ENOMEM;

	pmap->groups = elnath_groups;
	pmap->groups_count = ARRAY_SIZE(elnath_groups);
	pmap->funcs = elnath_functions;
	pmap->funcs_count = ARRAY_SIZE(elnath_functions);

	pctl_desc->name	= DRIVER_NAME,
	pctl_desc->owner = THIS_MODULE,
	pctl_desc->pctlops = &elnath_pinctrl_ops,
	pctl_desc->pmxops = &elnath_pmxops,
	pctl_desc->npins = PINS_COUNT;
	pctl_desc->pins = pins;

	pmap->dev = &pdev->dev;

	pmap->pctl = pinctrl_register(pctl_desc, &pdev->dev, pmap);
	if (IS_ERR(pmap->pctl)) {
		dev_err(&pdev->dev, "pinctrl driver registration failed\n");
		return PTR_ERR(pmap->pctl);
	}

#if 0 //def CONFIG_LEDS_CA77XX_PHY_2DIR
	{
	u32 reg;

	reg = readl_relaxed(pmap->gpiomux_regs + IO_DRV_OFFSET);
	reg |= BIT(9);	/* flash_ds for PHY Tx/Rx monitor */
	writel_relaxed(reg, pmap->gpiomux_regs + IO_DRV_OFFSET);
	}
#endif

	return 0;
}

static int elnath_pinctrl_remove(struct platform_device *pdev)
{
	struct elnath_pinmap *pmap = platform_get_drvdata(pdev);

	pinctrl_unregister(pmap->pctl);

	return 0;
}

static const struct of_device_id elnath_pinctrl_ids[] = {
	{ .compatible = "rtk,elnath-pinctrl" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, elnath_pinctrl_ids);

static struct platform_driver elnath_pinctrl_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = elnath_pinctrl_ids,
	},
	.probe = elnath_pinctrl_probe,
	.remove = elnath_pinctrl_remove,
};
module_platform_driver(elnath_pinctrl_driver);
