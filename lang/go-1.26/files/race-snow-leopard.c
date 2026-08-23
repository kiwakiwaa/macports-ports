/*
 * The Go race runtime is built on a newer macOS SDK. Its sanitizer support
 * references private direct-system-call entry points introduced after these
 * legacy systems, plus optional dyld-cache and pthread-introspection APIs.
 *
 * Keep this shim limited to the missing ABI. The sanitizer already contains
 * equivalent fallbacks, but the legacy linkers cannot resolve its weak imports
 * when the final executable is produced.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

void *__mmap(void *address, size_t length, int protection, int flags,
             int descriptor, off_t offset)
{
    return mmap(address, length, protection, flags, descriptor, offset);
}

int __munmap(void *address, size_t length)
{
    return munmap(address, length);
}

pid_t __fork(void)
{
    return fork();
}

bool _dyld_get_shared_cache_uuid(unsigned char uuid[16])
{
    (void)uuid;
    return false;
}

const void *_dyld_get_shared_cache_range(size_t *length)
{
    if (length != NULL) {
        *length = 0;
    }
    return NULL;
}

int dyld_shared_cache_iterate_text(const unsigned char uuid[16], void *callback)
{
    (void)uuid;
    (void)callback;
    return 0;
}

typedef void (*pthread_introspection_hook_t)(unsigned int, void *, void *,
                                             size_t);

pthread_introspection_hook_t
pthread_introspection_hook_install(pthread_introspection_hook_t hook)
{
    (void)hook;
    return NULL;
}
