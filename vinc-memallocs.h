/** vinc-memalloc - v0.0.1 - public domain custon memory allocators' loader
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

#if defined(VINC_MEMALLOCS_IMPL) || defined(VINC_IMPL)

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

#endif // VINC_MEMALLOCS_IMPL
#endif // VINC_MEMALLOCS_H

/**
 * Changle Log
 *
 * 12/18/2023 - project started
 *            - added `vinc_allocator_t` inteface
 *            - added __vinc_global_alloc constant
 */

/**
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
