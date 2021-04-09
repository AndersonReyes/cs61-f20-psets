#define M61_DISABLE 1
#include "m61.hh"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cassert>
#include <limits.h>
#include <vector>
#include <string.h>
#include <map>

struct metadata_node {
    const char* file;
    long line;
    unsigned long long sz;
};

static std::map<uintptr_t, metadata_node*> metadata;
static unsigned long long ntotal = 0;
static unsigned long long nactive = 0;
static unsigned long long active_size = 0;
static unsigned long long total_size = 0;
static unsigned long long fail_size = 0;
static unsigned long long nfail = 0;
static uintptr_t heap_min = LONG_MAX;
static uintptr_t heap_max = 0;
// TODO: check METASIZE is aligned
// static size_t METASIZE = sizeof(struct metadata_node) - 1;


/// m61_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then m61_malloc must
///    return a unique, newly-allocated pointer value. The allocation
///    request was at location `file`:`line`.

void* m61_malloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings

    void* memory = base_malloc(sz);
    if (sz >= LONG_MAX || !memory) {
        ++nfail;
        fail_size += sz;
        return nullptr;
    }

    uintptr_t uintptr_memory = (uintptr_t) memory;
    ++ntotal;
    ++nactive;
    // Your code here.
    total_size += sz;
    active_size += sz;

    metadata_node* meta = (struct metadata_node*) base_malloc(sizeof(struct metadata_node));
    meta->line = line;
    meta->file = file;
    meta->sz = sz;
    metadata.insert(std::pair<uintptr_t, metadata_node*>(uintptr_memory, meta));

    if (uintptr_memory < heap_min)
        heap_min = uintptr_memory;

    if (uintptr_memory + sz > heap_max)
        heap_max =  uintptr_memory + sz;
    return memory;
}


/// m61_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`, which must have been
///    returned by a previous call to m61_malloc. If `ptr == NULL`,
///    does nothing. The free was called at location `file`:`line`.

void m61_free(void* ptr, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    if (!ptr) return;

    uintptr_t uptr = (uintptr_t) ptr;
    auto meta = metadata.find(uptr);

    if (meta != metadata.end()) {
        --nactive;
        active_size -= meta->second->sz;
        metadata.erase(meta);
        base_free((void*) meta->second);
    } else {
        // TODO: panic because the ptr we trying to free was not allocated by u s
        printf("MEMORY BUG: invalid free of pointer %p, not in heap", ptr);
        exit(-1);
        return;
    }
    base_free(ptr);
}


/// m61_calloc(nmemb, sz, file, line)
///    Return a pointer to newly-allocated dynamic memory big enough to
///    hold an array of `nmemb` elements of `sz` bytes each. If `sz == 0`,
///    then must return a unique, newly-allocated pointer value. Returned
///    memory should be initialized to zero. The allocation request was at
///    location `file`:`line`.

void* m61_calloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test019).
    size_t total = nmemb * sz;
    // detect overflow
    if (nmemb != 0 && (total / nmemb) != sz) {
        ++nfail;
        fail_size += sz;
        return nullptr;
    }

    void* ptr = m61_malloc(total, file, line);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}


/// m61_get_statistics(stats)
///    Store the current memory statistics in `*stats`.

void m61_get_statistics(m61_statistics* stats) {
    // Stub: set all statistics to enormous numbers
    // memset(stats, 255, sizeof(m61_statistics));
	stats->nactive = nactive;
	stats->active_size = active_size;
	stats->total_size = total_size;
	stats->fail_size = fail_size;
    stats->nfail = nfail;
    stats->ntotal = ntotal;
    stats->heap_max = heap_max;
    stats->heap_min = heap_min;
}


/// m61_print_statistics()
///    Print the current memory statistics.

void m61_print_statistics() {
    m61_statistics stats;
    m61_get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}


/// m61_print_leak_report()
///    Print a report of all currently-active allocated blocks of dynamic
///    memory.

void m61_print_leak_report() {
    // Your code here.
}


/// m61_print_heavy_hitter_report()
///    Print a report of heavily-used allocation locations.

void m61_print_heavy_hitter_report() {
    // Your heavy-hitters code here
}
