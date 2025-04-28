#include "Allocators.h"

void* HeapAllocator::Allocate(std::size_t allocationSize, std::size_t alignment)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (allocationSize == 0)
    {
        return nullptr;
    }

    // Calculate the correct size based on allocationSize alone
    const std::size_t size = allocationSize;

    auto* ptr = std::malloc(size);

#ifdef TRACY_ENABLE
    TracyAlloc(ptr, size);
#endif

    return ptr;
}

void HeapAllocator::Deallocate(void* ptr)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

#ifdef TRACY_ENABLE
    TracyFree(ptr);
#endif

    std::free(ptr);
}