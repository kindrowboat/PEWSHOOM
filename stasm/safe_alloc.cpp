// $safe_alloc.cpp 3.0 milbo$ defines so memory allocation is checked
// TODO this won't work for failed alloc calls in libraries

#include <stasm.hpp>

void *_malloc (size_t size)
{
#undef malloc
void *p = malloc(size);
if (!p)
    {
    lprintf("\nOut of memory allocating %g megabytes\n",
            double(size) / (1024 * 1024));
    exit(1);
    }
return p;
}

void *_calloc (size_t num, size_t size)
{
#undef calloc
void *p = calloc(num, size);
if (!p)
    {
    lprintf("\nOut of memory allocating %g megabytes\n",
            double(num * size) / (1024 * 1024));
    exit(1);
    }
return p;
}
