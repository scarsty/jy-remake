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

#if 0
class SDLException {
public:
    SDLException(const char *str) {
        strncpy(m_message, str, sizeof(m_message));
    }
    const char *getStr() const { return m_message; }
private:
    char m_message[512];
};

class HardwareException : public SDLException {
public:
    HardwareException(const char *str) : SDLException(str) {}
};

class MemoryException : public SDLException {
public:
    MemoryException(const char *str) : SDLException(str) {}
};

class GameException : public SDLException {
public:
    GameException(const char *str) : SDLException(str) {}
};
#endif


//////////////////////////////////////////////////////////////////////////////////////////
// SDL_RWops
//////////////////////////////////////////////////////////////////////////////////////////

class RWops {
private:
    SDL_RWops *m_rwops;
public:
    RWops() : m_rwops(0) {}
    ~RWops() { close(); }


    bool ready() const { return m_rwops != NULL; }

    int fromFile(const char *filename, const char *mode);
    int fromConstMem(const void *mem, int size);
    int fromFp(FILE *fp, bool autoClose);
    int fromMem(void *mem, int size);
    void close();
    size_t read(void *buf, size_t size, size_t maxnum);
    size_t write(void *buf, size_t size, size_t num);
    Sint64 seek(Sint64 offset, int whence);
    Sint64 tell();
    Sint64 getLength() const;
    size_t writeBe16(Uint16 value) { return SDL_WriteBE16(m_rwops, value); }
    size_t writeBe32(Uint32 value) { return SDL_WriteBE32(m_rwops, value); }
    size_t writeBe64(Uint64 value) { return SDL_WriteBE64(m_rwops, value); }
    size_t writeLe16(Uint16 value) { return SDL_WriteLE16(m_rwops, value); }
    size_t writeLe32(Uint16 value) { return SDL_WriteLE32(m_rwops, value); }
    size_t writeLe64(Uint64 value) { return SDL_WriteLE64(m_rwops, value); }
    Uint16 readBe16() { return SDL_ReadBE16(m_rwops); }
    Uint32 readBe32() { return SDL_ReadBE32(m_rwops); }
    Uint32 readBe64() { return SDL_ReadBE64(m_rwops); }
    Sint16 readLe16() { return SDL_ReadLE16(m_rwops); }
    Sint32 readLe32() { return SDL_ReadLE32(m_rwops); }
    Sint64 readLe64() { return SDL_ReadLE64(m_rwops); }

    operator SDL_RWops*() { return m_rwops; }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Window
//////////////////////////////////////////////////////////////////////////////////////////

class Window {
private:
    SDL_Window *m_window;
public:
    Window() : m_window(0) {}
    ~Window() { destroy(); }
    int create(const char *title, int x, int y, int w, int h, Uint32 flags);
    void destroy(); 
    operator SDL_Window*() { return m_window; }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Renderer
//////////////////////////////////////////////////////////////////////////////////////////

class Renderer {
private:
    SDL_Renderer *m_renderer;
public:
    Renderer() : m_renderer(0) {}
    ~Renderer() { destroy(); }
    void destroy();
    int create(SDL_Window *win, int index, Uint32 flags);
    operator SDL_Renderer*() { return m_renderer; }

};

//////////////////////////////////////////////////////////////////////////////////////////
// Surface
//////////////////////////////////////////////////////////////////////////////////////////

class Surface {
private:
    SDL_Surface *m_surface;
public:
    //typedef std::shared_ptr<Surface> pointer;
    
    bool ready() const { return m_surface != NULL; }
    Surface() : m_surface(0) {}
    //Surface(const Surface &copy) throw(HardwareException);
    explicit Surface(SDL_Surface *m_surface);
    ~Surface() { destroy(); }
    int create(Uint32 flags, int w, int h, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    int createFrom(void *ptr, int w, int h, int depth, int pitch, Uint32 rmask, Uint32 gmak, Uint32 bmask, Uint32 amask);
    void destroy();
    operator SDL_Surface*() { return m_surface; }
    int loadBmp(const char *filename);
    int setColorKey(Uint8 r, Uint8 g, Uint8 b);
    //pointer convert(SDL_Surface *surf);
    Size getSize() const { return Size(m_surface->w, m_surface->h); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Texture
//////////////////////////////////////////////////////////////////////////////////////////

class Texture {
private:
    SDL_Texture *m_texture;
public:
    //typedef std::shared_ptr<Texture> pointer;
    Texture() : m_texture(0) {}
    Texture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h)
    {
        if (m_texture == NULL) {
            create(renderer, format, access, w, h);
        }
    }
    ~Texture() { destroy(); }
    void destroy();
    int create(SDL_Renderer *renderer, Uint32 format, int access, int w, int h);
    operator SDL_Texture*() { return m_texture; }
    int loadBmp(SDL_Renderer *renderer, const char *filename);
    int fromSurface(SDL_Renderer *renderer, SDL_Surface *surface);
};

#endif // JY_SDLXX_H

