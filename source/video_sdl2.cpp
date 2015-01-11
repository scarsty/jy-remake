/*
==========================================================================================
video_sdl2.cpp - Video模块的SDL2实现
==========================================================================================
*/

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include "video.h"
#include "util.h"
#include "image.h"
#include "charset.h"
#include "font.h"

static const int kWindowWidth = 854;
static const int kWindowHeight = 480;
static const int kMaxRects = 20;

static int current_rect;
static SDL_Rect clip_rects[kMaxRects];	// 当前设置的剪裁矩形


// “金庸群侠传”的UTF-8编码
static const char *jy_window_title = 
    "\xe9\x87\x91\xe5\xba\xb8\xe7\xbe\xa4\xe4\xbe\xa0\xe4\xbc\xa0";
static const char *jy_iconic_title = 
    "\xe9\x87\x91\xe5\xba\xb8\xe7\xbe\xa4\xe4\xbe\xa0\xe4\xbc\xa0";

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

//////////////////////////////////////////////////////////////////////////////////////////
// Sdl2VideoIniter
//////////////////////////////////////////////////////////////////////////////////////////

class Sdl2VideoIniter {
public:
    Sdl2VideoIniter();
    ~Sdl2VideoIniter();
    bool inited() const;
};

Sdl2VideoIniter::~Sdl2VideoIniter()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

Sdl2VideoIniter::Sdl2VideoIniter()
{
    if (!inited()) {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
            throw HardwareException("SDL Video Subsystem initilization failed.");
        }
    }
}

bool Sdl2VideoIniter::inited() const
{
    return SDL_WasInit(SDL_INIT_VIDEO);
}

//////////////////////////////////////////////////////////////////////////////////////////
// SDL2 Video Subsystem
//////////////////////////////////////////////////////////////////////////////////////////

class Sdl2Video : public Video {
public:
    Sdl2Video(int ww, int wh, int sw, int sh);
    ~Sdl2Video();
    virtual Uint32 convertColor(Uint32 color);
    virtual Size getWindowSize();
    virtual Size getScreenSize();
    virtual void fillRect(int x1, int y1, int x2, int y2, Uint32 color);
    virtual void drawRect(int x1, int y1, int x2, int y2, Uint32 color);
    virtual void darkenRect(int x1, int y1, int x2, int y2, int brightness);
    virtual void drawText(const char *str, int x, int y, Uint32 color);
    virtual void fadeIn(Uint32 ms);
    virtual void fadeOut(Uint32 ms);
    virtual void updateScreen();
    virtual SDL_Surface * getSurface();
    virtual void blitSurface(SDL_Surface *dest, int x, int y, int flag, int value);
    virtual SDL_Surface *convertSurface(SDL_Surface *surface);
private:
    Sdl2VideoIniter _sdl2VideoIniter;
    Window      _window;
    Renderer    _renderer;
    Surface     _screenSurface;
    Texture     _screenTexure;
    SDL_Rect    _destBlitRect;
    Size        _windowSize;
    Size        _screenSize;
};


static void calcDestBlitRect(int screenWidth, int screenHeight, 
        int windowWidth, int windowHeight, SDL_Rect *rect)
{
    float orgRatio = (float)screenWidth / screenHeight;
    float curRatio = (float)windowWidth / windowHeight;
    if (orgRatio > curRatio) {
        rect->w = windowWidth;
        rect->h = rect->w / orgRatio;
        rect->x = 0;
        rect->y = ((float)windowHeight - rect->h) / 2;
    } else {
        rect->h = windowHeight;
        rect->w = rect->h * orgRatio;
        rect->x = ((float)windowWidth - rect->w) / 2;
        rect->y = 0;
    }
}

int Video_GetWindowHeight(void)
{
    //int w, h;
    return Video::getInstance()->getWindowSize().cy;
}

int Video_GetWindowWidth(void)
{
    return Video::getInstance()->getWindowSize().cx;
}

void Video_Quit()
{
    delete Video::getInstance();
}

int Video_Init(int w, int h)
{
    Video::getInstance();
    return 0;
}

