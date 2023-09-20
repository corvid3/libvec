
#pragma once

/*

--- LIBV ---
a single-header vector library
written in pure C

CREDITS: korvo
    (korvonesto.com)

*/

#include <stddef.h>
#include <stdint.h>

struct libv_allocator_set {
    void* (*alloc)(size_t size);
    void (*free)(void* ptr);
    void* (*realloc)(void* ptr, size_t new_size);
};

struct libv {
    uint8_t* ptr;
    size_t len;
    size_t cap;
    size_t elem_size;

    struct libv_allocator_set allocator_set;
};

#undef LIBV_EXPORT
#ifdef LIBV_INLINE
#define LIBV_EXPORT static inline
#else
#define LIBV_EXPORT extern
#endif

#ifndef LIBV_DEFAULT_CAP
#define LIBV_DEFAULT_CAP (sizeof(size_t) * 8)
#endif

#ifndef LIBV_DEFAULT_GROWTH
#define LIBV_DEFAULT_GROWTH 2
#endif

/* -- CONSTRUCTORS -- */

// initialize a vector, using the standard library allocator
LIBV_EXPORT struct libv libv_init_default(size_t member_size);

// initialize a vector, using a custom set of allocator functions
LIBV_EXPORT struct libv libv_init_alloc(struct libv_allocator_set set,
                                        size_t member_size);

// destroy an allocator, and free the memory held by it
// does nothing when called on an already destroyed vector
LIBV_EXPORT void libv_destroy(struct libv* vector);

/* -- MEMORY MANAGEMENT -- */

// repeatedly grows the vectors capacity by multiplying its capacity by
// LIBV_DEFAULT_GROWTH, until the vectors capacity >= cap
LIBV_EXPORT void libv_grow(struct libv* vector, size_t cap);

// resize the capacity of the vector, destructively
LIBV_EXPORT void libv_resize(struct libv* vector, size_t size);

// resize the capacity of the vector to be at least size
// does not grow the vector past size unless it is already greater than size
LIBV_EXPORT void libv_reserve(struct libv* vector, size_t size);

/* -- MUTATING FUNCTIONS -- */

// push a value onto the end of a vector
LIBV_EXPORT void libv_push(struct libv* vector, void* data);

// pop a value into a location of memory
LIBV_EXPORT void libv_pop(struct libv* vector, void* into);

LIBV_EXPORT void libv_append(struct libv* into, struct libv* from);
LIBV_EXPORT void libv_append_list(struct libv* into, void* from, size_t len);

LIBV_EXPORT void libv_insert_vec(struct libv* into,
                                 struct libv* from,
                                 size_t whence);

// insert a value into a vector, preserving order,
// at a specific index specified by whence
LIBV_EXPORT void libv_insert(struct libv* vector, void* from, size_t whence);

// insert a value into a vector, not preserving order,
// at a specifid index specified by whence
// faster than regular insert
LIBV_EXPORT void libv_insert_fast(struct libv* vector,
                                  void* from,
                                  size_t whence);

// TODO: unimplemented
// LIBV_EXPORT void libv_insert_vec(struct libv *into, struct libv *from,
//                                    size_t whence);

// remove a value at a specified location, preserving order
LIBV_EXPORT void libv_remove(struct libv* vector, size_t whence);

// remove a value at a specified location, not preserving order
// faster than regular remove
LIBV_EXPORT void libv_remove_fast(struct libv* vector, size_t whence);

/* -- HELPER MACROS -- */

#define LIBV_GET(vector__, ty__, idx__) ((ty__*)(vector__)->ptr)[idx__]
#define LIBV_GET_BYTE_INDEX_OF(vector__, idx__) ((vector__)->elem_size * idx__)

#ifdef LIBV_IMPLEMENTATION

#include <stdlib.h>

static const struct libv_allocator_set libv_SG_default_allocator_set = {
    .alloc = malloc,
    .realloc = realloc,
    .free = free,
};

// TODO: optimize this
static inline void libv_memcpy(uint8_t* to, uint8_t* from, size_t size) {
    for (size_t i = 0; i < size; i++)
        to[i] = from[i];
}

/* -- CONSTRUCTORS -- */

LIBV_EXPORT struct libv libv_init_default(size_t size) {
    return (struct libv){
        .ptr = malloc(LIBV_DEFAULT_CAP * size),
        .cap = LIBV_DEFAULT_CAP,
        .len = 0,
        .elem_size = size,
        .allocator_set = libv_SG_default_allocator_set,
    };
}

LIBV_EXPORT struct libv libv_init_alloc(struct libv_allocator_set set,
                                        size_t member_size) {
    return (struct libv){
        .ptr = set.alloc(LIBV_DEFAULT_CAP * member_size),
        .cap = LIBV_DEFAULT_CAP,
        .len = 0,
        .elem_size = member_size,
        .allocator_set = set,
    };
}

