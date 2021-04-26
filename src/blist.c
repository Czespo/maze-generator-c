
/** blist.c, data-structures-c
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

#include "blist.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>


blist_t* blist_new()
{
    return malloc(sizeof(blist_t));
}

bool blist_init(blist_t* self, size_t capacity, size_t size)
{
    if(capacity)
    {
        self->array = malloc(capacity * size);
        if(self->array == NULL)
        {
            fprintf(stderr, "blist_init() Failed to allocate list array!\n");
            return false;
        }
    }
    else
    {
        self->array = NULL;
    }

    self->length = 0;
    self->capacity = capacity;
    self->size = size;
    self->destructor = NULL;
    return true;
}

blist_t* blist_create(size_t capacity, size_t size)
{
    blist_t* self = blist_new();
    if(self != NULL && !blist_init(self, capacity, size))
    {
        free(self);
        self = NULL;
    }

    return self;
}

void blist_destroy(blist_t* self)
{
    if(self->destructor != NULL)
    {
        for(size_t i = 0; i < self->length; i++)
            self->destructor(blist_get(self, i));
    }

    free(self->array);
    self->array = NULL;

    free(self);
    self = NULL;
}

void blist_extend(blist_t* self)
{
    self->capacity = (self->capacity) ? self->capacity << 1 : 10;
    self->array = realloc(self->array, self->capacity * self->size);
}

void blist_push(blist_t* self, void* data)
{
    if(self->length == self->capacity) blist_extend(self);

    blist_set(self, self->length++, data);
}

void blist_pop(blist_t* self)
{
    if(self->length)
    {
        if(self->destructor != NULL)
            self->destructor(blist_get(self, self->length - 1));

        self->length--;
    }
}

void blist_remove(blist_t* self, size_t index)
{
    char* i = blist_get(self, index);
    if(self->destructor != NULL)
        self->destructor(i);

    // Shift elements to the left.
    for(char* end = blist_get(self, self->length - 1); i != end; i += self->size)
        memcpy(i, i + self->size, self->size);

    self->length--;
}

void blist_set(blist_t* self, size_t index, void* data)
{
    if(index >= self->capacity) blist_extend(self);

    memcpy(blist_get(self, index), data, self->size);
}

void* blist_get(blist_t* self, size_t index)
{
    return self->array + (index * self->size);
}

void blist_copyget(blist_t* self, size_t index, void* buffer)
{
    memcpy(buffer, blist_get(self, index), self->size);
}

void* blist_begin(blist_t* self)
{
    return self->array;
}

void* blist_end(blist_t* self)
{
    return self->array + (self->length * self->size);
}
