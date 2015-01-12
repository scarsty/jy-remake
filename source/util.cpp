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

// 返回文件长度，若为0，则文件可能不存在
Sint64 Util_GetFileLength(const char *filename)
{
    RWops file(filename, "r");
    return file.getLength();
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
// class MemoryBlock
//////////////////////////////////////////////////////////////////////////////////////////

MemoryBlock::~MemoryBlock()
{
    Util_free(_ptr);
}

MemoryBlock::MemoryBlock(const char *fname)
{
    Sint64 len = Util_GetFileLength(fname);
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
// utility functions
//////////////////////////////////////////////////////////////////////////////////////////

int clamp(int x, int min, int max)
{
	x = x > max ? max : x;
	x = x < min ? min : x;
	return x;
}
