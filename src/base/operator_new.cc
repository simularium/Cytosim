// Cytosim was created by Francois Nedelec. Copyright 2019 Cambridge University

// replace global new & delete operators to control memory alignment

#include <new>
#include <cstdio>
#include <cstdlib>

void* operator new(std::size_t size)
{
    void * ptr = nullptr;
#if ( 1 )
    constexpr std::size_t sup = 1 << 30;
    if ( size > sup )
    {
        std::printf("Error: excessive memory requested %5zu\n", size);
        throw std::bad_alloc();
    }
#endif
#if ( 1 )
    // get memory aligned to 32 bytes
    if ( posix_memalign(&ptr, 32, size) )
        throw std::bad_alloc();
#else
    // system's default (unaligned) memory
    ptr = std::malloc(size);
#endif
    if ( ptr == nullptr )
        throw std::bad_alloc();
    //std::printf("Cytosim new %5zu %p\n", size, ptr);
    return ptr;
}


void operator delete(void * ptr) throw()
{
    //std::printf("Cytosim delete    %p\n", ptr);
    std::free(ptr);
}


/*
void* operator new[](std::size_t s) throw(std::bad_alloc)
{
    std::printf("Cytosim new[] %5zu\n", s);
    return ::operator new(s);
}

void operator delete[](void *ptr) throw()
{
    std::printf("Cytosim delete[]    %p\n", ptr);
    ::operator delete(ptr);
}
*/
