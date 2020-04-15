/*
 * ptrarray.c
 * simple pointer array implementation
 *
 * Copyright (c) 2011 Nikias Bassen, All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "ptrarray.h"

ptrarray_t *ptr_array_new(int capacity)
{
	ptrarray_t *pa = (ptrarray_t*)malloc(sizeof(ptrarray_t));
	pa->pdata = (void**)malloc(sizeof(void*) * capacity);
	pa->capacity = capacity;
	pa->capacity_step = (capacity > 4096) ? 4096 : capacity;
	pa->len = 0;
	return pa;
}

void ptr_array_free(ptrarray_t *pa)
{
	if (!pa) return;
	if (pa->pdata) {
		free(pa->pdata);
	}
	free(pa);
}

void ptr_array_add(ptrarray_t *pa, void *data)
{
	if (!pa || !pa->pdata || !data) return;
	size_t remaining = pa->capacity-pa->len;
	if (remaining == 0) {
		pa->pdata = realloc(pa->pdata, sizeof(void*) * (pa->capacity + pa->capacity_step));
		pa->capacity += pa->capacity_step;
	}
	pa->pdata[pa->len] = data;
	pa->len++;
}

void* ptr_array_index(ptrarray_t *pa, size_t array_index)
{
	if (!pa) return NULL;
	if (array_index >= pa->len) {
		return NULL;
	}
	return pa->pdata[array_index];
}
