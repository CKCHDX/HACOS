#include "memory.h"

void* LinearAllocator::s_base = nullptr;
void* LinearAllocator::s_current = nullptr;
size_t LinearAllocator::s_total_size = 0;
size_t LinearAllocator::s_allocated = 0;

void LinearAllocator::init(void* base, size_t size) {
    s_base = base;
    s_current = base;
    s_total_size = size;
    s_allocated = 0;
}

void* LinearAllocator::allocate(size_t size) {
    if (!s_base) return nullptr;
    if (s_allocated + size > s_total_size) return nullptr;
    
    void* ptr = s_current;
    s_current = (void*)((uint8_t*)s_current + size);
    s_allocated += size;
    
    return ptr;
}

void LinearAllocator::deallocate(void* ptr) {
    // Linear allocator doesn't free individual blocks
    (void)ptr;
}

// Global operator new/delete for freestanding C++
void* operator new(size_t size) {
    return LinearAllocator::allocate(size);
}

void* operator new[](size_t size) {
    return LinearAllocator::allocate(size);
}

void operator delete(void* ptr, size_t) {
    LinearAllocator::deallocate(ptr);
}

void operator delete[](void* ptr, size_t) {
    LinearAllocator::deallocate(ptr);
}

void operator delete(void* ptr) noexcept {
    LinearAllocator::deallocate(ptr);
}

void operator delete[](void* ptr) noexcept {
    LinearAllocator::deallocate(ptr);
}
