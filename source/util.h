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
#   define DLOG(...) Log("Function: %s, File: %s, Line: %d", \
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
    MemoryBlock() : m_ptr(0), m_size(0) {}
    ~MemoryBlock() { destroy(); }
    operator void*() { return m_ptr; }
    void *ptr() { return m_ptr; }
    size_t getSize() { return m_size; }
    size_t size() const { return m_size; }
    //int fromFile(const char *filename);
    int readFile(const char *filename);
    int create(const char *filename);
    int create(size_t size);
    void destroy();

    int alloc(size_t size);
    int read(RWops &rw, size_t size);

private:
    void *m_ptr;
    size_t m_size;
    DISALLOW_COPY_AND_ASSIGN(MemoryBlock);
};

#endif // JY_UTIL_H

