/*
 * Copyright (C) 2022 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _LIST_H_
#define _LIST_H_

// #include <stddef.h>

struct map_list {
	struct map_list *next;
	struct map_list *prev;
};

static inline void list_init(struct map_list *list)
{
	list->next = NULL;
	list->prev = NULL;
}

static inline void list_add(struct map_list *list, struct map_list *item)
{
	item->next = list->next;
	item->prev = list;
	if (list->next)
		list->next->prev = item;
	list->next = item;
}

static inline void list_del(struct map_list *item)
{
	if (item->next)
		item->next->prev = item->prev;
	if (item->prev)
		item->prev->next = item->next;
	item->next = NULL;
	item->prev = NULL;
}

static inline int list_is_empty(const struct map_list *list)
{
	return list->next == NULL;
}

static inline unsigned int list_len(const struct map_list *list)
{
	struct map_list *item;
	int count = 0;
	for (item = list->next; item != NULL; item = item->next)
		count++;
	return count;
}

#endif /* LIST_H */
