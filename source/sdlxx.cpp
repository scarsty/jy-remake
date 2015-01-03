// 简单的 SDL C++ 封装


#include "sdlxx.h"

//////////////////////////////////////////////////////////////////////////////////////////
// RWops
//////////////////////////////////////////////////////////////////////////////////////////

int RWops::fromFile(const char *file, const char *mode)
{
    m_rwops = SDL_RWFromFile(file, mode);
    return m_rwops? 0 : 1;
}

int RWops::fromConstMem(const void *mem, int size)
{
    m_rwops = SDL_RWFromConstMem(mem, size);
    return m_rwops? 0 : 1;
}

int RWops::fromMem(void *mem, int size)
{
    m_rwops = SDL_RWFromMem(mem, size);
    return m_rwops? 0 : 1;
}

int RWops::fromFp(FILE *fp, bool autoClose)
{
    m_rwops = SDL_RWFromFP(fp, autoClose? SDL_TRUE : SDL_FALSE);
    return m_rwops? 0 : 1;
}


void RWops::close()
{
    if (m_rwops) {
        SDL_RWclose(m_rwops);
        m_rwops = 0;
    }
}

size_t RWops::read(void *buf, size_t size, size_t maxnum)
{
    return SDL_RWread(m_rwops, buf, size, maxnum);
}

size_t RWops::write(void *buf, size_t size, size_t num)
{
    return SDL_RWwrite(m_rwops, buf, size, num);
}

Sint64 RWops::seek(Sint64 offset, int whence)
{
    return SDL_RWseek(m_rwops, offset, whence);
}

Sint64 RWops::tell()
{
    return SDL_RWtell(m_rwops);
}

Sint64 RWops::getLength() const
{
    Sint64 length = 0;
    Sint64 pos = 0;
    pos  = SDL_RWtell(m_rwops);
    SDL_RWseek(m_rwops, 0, RW_SEEK_END);
    length = SDL_RWtell(m_rwops);
    SDL_RWseek(m_rwops, pos, RW_SEEK_SET);
    return length;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Window
//////////////////////////////////////////////////////////////////////////////////////////

int Window::create(const char *title, int x, int y, int w, int h, Uint32 flags)
{
    destroy();
    m_window = SDL_CreateWindow(title, x, y, w, h, flags);
    return m_window? 0 : 1;
}

void Window::destroy()
{
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Renderer
//////////////////////////////////////////////////////////////////////////////////////////

int Renderer::create(SDL_Window *win, int index, Uint32 flags)
{
    destroy();
    m_renderer = SDL_CreateRenderer(win, index, flags);
    return m_renderer? 0 : 1;
}


void Renderer::destroy()
{
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Surface
//////////////////////////////////////////////////////////////////////////////////////////

void Surface::destroy()
{
    if (m_surface) {
        SDL_FreeSurface(m_surface);
        m_surface = 0;
    }
}

int Surface::create(Uint32 flags, int w, int h, int depth, 
        Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    destroy();
    m_surface = SDL_CreateRGBSurface(flags, w, h, depth, Rmask, Gmask, Bmask, Amask);
    return m_surface? 0 : 1;
}

int Surface::loadBmp(const char *filename)
{
    destroy();
    m_surface = SDL_LoadBMP(filename);
    return m_surface? 0 : 1;
}

int Surface::setColorKey(Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 key = SDL_MapRGB(m_surface->format, r, g, b);
    SDL_SetColorKey(m_surface, SDL_TRUE, key);
}

int Surface::createFrom(void *ptr, int w, int h, int depth, int pitch, Uint32 rmask,
        Uint32 gmask, Uint32 bmask, Uint32 amask)
{
    m_surface = SDL_CreateRGBSurfaceFrom(ptr, w, h, depth, pitch, rmask, gmask, bmask, amask);
    return m_surface? 0 : 1;
}

#if 0
Surface::Surface(const Surface &copy) throw(HardwareException)
{
    m_surface = SDL_ConvertSurface(copy.m_surface, copy.m_surface->format, 
            copy.m_surface->flags);
    if (!m_surface)
        throw HardwareException("SDL_ConvertSurface() failed.");
}
#endif

Surface::Surface(SDL_Surface *obj)
{
    m_surface = obj;
}

#if 0
Surface::pointer Surface::convert(SDL_Surface *src)
{
    SDL_Surface *p = SDL_ConvertSurface(m_surface, m_surface->format, m_surface->flags);
    Surface::pointer ret(new Surface(p));
    return ret;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Texture
//////////////////////////////////////////////////////////////////////////////////////////

void Texture::destroy()
{
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = 0;
    }
}

int Texture::create(SDL_Renderer *renderer, Uint32 format, int access, int w, int h)
{
    destroy();
    m_texture = SDL_CreateTexture(renderer, format, access, w, h);
    return m_texture? 0 : 1;
}

int Texture::loadBmp(SDL_Renderer *renderer, const char *filename)
{
    destroy();
    Surface s;
    s.loadBmp(filename);
    m_texture = SDL_CreateTextureFromSurface(renderer, s);
    return m_texture? 0 : 1;
}

int Texture::fromSurface(SDL_Renderer *renderer, SDL_Surface *surface)
{
    destroy();
    m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    return m_texture? 0 : 1;
}