unsigned char icon_data[] =
#include "icondata.cpp"
;
// 设置窗口图标
static SDL_Surface *the_logo = NULL;
#if 0
static void SetSDLWindowIcon(void)
{
	SDL_RWops *rw;

	rw = SDL_RWFromMem(icon_data, sizeof(icon_data));
	the_logo = SDL_LoadBMP_RW(rw, 0);
	SDL_FreeRW(rw);
	SDL_WM_SetIcon(the_logo, NULL);
}
#endif
static void SetSdl2WindowIcon(SDL_Window *win)
{
	SDL_RWops *rw;
	SDL_Surface *icon = NULL;

	rw = SDL_RWFromMem(icon_data, sizeof(icon_data));
	icon = SDL_LoadBMP_RW(rw, 0);
	SDL_FreeRW(rw);
	//SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format, 255, 0, 255));
	SDL_SetColorKey(icon, SDL_TRUE, SDL_MapRGB(icon->format, 255, 0, 255));
	//SDL_WM_SetIcon(icon, NULL);
    SDL_SetWindowIcon(win, icon);
	SDL_FreeSurface(icon);

}

//绘水平线
static void HLine32(int x1, int x2, int y, int color, unsigned char *vbuffer, int lpitch)
{

	int temp;
	int i;
	int max_x, max_y, min_x, min_y;
	Uint8 *vbuffer2;
	int bpp;
    SDL_Surface *surface = Video::getInstance()->getSurface();

	bpp = surface->format->BytesPerPixel;

	//手工剪裁
	min_x = surface->clip_rect.x;
	min_y = surface->clip_rect.y;
	max_x = surface->clip_rect.x + surface->clip_rect.w - 1;
	max_y = surface->clip_rect.y + surface->clip_rect.h - 1;

	if (y > max_y || y < min_y)
		return;

	if (x1 > x2) {
		temp = x1;
		x1 = x2;
		x2 = temp;
	}

	if (x1 > max_x || x2 < min_x)
		return;

	x1 = ((x1 < min_x) ? min_x : x1);
	x2 = ((x2 > max_x) ? max_x : x2);


	vbuffer2 = vbuffer + y * lpitch + x1 * bpp;
	switch (bpp) {
	case 2:					//16位色彩
		for (i = 0; i <= x2 - x1; i++) {
			*(Uint16 *) vbuffer2 = (Uint16) color;
			vbuffer2 += 2;
		}
		break;
	case 3:					//24位色彩
		for (i = 0; i <= x2 - x1; i++) {
			Uint8 *p = (Uint8 *) (&color);
			*vbuffer2 = *p;
			*(vbuffer2 + 1) = *(p + 1);
			*(vbuffer2 + 2) = *(p + 2);
			vbuffer2 += 3;
		}
		break;
	case 4:					//32位色彩
		for (i = 0; i <= x2 - x1; i++) {
			*(Uint32 *) vbuffer2 = (Uint32) color;
			vbuffer2 += 4;
		}
		break;
	}
}

