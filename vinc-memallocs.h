/** vinc-memalloc - v0.0.1 - public domain custom memory allocators in pure c
 *
 * LICENSE
 *
 *   See end of file for license information.
 *
 * TODO:
 * - [X] Unified Allocator Inteface
 * - [X] Allow user to change the global allocator by providing definition for
 *       functions:
 *       	void *__std_alloc_func(vinc_allocator_t*, size_t mem);
 *       	void *__std_free_func(vinc_allocator_t*, void *ptr);
 *       	void *__std_realloc_func(vinc_allocator_t*, void *ptr,
 * 					 size_t mem);
 * - [ ] Arena Allocator
 * - [ ] Fixed Buffer Allocator
 * - [ ] Memory Pool Allocator
 * - [ ] Logging Allocator
 *
 *   HOW TO USE
 *
 * To get this library working you need to define either VINC_MEMALLOCS_IMPL or
 * VINC_IMPL in *one* c source file. Like this:
 *	...
 *   -> #define VINC_MEMALLOCS_IMPL
 *      #include <vinc_memallocs.h>
 * 	...
 *
 * If you want to redefine global allocator, used by default (malloc), do this:
 * 	...
 *   -> #define VINC_PROVIDE_CUSTOM_GLOBAL_ALLOCATOR
 *      #include <vinc_memallocs.h>
 * 	...
 *	void *__std_alloc_func(vinc_allocator_t* _, size_t size) { ... }
 *	void __std_free_func(vinc_allocator_t* _, void *ptr) { ... }
 *	void *__std_realloc_func(vinc_allocator_t* _, void *ptr, size_t size)
 * 	{...}
 * 	...
 *
 *   SOME NOTES:
 *
 * - Naming Conventions
 *
 * Yes, I am using postfix `_t` for types defined by typedef. I know that it's'
 * bad, It's a matter of personal preference, and you can either use
 * `struct ...` type variation, just find and remove them in code. I justify
 * this by  using `vinc_` prefix.
 *
 * Prefix `__vinc` is used for function and types that are not supposed to be
 * called outsize the library header;
 *
 *   IDEAS FOR LATER:
 *
 * - Get rid of using libc functions in code. Redefine all the needed macros...
 *   Not sure, i really want it for now. But might be useful for some people on
 *   embedded systems
 */

#ifndef VINC_MEMALLOCS_H
#define VINC_MEMALLOCS_H

#include <stdlib.h>

struct vinc_allocator;
typedef void *(*vinc_alloc_func_t)(struct vinc_allocator* allocator,
								   size_t memory);
typedef void (*vinc_free_func_t)(struct vinc_allocator* allocator,
								 void *ptr);
typedef void *(*vinc_realloc_func_t)(struct vinc_allocator* allocator,
									 void *ptr, size_t size);

typedef struct vinc_allocator {
    /**
     * `parent` refers to a parent allocator, used by custom allocators in this
     * module.
     */
    struct vinc_allocator *parent;
    vinc_alloc_func_t alloc;
    vinc_free_func_t free;
    vinc_realloc_func_t realloc;
} vinc_allocator_t;


void *__std_alloc_func(vinc_allocator_t* _, size_t size);
void __std_free_func(vinc_allocator_t* _, void *ptr);
void *__std_realloc_func(vinc_allocator_t* _, void *ptr, size_t size);

/**
 * Definition of the global allocator interface.
 *
 * Serves as the basic memory allocator to be used, when user doesn't provide
 * local parant allocator for custome memory alloctors. By default, it uses
 * pointers to default libc `malloc`, 'free' and 'realloc'. But can be redined
 * by user this way:
 *
 * 	#define VINC_MEMALLOCS_IMPL
 *   -> #define VINC_MEMALLOCS_CUSTOM_GLOBAL_ALLOCATOR
 * 	#include <vinc_memallocs.h>
 */
const vinc_allocator_t __vinc_global_alloc = {
    .parent = NULL,
    .alloc = __std_alloc_func,
    .free = __std_free_func,
    .realloc = __std_realloc_func
};

/**
 * Declaration of custom arena allocator implemented as a linked-list. It uses
 * `struct __vinc_arena_region` as it's nodes.
 *
 * TODO: Complete the docs
 */
struct __vinc_arena_region {
    struct __vinc_arena_region*  next;
    size_t size;
    void* rawmem;
    void* pointer;
};

typedef struct vinc_arena_allocator {
    const vinc_allocator_t* parent;
    size_t min_region_size;
    struct __vinc_arena_region *root;
} vinc_arena_allocator_t;


void vinc_arena_init(vinc_arena_allocator_t* arena, vinc_allocator_t *parent,
					 size_t min_region_size);
void vinc_arena_deinit(vinc_arena_allocator_t* arena);

