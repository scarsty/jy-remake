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
static const char *jy_window_title = "\xe9\x87\x91\xe5\xba\xb8\xe7\xbe\xa4\xe4\xbe\xa0\xe4\xbc\xa0";
static const char *jy_iconic_title = "\xe9\x87\x91\xe5\xba\xb8\xe7\xbe\xa4\xe4\xbe\xa0\xe4\xbc\xa0";

// SDL2 implementation
class SDL2Video : public Video {
public:
    SDL2Video();
    ~SDL2Video();
    virtual Uint32 convertColor(Uint32 color);
    virtual Size getWindowSize();
    virtual Size getScreenSize();
    virtual int create(int windowW, int windowH, int screenW, int screenH);
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
    Size        m_windowSize;
    Size        m_screenSize;
    Surface     m_screenSurface;
    Texture     m_screenTexture;
    Renderer    m_renderer;
    Window      m_window;
    SDL_Rect    m_destBlitRect;
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
    return Video::getInstance()->create(w, h, 320, 200);
}

#if 0
unsigned char icon_data =
#include "icondata.c"
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
static void SetSDLWindowIcon(void)
{
	SDL_RWops *rw;
	SDL_Surface *icon = NULL;

	rw = SDL_RWFromMem(icon_data, sizeof(icon_data));
	icon = SDL_LoadBMP_RW(rw, 0);
	SDL_FreeRW(rw);
	//SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format, 255, 0, 255));
	SDL_SetColorKey(icon, SDL_TRUE, SDL_MapRGB(icon->format, 255, 0, 255));
	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);

}
#endif

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
static void VLine32(int y1, int y2, int x, int color, unsigned char *vbuffer, int lpitch)
{

	int temp;
	int i;
	int max_x, max_y, min_x, min_y;
	Uint8 *vbuffer2;
	int bpp;
    SDL_Surface *surface = Video::getInstance()->getSurface();

	bpp = surface->format->BytesPerPixel;

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

	vbuffer2 = vbuffer + y1 * lpitch + x * bpp;
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

Video *Video::instance = 0;

Video *Video::getInstance()
{
    if (!instance) {
        instance = new SDL2Video();
    }
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
// class SDL2Video
//////////////////////////////////////////////////////////////////////////////////////////

SDL2Video::SDL2Video()
{
}

SDL2Video::~SDL2Video()
{
}

Uint32 SDL2Video::convertColor(Uint32 color)
{
    SDL_PixelFormat *format = ((SDL_Surface *)m_screenSurface)->format;
	return SDL_MapRGB(format, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

Size SDL2Video::getScreenSize()
{
    return m_screenSize;
}

Size SDL2Video::getWindowSize()
{
    return m_windowSize;
}

void SDL2Video::fillRect(int x1, int y1, int x2, int y2, Uint32 color)
{
    SDL_Surface *surface = (SDL_Surface *)m_screenSurface;
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

void SDL2Video::darkenRect(int x1, int y1, int x2, int y2, int brightness)
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


    Surface surf;
    SDL_PixelFormat *format = getSurface()->format;
    surf.create(0, r2.w, r2.h, 32, format->Rmask, format->Gmask, format->Bmask, 0);
    SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
    SDL_FillRect(surf, NULL, 0);
    SDL_SetSurfaceAlphaMod(surf, (Uint8) brightness);
    SDL_BlitSurface(surf, NULL, m_screenSurface, &r2);
}


int SDL2Video::create(int windowWidth, int windowHeight, int screenWidth, int screenHeight)
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        Log("cannot init video subsystem");
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    if (m_window.create("Heros of Jin Yong", SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0)) {
        Log("cannot create window");
        return 1;
    }
    if (m_renderer.create(m_window, -1, SDL_RENDERER_ACCELERATED)) {
        Log("cannot create m_renderer");
        return 1;
    }

    m_screenSize = Size(screenWidth, screenHeight);
    m_windowSize = Size(windowWidth, windowHeight);

    //calcDestBlitRect(screenWidth, screenHeight, winWidth, winHeight, &m_destBlitRect);
    calcDestBlitRect(m_screenSize.cx, m_screenSize.cy, m_windowSize.cx, 
            m_windowSize.cy, &m_destBlitRect);
    m_screenSurface.create(0, screenWidth, screenHeight, 32, 0, 0, 0, 0);
    //m_screenTexture.fromSurface(m_renderer, m_screenSurface);
	m_screenTexture.create(m_renderer, SDL_PIXELFORMAT_ARGB8888, 
            SDL_TEXTUREACCESS_STATIC, screenWidth, screenHeight);
    return 0;
}


void SDL2Video::fadeIn(Uint32 ms)
{
    int i;
    Uint32 d = ms;

    Texture tex;
    tex.fromSurface(m_renderer, m_screenSurface);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    for (i=0; i<255; i++) {
        Uint8 alpha = i;
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
        SDL_RenderClear(m_renderer);
        SDL_SetTextureAlphaMod(tex, alpha);
        SDL_RenderCopy(m_renderer, tex, NULL, &m_destBlitRect);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(5);
    }
    SDL_SetTextureBlendMode(m_screenTexture, SDL_BLENDMODE_NONE);
    SDL_DestroyTexture(tex);
}

void SDL2Video::fadeOut(Uint32 ms)
{
    int i;
    Uint32 d = ms;

    Texture tex;
    tex.fromSurface(m_renderer, m_screenSurface);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    for (i=0; i<255; i++) {
        Uint8 alpha = 255-i;
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
        SDL_RenderClear(m_renderer);
        SDL_SetTextureAlphaMod(tex, alpha);
        SDL_RenderCopy(m_renderer, tex, NULL, &m_destBlitRect);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(5);
    }
    SDL_SetTextureBlendMode(m_screenTexture, SDL_BLENDMODE_NONE);
    SDL_DestroyTexture(tex);
}

void SDL2Video::drawText(const char *str, int x, int y, Uint32 color)
{
    int width = 0;
    int height = 0;
    Font_GetSize(str, &width, &height);
    Font_SetColor(color);
    Font_DrawText(m_screenSurface, str, x, y);
}

void SDL2Video::drawRect(int x1, int y1, int x2, int y2, Uint32 color)
{
	Uint8 *p;
	int lpitch = 0;
	Uint32 c;
	SDL_Rect rect1, rect2;
	int xmin, xmax, ymin, ymax;
    SDL_Surface *surface = (SDL_Surface *)m_screenSurface;

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

void SDL2Video::updateScreen()
{
    SDL_Surface *surface = (SDL_Surface *)m_screenSurface;
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(m_renderer, NULL);
	SDL_UpdateTexture(m_screenTexture, NULL, surface->pixels, surface->pitch);
	SDL_RenderCopy(m_renderer, m_screenTexture, NULL, &m_destBlitRect);
	SDL_RenderPresent(m_renderer);
}

SDL_Surface *SDL2Video::getSurface()
{
    return static_cast<SDL_Surface *>(m_screenSurface);
}

SDL_Surface *SDL2Video::convertSurface(SDL_Surface *surf)
{
	return SDL_ConvertSurface(surf, this->getSurface()->format, 0);
}

void SDL2Video::blitSurface(SDL_Surface *lps, int x, int y, int flag, int value)
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

