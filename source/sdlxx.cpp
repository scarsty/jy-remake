// 简单的 SDL C++ 封装


#include "sdlxx.h"
#include "util.h"


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
// class Window
//////////////////////////////////////////////////////////////////////////////////////////

Window::Window(const char *title, int x, int y, int w, int h, Uint32 flags)
{
    _window = SDL_CreateWindow(title, x, y, w, h, flags);
    if (!_window) {
        throw HardwareException("SDL_CreateWindow() failed.");
    }
}

Window::~Window() throw()
{
    SDL_DestroyWindow(_window);
}

//////////////////////////////////////////////////////////////////////////////////////////
// class Renderer
//////////////////////////////////////////////////////////////////////////////////////////

Renderer::Renderer(SDL_Window *window, int index, Uint32 flags)
{
    _renderer = SDL_CreateRenderer(window, index, flags);
    if (!_renderer) {
        DLOG("error :%s", SDL_GetError());
        throw HardwareException("SDL_CreateRenderer() failed.");
    }
}

Renderer::~Renderer() throw()
{
    SDL_DestroyRenderer(_renderer);
}

//////////////////////////////////////////////////////////////////////////////////////////
// class Surface
//////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////
// class Texture
//////////////////////////////////////////////////////////////////////////////////////////

Texture::Texture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h)
{
    _texture = SDL_CreateTexture(renderer, format, access, w, h);
}

Texture::Texture(SDL_Renderer *renderer, const char *filename)
{
    Surface surf(filename);
    _texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (!_texture) {
        throw HardwareException("SDL_CreateTextureFromSurface() failed.");
    }
}

Texture::Texture(SDL_Renderer *renderer, Surface& surf)
{
    _texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (!_texture) {
        throw HardwareException("SDL_CreateTextureFromSurface() failed.");
    }
}

Texture::~Texture() throw()
{
    SDL_DestroyTexture(_texture);
}

