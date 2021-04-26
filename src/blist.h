
/** blist.h, data-structures-c
 *
 *  MIT License
 *
 *  Copyright (C) 2021 Czespo
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

/**
 * A blist stores its elements as typeless blobs of bytes.
 */

#ifndef DS_BLIST_H
#define DS_BLIST_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct blist_s {
    unsigned char* array;
    size_t length, capacity, size;

    /// If `destructor` is not NULL, each element is
    /// passed to it when calling `blist_destroy()`.
    void (*destructor)(void*);
} blist_t;

blist_t* blist_new();
bool blist_init(blist_t* self, size_t capacity, size_t size);
blist_t* blist_create(size_t capacity, size_t size);
void blist_destroy(blist_t* self);

/** \brief Copies `list->size` bytes from `data` to the end of the list.
 *
 * \param list blist_t*
 * \param data void*
 * \param size size_t
 * \return void
 */
void blist_push(blist_t* self, void* data);

/** \brief Removes the last element from the list. If `destructor` is
 *  not NULL, the element is passed to it.
 *
 * \param self blist_t*
 * \return void
 *
 */
void blist_pop(blist_t* self);

/** \brief Removes the element at `index` and shifts all elements to
 *  the right of it one place. If `destructor` is not NULL,
 *  the element is passed to it.
 *
 * \param self blist_t*
 * \param index size_t
 * \return void
 */
void blist_remove(blist_t* self, size_t index);

/** \brief Copies `list->size` bytes from `data` into the list at `index`.
 *
 * \param list blist_t*
 * \param index size_t
 * \param data void*
 * \return void
 */
void blist_set(blist_t* self, size_t index, void* data);

/** \brief Returns a pointer to the element at `index`.
 *
 * \param list blist_t*
 * \param index size_t
 * \return void*
 */
void* blist_get(blist_t* self, size_t index);

/** \brief Copies the element at `index` into `buffer`.
 *
 * \param self blist_t*
 * \param index size_t
 * \param buffer void*
 * \return void
 */
void blist_copyget(blist_t* self, size_t index, void* buffer);

/** \brief Returns a pointer to the first element of the list.
 *
 * \param self blist_t*
 * \return void*
 */
void* blist_begin(blist_t* self);

/** \brief Returns a pointer one past the last element of the list.
 *
 * \param self blist_t*
 * \return void*
 */
void* blist_end(blist_t* self);

#endif // DS_BLIST_H
