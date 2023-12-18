#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define VINC_IMPL
#include "vinc-memallocs.h"

int main(void) {
    vinc_arena_allocator_t arena;
    vinc_arena_init(&arena, NULL, 17);

    char *some_str = vinc_arena_alloc(&arena, 32);
    strcpy(some_str, "hello, world!");

    printf("%s", some_str);

    vinc_arena_deinit(&arena);

    return 0;
}
