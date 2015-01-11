/**
 * util.cpp - utility functions make life easier
 */

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <cstring>
#include "util.h"

#if !defined (ANDROID)
static FILE *logfile = NULL;
void LogExit(void)
{
	fclose(logfile);
}

void Log(const char *fmt, ...)
{
	static char logbuf[512];
	va_list arglist;

	if (!logfile) {
		char buf[256];
		strcpy(buf, "log.txt");
		logfile = fopen(buf, "w");
		if (!logfile)
			exit(1);
		atexit(LogExit);
	}
	va_start(arglist, fmt);
	SDL_vsnprintf(logbuf, sizeof(logbuf), fmt, arglist);
	va_end(arglist);
	fputs(logbuf, logfile);
	fputs("\n", logfile);
	fflush(logfile);
}
#endif /* ANDROID */

char *va(const char *format, ...)
{
	static char string[256];
	va_list argptr;

	va_start(argptr, format);
	SDL_vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	return string;
}

void *Util_malloc(size_t size)
{
    void *ret = NULL;
    ret = malloc(size);
    if (!ret) {
        Log("out of memory");
        exit(-1);
    }
    return ret;
}


//////////////////////////////////////////////////////////////////////////////////////////
// utility functions
//////////////////////////////////////////////////////////////////////////////////////////

int clamp(int x, int min, int max)
{
	x = x > max ? max : x;
	x = x < min ? min : x;
	return x;
}

//////////////////////////////////////////////////////////////////////////////////////////
// class RWops
//////////////////////////////////////////////////////////////////////////////////////////

RWops::RWops(const char *filename, const char *mode)
{
    _rw = SDL_RWFromFile(filename, mode);
    if (!_rw) {
        throw HardwareException("SDL_RWFromFile() failed.");
    }
}

RWops::RWops(const void *mem, int size)
{
    _rw = SDL_RWFromConstMem(mem, size);
    if (!_rw) {
        throw HardwareException("SDL_RWFromFile() failed.");
    }
}

RWops::RWops(void *mem, int size)
{
    _rw = SDL_RWFromMem(mem, size);
    if (!_rw) {
        throw HardwareException("SDL_RWFromFile() failed.");
    }
}


RWops::RWops(FILE *fp, bool autoClose)
{
    _rw = SDL_RWFromFP(fp, autoClose? SDL_TRUE : SDL_FALSE);
    if (!_rw) {
        throw HardwareException("SDL_RWFromFile() failed.");
    }
}


RWops::~RWops() throw()
{
    SDL_RWclose(_rw);
}

size_t RWops::read(void *buf, size_t size, size_t maxnum)
{
    return SDL_RWread(_rw, buf, size, maxnum);
}

size_t RWops::write(void *buf, size_t size, size_t num)
{
    return SDL_RWwrite(_rw, buf, size, num);
}

Sint64 RWops::seek(Sint64 offset, int whence)
{
    return SDL_RWseek(_rw, offset, whence);
}

Sint64 RWops::tell()
{
    return SDL_RWtell(_rw);
}

Sint64 RWops::getLength() const
{
    Sint64 length = 0;
    Sint64 pos = 0;
    pos  = SDL_RWtell(_rw);
    SDL_RWseek(_rw, 0, RW_SEEK_END);
    length = SDL_RWtell(_rw);
    SDL_RWseek(_rw, pos, RW_SEEK_SET);
    return length;
}


//////////////////////////////////////////////////////////////////////////////////////////
// class MemoryBlock
//////////////////////////////////////////////////////////////////////////////////////////

MemoryBlock::~MemoryBlock()
{
    Util_free(_ptr);
}

MemoryBlock::MemoryBlock(const char *fname)
{
    Sint64 len = RWops(fname).getLength();
    alloc(len);
    try {
        readFile(fname);
    }
    catch(...) {
        release();
        throw;
    }
}

MemoryBlock::MemoryBlock(size_t size)
{
    alloc(size);
}

int MemoryBlock::alloc(size_t size)
{
    _ptr = Util_malloc(size);
    _size = size;
    return 0;
}

void MemoryBlock::release()
{
    Util_free(_ptr);
    _size = 0;
}

int MemoryBlock::read(RWops &file, size_t size)
{
    file.read(_ptr, 1, size);
}

int MemoryBlock::readFile(const char *fn)
{
    RWops file(fn, "r");
    file.read(_ptr, 1, file.getLength());
}

//////////////////////////////////////////////////////////////////////////////////////////
// class Surface
//////////////////////////////////////////////////////////////////////////////////////////

Surface::Surface()
    : _surface(NULL)
{
}

Surface::Surface(Uint32 flags, int w, int h, int depth,
        Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    _surface = SDL_CreateRGBSurface(flags, w, h, depth, Rmask, Gmask, Bmask, Amask);
    if (!_surface) {
        throw HardwareException("SDL_CreateRGBSurface() failed.");
    }
}

Surface::Surface(const char *filename)
{
    _surface = SDL_LoadBMP(filename);
    if (!_surface) {
        throw HardwareException("SDL_LoadBMP() failed.");
    }
}

Surface::Surface(const Surface &copy)
{
    _surface = SDL_ConvertSurface(copy._surface, copy._surface->format, 
            copy._surface->flags);
    if (!_surface) {
        throw HardwareException("SDL_ConvertSurface() failed.");
    }
}

Surface::~Surface() throw()
{
    SDL_FreeSurface(_surface);
}

int Surface::setColorKey(Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 key = SDL_MapRGB(_surface->format, r, g, b);
    SDL_SetColorKey(_surface, SDL_TRUE, key);
}

void Surface::reset(SDL_Surface *surface)
{
    if (_surface) {
        SDL_FreeSurface(_surface);
        _surface = NULL;
    }
    _surface = surface;
}

Surface::pointer Surface::loadPng(RWops &rw)
{
}

Surface::pointer Surface::loadPng(const char *filename)
{
}

