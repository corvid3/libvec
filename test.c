#include <stdlib.h>

#define LIBV_IMPLEMENTATION
#include "libtest/test.h"

#include "libvec.h"

static const char* create_destroy(void) {
    struct libv vec = libv_init_default(sizeof(int));
    libv_destroy(&vec);
    return NULL;
}

static const char* push(void) {
    struct libv vec = libv_init_default(sizeof(int));

    int data = 4;
    libv_push(&vec, &data);

    if (LIBV_GET(&vec, int, 0) != 4)
        return TESTC_BASIC_ERR;

    libv_destroy(&vec);
    return NULL;
}

static const char* insert(void) {
    struct libv vec = libv_init_default(sizeof(int));

    int v0 = 0, v1 = 1, v2 = 2;
    libv_push(&vec, &v0);
    libv_push(&vec, &v2);
    libv_insert(&vec, &v1, 1);

    if (LIBV_GET(&vec, int, 1) != 1)
        return TESTC_BASIC_ERR;

    libv_destroy(&vec);
    return NULL;
}

static const char* array_append(void) {
    struct libv into = libv_init_default(sizeof(int));
    const int from[] = {
        0, 1, 2, 3, 4,
    };

    libv_append_list(&into, (void*)from, sizeof from / sizeof(int));

    for (size_t i = 0; i < 5; i++)
        if (LIBV_GET(&into, int, i) != (int)i)
            return TESTC_BASIC_ERR;

    libv_destroy(&into);
    return NULL;
}

static const char* vec_insert(void) {
    const int into_init[] = {
        0,
        1,
        4,
        5,
    };

    const int from_init[] = {
        2,
        3,
    };

    struct libv into = libv_init_default(sizeof(int));
    struct libv from = libv_init_default(sizeof(int));

    libv_append_list(&into, (void*)into_init, sizeof(into_init) / sizeof(int));
    libv_append_list(&from, (void*)from_init, sizeof(from_init) / sizeof(int));

    libv_insert_vec(&into, &from, 2);

    for (size_t i = 0; i < 6; i++)
        if (LIBV_GET(&into, int, i) != (int)i)
            return TESTC_BASIC_ERR;

    libv_destroy(&into);
    return NULL;
}

static test_t tests[] = {
    (test_t){
        .ptr = create_destroy,
        .name = "create destroy",
        .desc = "",
    },

    (test_t){
        .ptr = push,
        .name = "push data",
        .desc = "",
    },

    (test_t){
        .ptr = insert,
        .name = "insert in middle",
        .desc = "",
    },

    (test_t){
        .ptr = array_append,
        .name = "append to vec from array",
        .desc = "",
    },

    (test_t){
        .ptr = vec_insert,
        .name = "insert into array from another",
        .desc = "",
    },
};

int main(void) {
    for (size_t i = 0; i < sizeof tests / sizeof(test_t); i++) {
        execute_test(tests[i]);
    }
}
