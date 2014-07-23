/*
 * hashbase - https://github.com/MaciejCzyzewski/hashbase
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Maciej A. Czyzewski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author: Maciej A. Czyzewski <maciejanthonyczyzewski@gmail.com>
 */

#ifndef _HB_MAP_H_
#define _HB_MAP_H_

#define HB_MAP_FULL -3                      /* Hashmap is full */
#define HB_MAP_OMEM -2                      /* Out of Memory */

/* any_t is a pointer. This allows you to put arbitrary structures in
 * the map. */
typedef void *any_t;

/* PFany is a pointer to a function that can take two any_t arguments
 * and return an integer. Returns status code.. */
typedef int (*PFany)(any_t, any_t);

/* We need to keep keys and values. */
typedef struct _map_bucket {
    char* key;
    int in_use;
    any_t data;
} map_bucket_t;

/* A map has some maximum size and current size,
 * as well as the data to hold. */
typedef struct _map {
    int table_size;
    int size;
    map_bucket_t *data;
} map_t;

/* Create global database. */
int    map_init(void);

/* Return an empty map. Returns NULL if empty. */
map_t *map_new(void);

/* Iteratively call f with argument (item, data) for
 * each element data in the map. The function must
 * return a map status code. If it returns anything other
 * than HB_OK the traversal is terminated. f must
 * not reenter any map functions, or deadlock may arise. */
int    map_iterate(map_t *, PFany, any_t);

/* Add an element to the map. Return HB_OK or MAP_OMEM. */
int    map_put(map_t *, char *, any_t);
 
/* Get an element from the map. Return HB_OK or HB_ERR. */
int    map_get(map_t *, char *, any_t *);

/* Remove an element from the map. Return HB_OK or HB_ERR. */
int    map_remove(map_t *, char *);

/* Get any element. Return HB_OK or HB_ERR.
 * remove - should the element be removed from the map */
int    map_get_one(map_t *, any_t *, int);

/* Free the map. */
void   map_free(map_t *);

/* Get the current size of a map. */
int    map_length(map_t *);

#endif