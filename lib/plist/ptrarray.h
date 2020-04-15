/*
 * ptrarray.h
 * header file for simple pointer array implementation
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
#ifndef PTRARRAY_H
#define PTRARRAY_H
#include <stdlib.h>

typedef struct ptrarray_t {
	void **pdata;
	size_t len;
	size_t capacity;
	size_t capacity_step;
} ptrarray_t;

ptrarray_t *ptr_array_new(int capacity);
void ptr_array_free(ptrarray_t *pa);
void ptr_array_add(ptrarray_t *pa, void *data);
void* ptr_array_index(ptrarray_t *pa, size_t index);
#endif
