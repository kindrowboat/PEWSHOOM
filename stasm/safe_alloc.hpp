// $safe_alloc.hpp 3.0 milbo$ defines so memory allocation is checked

#if !defined(safe_alloc_hpp)
#define safe_alloc_hpp

extern void *_malloc(size_t size);
extern void *_calloc(size_t num, size_t size);

#define calloc(num, size) _calloc((num), (size))
#define malloc(size)      _malloc(size)

#endif // safe_alloc_hpp
