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
    RWops file;
    file.fromFile(filename, "r");
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


void *Util_MemoryFromFile(const char *fname, size_t * psize)
{
	Sint64 length = 0;
	void *mem = NULL;
    RWops file;

    file.fromFile(fname, "r");
    length = file.getLength();
	mem = Util_malloc(length);
    if (!mem)
        goto out;
    file.read(mem, 1, length);
    file.close();
	*psize = (size_t) length;
	return mem;

out:
    Util_free(mem);
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
// class MemoryBlock
//////////////////////////////////////////////////////////////////////////////////////////



int MemoryBlock::create(const char *fname)
{
    if (m_ptr) {
        DLOG("already created");
        return 1;
    }
    create(Util_GetFileLength(fname));
    readFile(fname);
}


int MemoryBlock::alloc(size_t size)
{
    m_ptr = Util_malloc(size);
    m_size = size;
    return 0;
}


int MemoryBlock::read(RWops &file, size_t size)
{
    file.read(m_ptr, 1, size);
}



int MemoryBlock::create(size_t size)
{
    m_ptr = Util_malloc(size);
    m_size = size;
    return 0;
}


#if 0
int MemoryBlock::readFile(const char *fname)
{
	Sint64 length = 0;
	SDL_RWops *rw = 0;

	rw = SDL_RWFromFile(fname, "r");
	if (!rw) {
        Log("cannot open file: %s", fname);
        goto out;
    }
	SDL_RWseek(rw, 0, RW_SEEK_END);
	length = SDL_RWtell(rw);
	SDL_RWseek(rw, 0, RW_SEEK_SET);

	if (SDL_RWread(rw, m_ptr, 1, length) != length) {
        Log("%s(): something wrong happed.");
    }

	SDL_RWclose(rw);
    return 0;

out:
    if (rw)
        SDL_RWclose(rw);
    return 1;
}
#endif

int MemoryBlock::readFile(const char *fn)
{
    RWops file;
    file.fromFile(fn, "r");
    file.read(m_ptr, 1, file.getLength());
}



void MemoryBlock::destroy()
{
    Util_free(m_ptr);
    m_size = 0;
}


int clamp(int x, int min, int max)
{
	x = x > max ? max : x;
	x = x < min ? min : x;
	return x;
}


#if 0
void Util_free(VoidPtr &p)
{
    if (p) {
        free(p);
        p = NULL;
    }
}
#endif

