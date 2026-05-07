/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of ME attribute: Physical path termination point LCT UNI (83)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Physical path termination point LCT UNI (83)
 */

#ifndef __MIB_LCTUNI_TABLE_H__
#define __MIB_LCTUNI_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table LCTUNI attribute index */
#define MIB_TABLE_LCTUNI_ATTR_NUM (2)
#define MIB_TABLE_LCTUNI_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_LCTUNI_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)2)

// Table LCT UNI entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  AdminState;
} __attribute__((aligned)) MIB_TABLE_LCTUNI_T;


#ifdef __cplusplus
}
#endif

#endif
