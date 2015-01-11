// 简单的 SDL C++ 封装

#ifndef JY_SDLXX_H
#define JY_SDLXX_H

#include <SDL.h>
#include <cstring>
#include <cstdio>
#include <memory>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    TypeName& operator=(const TypeName&);

//////////////////////////////////////////////////////////////////////////////////////////
// Utils
//////////////////////////////////////////////////////////////////////////////////////////

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


//////////////////////////////////////////////////////////////////////////////////////////
// Exception
//////////////////////////////////////////////////////////////////////////////////////////

class SDLException {
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

class RWops {
private:
    SDL_RWops *_rw;
public:
    ~RWops() throw();
    bool ready() const { return _rw != NULL; }
    RWops(const char *filename, const char *mode);
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
// Window
//////////////////////////////////////////////////////////////////////////////////////////

class Window {
private:
    SDL_Window *_window;
public:
    Window(const char *title, int x, int y, int w, int h, Uint32 flags);
    ~Window() throw();
    operator SDL_Window*() const { return _window; }
    SDL_Window *get() const { return _window; }
};

//////////////////////////////////////////////////////////////////////////////////////////
// class Renderer
//////////////////////////////////////////////////////////////////////////////////////////

class Renderer {
private:
    SDL_Renderer *_renderer;
public:
    Renderer(SDL_Window *window, int index, Uint32 flags);
    ~Renderer() throw();
    operator SDL_Renderer*() const { return _renderer; }
    SDL_Renderer *get() const { return _renderer; }
};

//////////////////////////////////////////////////////////////////////////////////////////
// class Surface
//////////////////////////////////////////////////////////////////////////////////////////

class Surface {
private:
    SDL_Surface *_surface;
public:
    //typedef std::shared_ptr<Surface> pointer;
    
    Surface(Uint32 flags, int w, int h, int depth,
            Uint32 Rmas, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    Surface(const char *filename);
    Surface(const Surface& copy);
    ~Surface() throw();
#if 0
    int create(Uint32 flags, int w, int h, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    int createFrom(void *ptr, int w, int h, int depth, int pitch, Uint32 rmask, Uint32 gmak, Uint32 bmask, Uint32 amask);
#endif
    operator SDL_Surface*() const { return _surface; }
    SDL_Surface *get() const { return _surface; }
    int setColorKey(Uint8 r, Uint8 g, Uint8 b);
    //pointer convert(SDL_Surface *surf);
    Size getSize() const { return Size(_surface->w, _surface->h); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// class Texture
//////////////////////////////////////////////////////////////////////////////////////////

class Texture {
private:
    SDL_Texture *_texture;
public:
    //typedef std::shared_ptr<Texture> pointer;
    Texture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h);
    Texture(SDL_Renderer *renderer, const char *filename);
    Texture(SDL_Renderer *renderer, Surface& surf);
    ~Texture() throw();
    operator SDL_Texture*() const { return _texture; }
    SDL_Texture *get() const { return _texture; }
};

#endif // JY_SDLXX_H

