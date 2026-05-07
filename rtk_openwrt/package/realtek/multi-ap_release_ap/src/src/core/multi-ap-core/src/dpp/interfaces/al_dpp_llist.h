/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  Linked List for AL DPP Functions
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _AL_DPP_LINKED_LIST_H_
#define _AL_DPP_LINKED_LIST_H_

#include "platform.h"

struct dpp_llist_head {
	struct dpp_llist_head *prev;
	struct dpp_llist_head *next;
};

static inline void dpp_llist_init(struct dpp_llist_head *list)
{
	list->next = NULL;
	list->prev = NULL;
}

static inline void dpp_llist_add(struct dpp_llist_head *list, void *new_item, size_t new_item_size)
{
	struct dpp_llist_head *item;
	item = (struct dpp_llist_head *)PLATFORM_MALLOC(new_item_size);
	PLATFORM_MEMCPY(item, new_item, new_item_size);
	item->prev = list;
	item->next = list->next;
	if (list->next)
		list->next->prev = item;
	list->next = item;
}

static inline void dpp_llist_remove(struct dpp_llist_head *item)
{
	if (item->next)
		item->next->prev = item->prev;
	if (item->prev)
		item->prev->next = item->next;
	PLATFORM_FREE(item);
}

static inline void dpp_llist_empty(struct dpp_llist_head *list)
{
	struct dpp_llist_head *entry = list->next;
	struct dpp_llist_head *next;

	while (entry != NULL) {
		next = entry->next;
		PLATFORM_FREE(entry);
		entry = next;
	}
	dpp_llist_init(list);
}

#endif /* _AL_DPP_LINKED_LIST_H_ */