LIBV_EXPORT void libv_destroy(struct libv* vec) {
    if (vec->ptr) {
        vec->allocator_set.free(vec->ptr);
        vec->ptr = 0;
        vec->len = 0;
        vec->cap = 0;
        vec->elem_size = 0;
    }
}

/* -- MEMORY MANAGEMENT -- */

LIBV_EXPORT void libv_grow(struct libv* vector, size_t cap) {
    while (vector->cap < cap)
        vector->cap *= LIBV_DEFAULT_GROWTH;
    vector->ptr = vector->allocator_set.realloc(
        vector->ptr, vector->cap * vector->elem_size);
}

LIBV_EXPORT void libv_resize(struct libv* vector, size_t size) {
    vector->cap = size;
    vector->ptr = vector->allocator_set.realloc(vector->ptr, size);
}

LIBV_EXPORT void libv_reserve(struct libv* vector, size_t size) {
    if (vector->cap >= size)
        return;

    vector->cap = size;
    vector->ptr =
        vector->allocator_set.realloc(vector->ptr, size * vector->elem_size);
}

/* -- MUTATING FUNCTIONS -- */

LIBV_EXPORT void libv_push(struct libv* vector, void* data) {
    libv_grow(vector, vector->len + 1);
    libv_memcpy(&vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, vector->len)], data,
                vector->elem_size);
    vector->len += 1;
}

LIBV_EXPORT void libv_pop(struct libv* vector, void* into) {
    libv_memcpy(into,
                &vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, vector->len - 1)],
                vector->elem_size);
    vector->len -= 1;
}

LIBV_EXPORT void libv_append(struct libv* into, struct libv* from) {
    libv_grow(into, into->cap + from->cap);

    for (size_t i = 0; i < from->len; i++) {
        libv_memcpy(into->ptr + (i + into->len) * into->elem_size,
                    from->ptr + i * into->elem_size, into->elem_size);
    }

    into->len += from->len;
}

LIBV_EXPORT void libv_append_list(struct libv* into, void* from, size_t len) {
    libv_grow(into, into->len + len);

    for (size_t i = 0; i < len; i++) {
        libv_memcpy(into->ptr + (i + into->len) * into->elem_size,
                    &((uint8_t*)from)[i * into->elem_size], into->elem_size);
    }

    into->len += len;
}

LIBV_EXPORT void libv_insert_vec(struct libv* into,
                                 struct libv* from,
                                 size_t whence) {
    const size_t elem_size = into->elem_size;

    libv_grow(into, into->len + from->len);

    for (size_t i = into->len - 1; i > whence; i--) {
        void* from_ptr = into->ptr + i * elem_size;
        void* into_ptr = into->ptr + (i + from->len) * elem_size;
        libv_memcpy(into_ptr, from_ptr, elem_size);
    }

    for (size_t i = 0; i < from->len; i++) {
        void* from_ptr = from->ptr + i * elem_size;
        void* into_ptr = into->ptr + (i + whence) * elem_size;
        libv_memcpy(into_ptr, from_ptr, elem_size);
    }
    into->len += from->len;
}

// must preserve order
LIBV_EXPORT void libv_insert(struct libv* vector, void* from, size_t whence) {
    libv_grow(vector, vector->len + 1);

    // move all elements in front of whence one forward
    for (size_t i = vector->len; i > whence; i--) {
        const size_t byte_idx_here = LIBV_GET_BYTE_INDEX_OF(vector, i);
        const size_t byte_idx_before = LIBV_GET_BYTE_INDEX_OF(vector, i - 1);

        libv_memcpy(&vector->ptr[byte_idx_here], &vector->ptr[byte_idx_before],
                    vector->elem_size);
    }

    // now move from into idx whence
    libv_memcpy(&vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, whence)], from,
                vector->elem_size);

    vector->len += 1;
}

LIBV_EXPORT void libv_insert_fast(struct libv* vector,
                                  void* from,
                                  size_t whence) {
    libv_grow(vector, vector->len + 1);

    libv_memcpy(&vector->ptr[vector->len],
                &vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, whence)],
                vector->elem_size);
    libv_memcpy(&vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, whence)], from,
                vector->elem_size);
}

// must preserve order
LIBV_EXPORT void libv_remove(struct libv* vector, size_t whence) {
    for (size_t i = whence; i < vector->len; i++)
        libv_memcpy(&vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, i)],
                    &vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, i + 1)],
                    vector->elem_size);
    vector->len -= 1;
}

LIBV_EXPORT void libv_remove_fast(struct libv* vector, size_t whence) {
    libv_memcpy(&vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, whence)],
                &vector->ptr[LIBV_GET_BYTE_INDEX_OF(vector, vector->len - 1)],
                vector->elem_size);
    vector->len -= 1;
}

#endif
