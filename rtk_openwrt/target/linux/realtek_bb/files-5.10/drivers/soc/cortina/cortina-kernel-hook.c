/*
 * Copyright (c) Cortina-Access Limited 2015.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * cortina-kernel-hook.c
 *   Only contain necessary code in Linux tree; otherwise code should be
 *   in kernel hook module.
 */

#include <linux/types.h>
#include <linux/module.h>

uint32_t ca_kh_hook = 0xfff8ffff;
EXPORT_SYMBOL(ca_kh_hook);

uint32_t ca_kh_debug = 0x0;
EXPORT_SYMBOL(ca_kh_debug);

uint32_t ca_kh_udp_offload_occasion = 0;
EXPORT_SYMBOL(ca_kh_udp_offload_occasion);

