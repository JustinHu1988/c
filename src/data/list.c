/*
 * Copyright (c) 2015 TextGlass
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include "list.h"

tg_list *tg_list_alloc(size_t initial_len, void (*callback)(void *item))
{
	tg_list *list;

	if(initial_len > TG_LIST_PREALLOC)
	{
		initial_len -= TG_LIST_PREALLOC;
	}
	else
	{
		initial_len = 0;
	}

	list = calloc(1, sizeof(tg_list) + (sizeof(tg_list_item) * initial_len));

	assert(list);

	tg_list_init(list, initial_len, callback);

	list->malloc = 1;

	TAILQ_INIT(&list->head);

	return list;
}

void tg_list_init(tg_list * list, size_t initial_len, void (*callback)(void *item))
{
	assert(list && list->magic != TG_LIST_MAGIC);

	list->magic = TG_LIST_MAGIC;
	list->size = 0;
	list->prealloc_len = TG_LIST_PREALLOC + initial_len;
	list->callback = callback;
	list->malloc = 0;

	TAILQ_INIT(&list->head);
}

static tg_list_item *tg_list_item_alloc(tg_list *list)
{
	tg_list_item *item;

	assert(list && list->magic == TG_LIST_MAGIC);

	if(list->size < list->prealloc_len)
	{
		item = &list->prealloc[list->size];

		assert(item->magic != TG_LIST_ITEM_MAGIC);

		item->magic = TG_LIST_ITEM_MAGIC;
		item->malloc = 0;

		return item;
	}

	item = malloc(sizeof(tg_list_item));

	assert(item);

	item->magic = TG_LIST_ITEM_MAGIC;
	item->malloc = 1;

	return item;
}

void tg_list_add(tg_list *list, void *value)
{
	tg_list_item *add;

	assert(list && list->magic == TG_LIST_MAGIC);
	assert(value);

	add = tg_list_item_alloc(list);

	add->value = value;

	TAILQ_INSERT_TAIL(&list->head, add, entry);

	list->size++;
}

void *tg_list_get(tg_list *list, size_t index)
{
	tg_list_item *item;

	assert(list && list->magic == TG_LIST_MAGIC);

	TG_LIST_FOREACH(list, item)
	{
		if(!index)
		{
			return item->value;
		}

		index--;
	}

	return NULL;
}

void *tg_list_get_from(tg_list_item *item, size_t index)
{
	assert(item);

	while(item)
	{
		assert(item->magic == TG_LIST_ITEM_MAGIC);

		if(!index)
		{
			return item->value;
		}

		index--;

		item = TAILQ_NEXT(item, entry);
	}

	return NULL;
}

long tg_list_index_str(tg_list *list, const char *value)
{
	long i = 0;
	tg_list_item *item;

	assert(list && list->magic == TG_LIST_MAGIC);
	assert(value);

	TG_LIST_FOREACH(list, item)
	{
		if(!strcmp(value, (char*)item->value))
		{
			return i;
		}

		i++;
	}

	return -1;
}

int tg_list_item_valid(tg_list_item *item)
{
	if(!item)
	{
		return 0;
	}

	assert(item->magic == TG_LIST_ITEM_MAGIC);

	return 1;
}

void tg_list_free(tg_list *list)
{
	tg_list_item *item, *next;

	if(!list)
	{
		return;
	}

	assert(list && list->magic == TG_LIST_MAGIC);

	TAILQ_FOREACH_SAFE(item, &list->head, entry, next)
	{
		TAILQ_REMOVE(&list->head, item, entry);

		assert(item->magic == TG_LIST_ITEM_MAGIC);

		if(list->callback)
		{
			list->callback(item->value);
		}

		item->magic = 0;

		if(item->malloc)
		{
			free(item);
		}

		list->size--;
	}

	assert(!list->size);

	TAILQ_INIT(&list->head);

	list->magic = 0;

	if(list->malloc)
	{
		free(list);
	}
}