//绘垂直线
static void VLine32(int y1, int y2, int x, int color, unsigned char *vbuffer, 
        int lpitch)
{

	int temp;
	int i;
	int max_x, max_y, min_x, min_y;
	Uint8 *vbuffer2;
    SDL_Surface *surface = Video::getInstance()->getSurface();

	min_x = surface->clip_rect.x;
	min_y = surface->clip_rect.y;
	max_x = surface->clip_rect.x + surface->clip_rect.w - 1;
	max_y = surface->clip_rect.y + surface->clip_rect.h - 1;


	if (x > max_x || x < min_x)
		return;

	if (y1 > y2) {
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	if (y1 > max_y || y2 < min_y)
		return;

	y1 = ((y1 < min_y) ? min_y : y1);
	y2 = ((y2 > max_y) ? max_y : y2);

	vbuffer2 = vbuffer + y1 * lpitch + x * 4;
    for (i = 0; i <= y2 - y1; i++) {
        *(Uint32 *) vbuffer2 = (Uint32) color;
        vbuffer2 += lpitch;
    }

#if 0
// 只支持32位色，其他色深不再支持。
	switch (bpp) {
	case 2:
		for (i = 0; i <= y2 - y1; i++) {
			*(Uint16 *) vbuffer2 = (Uint16) color;
			vbuffer2 += lpitch;
		}
		break;
	case 3:
		for (i = 0; i <= y2 - y1; i++) {
			Uint8 *p = (Uint8 *) (&color);
			*vbuffer2 = *p;
			*(vbuffer2 + 1) = *(p + 1);
			*(vbuffer2 + 2) = *(p + 2);

			vbuffer2 += lpitch;
		}
		break;
	case 4:
		for (i = 0; i <= y2 - y1; i++) {
			*(Uint32 *) vbuffer2 = (Uint32) color;
			vbuffer2 += lpitch;
		}
		break;
    default:
        Log("%s(): unsupported color depth.", __func__);
        break;
	}
#endif
}


// 转换0RGB到当前屏幕颜色
Uint32 ConvertColor(Uint32 color)
{
    return Video::getInstance()->convertColor(color);
}


int Video_GetScreenWidth(void)
{
    return Video::getInstance()->getScreenSize().cx;
}


int Video_GetScreenHeight(void)
{
    return Video::getInstance()->getScreenSize().cy;
}


// 绘制矩形框
// (x1,y1)--(x2,y2) 框的左上角和右下角坐标
// color 颜色
int Video_DrawRect(int x1, int y1, int x2, int y2, int color)
{
    Video::getInstance()->drawRect(x1, y1, x2, y2, color);
}

// 图形填充
// 如果x1,y1,x2,y2均为0，则填充整个表面
// color, 填充色，用RGB表示，从高到低字节为0RGB
int Video_FillColor(int x1, int y1, int x2, int y2, int color)
{
    Video::getInstance()->fillRect(x1, y1, x2, y2, color);
}

void Video_UpdateScreen(void)
{
    Video::getInstance()->updateScreen();
}

void Video_FadeOut(int ms)
{
    //FadeOut(ms);
    Video::getInstance()->fadeOut(ms);
}


void Video_FadeIn(int ms)
{
    //FadeIn(ms);
    Video::getInstance()->fadeIn(ms);
}


//设置裁剪
int JY_SetClip(int x1, int y1, int x2, int y2)
{
    SDL_Surface *surface = Video::getInstance()->getSurface();
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
		SDL_SetClipRect(surface, NULL);
		current_rect = 0;
	} else {
		SDL_Rect rect;
		rect.x = (Sint16) x1;
		rect.y = (Sint16) y1;
		rect.w = (Uint16) (x2 - x1);
		rect.h = (Uint16) (y2 - y1);

		clip_rects[current_rect] = rect;

		SDL_SetClipRect(surface, &clip_rects[current_rect]);
		current_rect = current_rect + 1;
		if (current_rect >= kMaxRects) {
			current_rect = 0;
		}
	}

	return 0;
}


// 把表面blt到背景或者前景表面
// x,y 要加载到表面的左上角坐标
int BlitSurface(SDL_Surface * lps, int x, int y, int flag, int value)
{
    Video::getInstance()->blitSurface(lps, x, y, flag, value);
}

// 背景变暗
// 把源表面(x1,y1,x2,y2)矩形内的所有点亮度降低
// bright 亮度等级 0-256 
int Video_DarkenRect(int x1, int y1, int x2, int y2, int Bright)
{
    Video::getInstance()->darkenRect(x1, y1, x2, y2, Bright);
}


int Video_DrawText(int x, int y, const char *str, int color, int size, const char *fontname, int charset, int OScharset)
{
    Video::getInstance()->drawText(str, x, y, color);
}



SDL_Surface *Video_ConvertSurface(SDL_Surface * surface)
{
	return SDL_ConvertSurface(surface, Video_GetCanvas()->format, 0);
}


SDL_Surface *Video_GetCanvas(void)
{
    return Video::getInstance()->getSurface();
}

//////////////////////////////////////////////////////////////////////////////////////////
// class Video
//////////////////////////////////////////////////////////////////////////////////////////

Video *Video::_instance = NULL;

