#ifndef JY_UTIL_H
#define JY_UTIL_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <exception>
#include <string.h>
#include "sharedptr.hpp"

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

// disable copy ctor and assign operator
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    TypeName& operator=(const TypeName&);


#ifndef BIT
#define BIT(x) (1<<(x))
#endif // BIT

template <typename T> inline void Util_free(T*&p)
{
    if (p) {
        free(p);
        p = NULL;
    }
}

void        Log(const char *fmt, ...);
char        *va(const char *format, ...);
void        *Util_malloc(size_t size);
int         clamp(int x, int min, int max);
const char *Util_GetResourcePath(const char *fname);

//////////////////////////////////////////////////////////////////////////////////////////
// Utils
//////////////////////////////////////////////////////////////////////////////////////////

class Point {
public:
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
};

class Size {
public:
    Uint32 cx;
    Uint32 cy;

    Size(Uint32 cx, Uint32 cy) : cx(cx), cy(cy)
    {
    }

    Size() : cx(0), cy(0)
    {
    }
};


class Rect {
private:
    int _x, _y, _w, _h;
public:
    int x1() const { return _x; }
    int y1() const { return _y; }
    int x2() const { return _x + _w - 1; }
    int y2() const { return _y + _h - 1; }

    Rect(const Point& pt1, const Point& pt2)
    {
        _x = pt1.x;
        _y = pt1.y;
        _w = pt2.x - pt1.x + 1;
        _h = pt2.y - pt1.y + 1;
    }

    Rect(const Point& pt, const Size& sz)
    {
        _x = pt.x;
        _y = pt.y;
        _w = sz.cx;
        _y = sz.cy;
    }
};


//////////////////////////////////////////////////////////////////////////////////////////
// Exception
//////////////////////////////////////////////////////////////////////////////////////////

class SDLException : public std::exception
{
public:
    SDLException(const char *str) throw() {
        strncpy(mszMessage, str, sizeof(mszMessage));
    }
    const char *what() const throw() { return mszMessage; }
private:
    char mszMessage[512];
};

class HardwareException : public SDLException {
public:
    HardwareException(const char *str) throw(): SDLException(str) {}
};

class MemoryException : public SDLException {
public:
    MemoryException(const char *str) throw(): SDLException(str) {}
};

class ResourceException : public SDLException {
public:
    ResourceException(const char *str) throw(): SDLException(str) {}
};

class GameException : public SDLException {
public:
    GameException(const char *str) throw(): SDLException(str) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// SDL_RWops
//////////////////////////////////////////////////////////////////////////////////////////

// RWops 模块 SDL 1.2 和 SDL 2.0 基本上是一样的。
class RWops {
private:
    SDL_RWops *_rw;
public:
    ~RWops() throw();
    bool ready() const { return _rw != NULL; }
    RWops(const char *filename, const char *mode="r");
    RWops(const void *mem, int size);
    RWops(FILE *fp, bool autoClose);
    RWops(void *mem, int size);
    size_t read(void *buf, size_t size, size_t maxnum);
    size_t write(void *buf, size_t size, size_t num);
    Sint64 seek(Sint64 offset, int whence);
    Sint64 tell();
    Sint64 getLength() const;
    size_t writeBe16(Uint16 value) { return SDL_WriteBE16(_rw, value); }
    size_t writeBe32(Uint32 value) { return SDL_WriteBE32(_rw, value); }
    size_t writeBe64(Uint64 value) { return SDL_WriteBE64(_rw, value); }
    size_t writeLe16(Uint16 value) { return SDL_WriteLE16(_rw, value); }
    size_t writeLe32(Uint16 value) { return SDL_WriteLE32(_rw, value); }
    size_t writeLe64(Uint64 value) { return SDL_WriteLE64(_rw, value); }
    Uint16 readBe16() { return SDL_ReadBE16(_rw); }
    Uint32 readBe32() { return SDL_ReadBE32(_rw); }
    Uint32 readBe64() { return SDL_ReadBE64(_rw); }
    Sint16 readLe16() { return SDL_ReadLE16(_rw); }
    Sint32 readLe32() { return SDL_ReadLE32(_rw); }
    Sint64 readLe64() { return SDL_ReadLE64(_rw); }
    operator SDL_RWops*() const { return _rw; }
    SDL_RWops *get() const { return _rw; }
};

//////////////////////////////////////////////////////////////////////////////////////////
// MemoryBlock
//////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////
// class Surface
//////////////////////////////////////////////////////////////////////////////////////////

class Surface {
private:
    SDL_Surface *_surface;
public:
    typedef sharedptr<SDL_Surface> pointer;
    Surface(Uint32 flags, int w, int h, int depth,
            Uint32 Rmas, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    Surface(const char *filename);
    Surface(const Surface& copy);
    Surface();
    ~Surface() throw();
    operator SDL_Surface*() const { return _surface; }
    SDL_Surface *get() const { return _surface; }
    int setColorKey(Uint8 r, Uint8 g, Uint8 b);
    Size getSize() const { return Size(_surface->w, _surface->h); }
    void reset(SDL_Surface *surface);
    pointer loadJbm(RWops &rw); // jy bitmap
    pointer loadPng(const char *filename);
    pointer loadPng(RWops &rw);

};

#endif // JY_UTIL_H

