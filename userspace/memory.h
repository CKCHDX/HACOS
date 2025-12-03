#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>
#include <cstdint>

// Simple linear allocator for freestanding environment
class LinearAllocator {
public:
    static void init(void* base, size_t size);
    static void* allocate(size_t size);
    static void deallocate(void* ptr);
    
private:
    static void* s_base;
    static void* s_current;
    static size_t s_total_size;
    static size_t s_allocated;
};

#endif