void *vinc_arena_alloc(vinc_arena_allocator_t* arena, size_t size);
void vinc_arena_free(vinc_arena_allocator_t* arena, void *ptr); // No-Op
void *vinc_arena_realloc(vinc_arena_allocator_t* arena, void *ptr, size_t size);

////////////////////////////////////////////////////////////////////////////////
/// IMPLEMENTATION PART

#define __VINC_MAX(A, B) \
    (A) > (B) ? (A) : (B)
#define __VINC_MIN(A, B) \
    (A) < (B) ? (A) : (B)

/**
 * Following code is implenation that is included into *one* source file that
 * should #define either VINC_MEMALLOCS_IMPL or VINC_IMPL.
 */
#if defined(VINC_MEMALLOCS_IMPL) || defined(VINC_IMPL)

/**
 * Providing global allocator as default libc's malloc if user hasn't provided
 * his own definitions.
 */
#ifndef VINC_PROVIDE_CUSTOM_GLOBAL_ALLOCATOR
#include <malloc.h>

void *__std_alloc_func(vinc_allocator_t* _, size_t size) {
    return malloc(size);
}
void __std_free_func(vinc_allocator_t* _, void *ptr) {
    free(ptr);
}
void *__std_realloc_func(vinc_allocator_t* _, void *ptr, size_t size) {
    return realloc(ptr, size);
}
#endif // VINC_MEMALLOCS_CUSTOM_GLOBAL_ALLOCATOR


/**
 * Implementation of `vinc_arena_allocator_t` methods.
 *
 * TODO: Write some details about this implementation
 *
 * NOTE: This implementation is expecting you to use it in the corect order, as
 * 	 there's not NULL pointer checks, in order to provide max perfomance
 * 	 implementation
 * 	 TODO: Provide examples of corect `vinc_arena_allocator_t`'s usage order
 *
 * TODO: Rewrite this crap later, when enshure that it at least works
 */

static inline void *__vinc_allocate_region(vinc_arena_allocator_t* arena,
					   size_t size) {
    struct __vinc_arena_region* region = arena->parent->alloc(
		(vinc_allocator_t *) &arena->parent,
		sizeof(region[0])
    );
    region->rawmem = arena->parent->alloc(
		(vinc_allocator_t *) &arena->parent,
		size
    );
	region->pointer = region->rawmem;
	region->size = size;

    return region;
}

void* vinc_arena_region_alloc(
    vinc_arena_allocator_t *arena,
    size_t size
){
    struct __vinc_arena_region *region = arena->root;
    if (!region) {
		region = __vinc_allocate_region(
			arena, __VINC_MAX(size, arena->min_region_size)
		);
		arena->root = region;
		return region->pointer;
    }

    while (!region->next)
		region = region->next;

    if (region->size - (region->rawmem - region->pointer) < size) {
		region->next = __vinc_allocate_region(
			arena, __VINC_MAX(size, arena->min_region_size)
		);
		return region->next->pointer;
    }

    void* ptr = region->pointer;
    region->pointer = (void*)((size_t)region->pointer + size);

    return ptr;
}

void vinc_arena_init(vinc_arena_allocator_t* arena, vinc_allocator_t *parent,
					 size_t min_region_size) {
    arena->min_region_size = min_region_size;
    if (parent)
		arena->parent = parent;
    else
		arena->parent = &__vinc_global_alloc;
}
void vinc_arena_deinit(vinc_arena_allocator_t* arena) {

}

void *vinc_arena_alloc(vinc_arena_allocator_t* arena, size_t size) {
    return NULL; // FIXME
}

void *vinc_arena_realloc(vinc_arena_allocator_t* arena, void *ptr,
						 size_t size) {
    return NULL; // FIXME
}

void vinc_arena_free(vinc_arena_allocator_t* arena, void *ptr) {} // No-Op

#endif // VINC_MEMALLOCS_IMPL
#endif // VINC_MEMALLOCS_H

/**
 * CHANGE LOG
 *
 * 12/18/2023 - project started
 *            - added `vinc_allocator_t` inteface
 *            - added __vinc_global_alloc constant
 */

/**
 *  LICENSE
 *
 *  This is free and unencumbered software released into the public domain.
 *
 *  Anyone is free to copy, modify, publish, use, compile, sell, or
 *  distribute this software, either in source code form or as a compiled
 *  binary, for any purpose, commercial or non-commercial, and by any
 *  means.
 *
 *  In jurisdictions that recognize copyright laws, the author or authors
 *  of this software dedicate any and all copyright interest in the
 *  software to the public domain. We make this dedication for the benefit
 *  of the public at large and to the detriment of our heirs and
 *  successors. We intend this dedication to be an overt act of
 *  relinquishment in perpetuity of all present and future rights to this
 *  software under copyright law.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For more information, please refer to <https://unlicense.org>
 */