Video *Video::getInstance()
{
    if (_instance == NULL) {
        _instance = new Sdl2Video(800, 500, 320, 200);
    }
    return _instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
// class Sdl2Video
//////////////////////////////////////////////////////////////////////////////////////////

Sdl2Video::Sdl2Video(int ww, int wh, int sw, int sh)
    : _window("Heros of Jinyong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ww, wh, 0),
    _renderer(_window, -1, SDL_RENDERER_ACCELERATED),
    _screenSurface(0, sw, sh, 32, 0, 0, 0, 0),
    _screenTexure(_renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, sw, sh)
{
    _screenSize = Size(sw, sh);
    _windowSize = Size(ww, wh);
    calcDestBlitRect(sw, sh, ww, wh, &_destBlitRect);
    SetSdl2WindowIcon(_window);
}


Sdl2Video::~Sdl2Video()
{
}

Uint32 Sdl2Video::convertColor(Uint32 color)
{
    SDL_PixelFormat *format = ((SDL_Surface *)_screenSurface)->format;
	return SDL_MapRGB(format, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

Size Sdl2Video::getScreenSize()
{
    return _screenSize;
}

Size Sdl2Video::getWindowSize()
{
    return _windowSize;
}

void Sdl2Video::fillRect(int x1, int y1, int x2, int y2, Uint32 color)
{
    SDL_Surface *surface = (SDL_Surface *)_screenSurface;
	Uint32 c = convertColor(color);
	SDL_Rect rect;

	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
		SDL_FillRect(surface, NULL, c);
	} else {
		rect.x = (Sint16) x1;
		rect.y = (Sint16) y1;
		rect.w = (Uint16) (x2 - x1);
		rect.h = (Uint16) (y2 - y1);

		SDL_FillRect(surface, &rect, c);
	}
}

void Sdl2Video::darkenRect(int x1, int y1, int x2, int y2, int brightness)
{
	SDL_Rect r1, r2;

	if (x2 <= x1 || y2 <= y1)
		return;

	brightness = 255 - brightness;
	if (brightness > 255)
		brightness = 255;

	r1.x = (Sint16) x1;
	r1.y = (Sint16) y1;
	r1.w = (Uint16) (x2 - x1);
	r1.h = (Uint16) (y2 - y1);
	r2 = r1;


    SDL_PixelFormat *format = getSurface()->format;
    Surface surf(0, r2.w, r2.h, 32,
            format->Rmask,
            format->Gmask,
            format->Bmask,
            0);
    SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
    SDL_FillRect(surf, NULL, 0);
    SDL_SetSurfaceAlphaMod(surf, (Uint8) brightness);
    SDL_BlitSurface(surf, NULL, _screenSurface, &r2);
}

void Sdl2Video::fadeIn(Uint32 ms)
{
    int i;
    Uint32 d = ms;

    Texture tex(_renderer, _screenSurface);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    for (i=0; i<255; i++) {
        Uint8 alpha = i;
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
        SDL_RenderClear(_renderer);
        SDL_SetTextureAlphaMod(tex, alpha);
        SDL_RenderCopy(_renderer, tex, NULL, &_destBlitRect);
        SDL_RenderPresent(_renderer);
        SDL_Delay(5);
    }
    SDL_SetTextureBlendMode(_screenTexure, SDL_BLENDMODE_NONE);
}

void Sdl2Video::fadeOut(Uint32 ms)
{
    int i;
    Uint32 d = ms;

    Texture tex(_renderer, _screenSurface);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    for (i=0; i<255; i++) {
        Uint8 alpha = 255-i;
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
        SDL_RenderClear(_renderer);
        SDL_SetTextureAlphaMod(tex, alpha);
        SDL_RenderCopy(_renderer, tex, NULL, &_destBlitRect);
        SDL_RenderPresent(_renderer);
        SDL_Delay(5);
    }
    SDL_SetTextureBlendMode(_screenTexure, SDL_BLENDMODE_NONE);
}

void Sdl2Video::drawText(const char *str, int x, int y, Uint32 color)
{
    int width = 0;
    int height = 0;
    Font_GetSize(str, &width, &height);
    Font_SetColor(color);
    Font_DrawText(_screenSurface, str, x, y);
}

void Sdl2Video::drawRect(int x1, int y1, int x2, int y2, Uint32 color)
{
	Uint8 *p;
	int lpitch = 0;
	Uint32 c;
	SDL_Rect rect1, rect2;
	int xmin, xmax, ymin, ymax;
    SDL_Surface *surface = (SDL_Surface *)_screenSurface;

	if (x1 < x2) {
		xmin = x1;
		xmax = x2;
	} else {
		xmin = x2;
		xmax = x1;
	}

	if (y1 < y2) {
		ymin = y1;
		ymax = y2;
	} else {
		ymin = y2;
		ymax = y1;
	}

	rect1.x = (Sint16) xmin;
	rect1.y = (Sint16) ymin;
	rect1.w = (Uint16) (xmax - xmin + 1);
	rect1.h = (Uint16) (ymax - ymin + 1);

	SDL_LockSurface(surface);
	p = (Uint8 *) surface->pixels;
	lpitch = surface->pitch;

	c = convertColor(color);

	rect2 = rect1;

	x1 = rect2.x;
	y1 = rect2.y;
	x2 = rect2.x + rect2.w - 1;
	y2 = rect2.y + rect2.h - 1;

	HLine32(x1, x2, y1, c, p, lpitch);
	HLine32(x1, x2, y2, c, p, lpitch);
	VLine32(y1, y2, x1, c, p, lpitch);
	VLine32(y1, y2, x2, c, p, lpitch);

	SDL_UnlockSurface(surface);
}

void Sdl2Video::updateScreen()
{
    SDL_Surface *surface = (SDL_Surface *)_screenSurface;
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(_renderer, NULL);
	SDL_UpdateTexture(_screenTexure, NULL, surface->pixels, surface->pitch);
	SDL_RenderCopy(_renderer, _screenTexure, NULL, &_destBlitRect);
	SDL_RenderPresent(_renderer);
}

SDL_Surface *Sdl2Video::getSurface()
{
    return static_cast<SDL_Surface *>(_screenSurface);
}

SDL_Surface *Sdl2Video::convertSurface(SDL_Surface *surf)
{
	return SDL_ConvertSurface(surf, this->getSurface()->format, 0);
}

void Sdl2Video::blitSurface(SDL_Surface *lps, int x, int y, int flag, int value)
{
	SDL_Surface *tmps;
	SDL_Rect rect;
	int i, j;
    SDL_Surface *surface = getSurface();

	Uint32 color = ConvertColor(COLOR_KEY);

	if (value > 255)
		value = 255;

	rect.x = (Sint16) x;
	rect.y = (Sint16) y;

	if ((flag & 0x2) == 0) {	// 没有alpla
		SDL_BlitSurface(lps, NULL, surface, &rect);
	} else {					// 有alpha
		if ((flag & 0x4) || (flag & 0x8)) {	// 黑白
			int bpp = lps->format->BitsPerPixel;
			tmps = SDL_CreateRGBSurface(0, lps->w, lps->h, bpp, lps->format->Rmask, lps->format->Gmask, lps->format->Bmask, 0);
            // tmpSurface.create(0, lps->w, lps->h, bpp, 
            //         lps->format->Rmask,
            //         lps->format->Gmask,
            //         lps->format->Bmask,
            //         lps->format->Amask);

			SDL_FillRect(tmps, NULL, color);
			//SDL_SetColorKey(tmps,SDL_SRCCOLORKEY ,color);
			SDL_SetColorKey(tmps, SDL_TRUE, color);
			SDL_BlitSurface(lps, NULL, tmps, NULL);
			SDL_LockSurface(tmps);

			if (bpp == 16) {
				for (j = 0; j < tmps->h; j++) {
					Uint16 *p = (Uint16 *) ((Uint8 *) tmps->pixels + j * tmps->pitch);
					for (i = 0; i < tmps->w; i++) {
						if (*p != (Uint16) color) {
							if (flag & 0x4)
								*p = 0;
							else
								*p = 0xffff;
						}
						p++;
					}
				}
			} else if (bpp == 32) {
				for (j = 0; j < tmps->h; j++) {
					Uint32 *p = (Uint32 *) ((Uint8 *) tmps->pixels + j * tmps->pitch);
					for (i = 0; i < tmps->w; i++) {
						if (*p != color) {
							if (flag & 0x4)
								*p = 0;
							else
								*p = 0xffffffff;
						}
						p++;
					}
				}
			} else if (bpp == 24) {
				for (j = 0; j < tmps->h; j++) {
					Uint8 *p = (Uint8 *) tmps->pixels + j * tmps->pitch;
					for (i = 0; i < tmps->w; i++) {
						if ((*p != *(Uint8 *) & color) && (*(p + 1) != *((Uint8 *) & color + 1)) && (*(p + 2) != *((Uint8 *) & color + 2))) {
							if (flag & 0x4) {
								*p = 0;
								*(p + 1) = 0;
								*(p + 2) = 0;
							} else {
								*p = 0xff;
								*(p + 1) = 0xff;
								*(p + 2) = 0xff;
							}
						}
						p += 3;
					}
				}
			}
			SDL_UnlockSurface(tmps);
            SDL_SetSurfaceBlendMode(tmps, SDL_BLENDMODE_BLEND);
			SDL_SetSurfaceAlphaMod(tmps, (Uint8) value);
			SDL_BlitSurface(tmps, NULL, surface, &rect);
			SDL_FreeSurface(tmps);
		} else {
            SDL_SetSurfaceBlendMode(tmps, SDL_BLENDMODE_BLEND);
			//SDL_SetAlpha(lps,SDL_SRCALPHA,(Uint8)value);
			SDL_SetSurfaceAlphaMod(lps, (Uint8) value);
			SDL_BlitSurface(lps, NULL, surface, &rect);
            SDL_SetSurfaceBlendMode(tmps, SDL_BLENDMODE_NONE);
		}
	}
}

