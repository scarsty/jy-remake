#ifndef JY_UTIL_H
#define JY_UTIL_H

#include <stddef.h>
#include <stdlib.h>
#include "sdlxx.h"

#ifdef ANDROID
#define Log SDL_Log
#endif

// Log with function name, file name and line number.
// Debug log
#ifndef NDEBUG
#define LOCATION() Log(va("File: %s, Line: %d", __FILE__, __LINE__))
#define DLOG(...) Log("Function: %s, File: %s, Line: %d", \
    __func__, __FILE__, __LINE__); Log(__VA_ARGS__)
#else
#   define DLOG(...) ((void)0)
#endif /* NDEBUG */

template <typename T> inline void Util_free(T*&p)
{
    if (p) {
        free(p);
        p = NULL;
    }
}


void        Log(const char *fmt, ...);
char        *va(const char *format, ...);
void        *Util_MemoryFromFile(const char *filename, size_t *psize);
Sint64      Util_GetFileLength(const char *filename);
void        *Util_malloc(size_t size);
int         clamp(int x, int min, int max);

class MemoryBlock {
public:
    MemoryBlock() : _ptr(0), _size(0) {}
    MemoryBlock(const char *filename);
    MemoryBlock(size_t size);
    ~MemoryBlock();
    operator void*() { return _ptr; }
    size_t getSize() { return _size; }
    void *getPtr() { return _ptr; }
    int readFile(const char *filename);
    int alloc(size_t size);
    void release();
    int read(RWops &rw, size_t size);

private:
    void *_ptr;
    size_t _size;
    DISALLOW_COPY_AND_ASSIGN(MemoryBlock);
};

#endif // JY_UTIL_H

