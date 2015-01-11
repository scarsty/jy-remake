/*
==========================================================================================
video_sdl.cpp - Video模块的SDL 1.2 实现
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
// SdlVideoIniter
//////////////////////////////////////////////////////////////////////////////////////////

class SdlVideoIniter {
public:
    SdlVideoIniter();
    ~SdlVideoIniter();
    bool inited() const;
};

SdlVideoIniter::~SdlVideoIniter()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

SdlVideoIniter::SdlVideoIniter()
{
    if (!inited()) {
        //SDL_putenv("SDL_VIDEODRIVER=directx");
        //SDL_putenv("SDL_VIDEO_CENTERED=center");
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
            throw HardwareException("SDL Video Subsystem initilization failed.");
        }
    }
}

bool SdlVideoIniter::inited() const
{
    return SDL_WasInit(SDL_INIT_VIDEO);
}

//////////////////////////////////////////////////////////////////////////////////////////
// SDL2 Video Subsystem
//////////////////////////////////////////////////////////////////////////////////////////

class SdlVideo : public Video {
public:
    SdlVideo(int ww, int wh, int sw, int sh);
    ~SdlVideo();
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
    void setVideoMode(int ww, int wh, int sw, int sh);
    void ShowSlow(int delaytime, int Flag);
private:
    SdlVideoIniter _sdlVideoIniter;
    //std::auto_ptr<SDL_Surface> _screenSurface;
    //std::auto_ptr<SDL_Surface> _windowSurface;
    Surface _screenSurface;
    Surface _windowSurface;
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

void SdlVideo::setVideoMode(int ww, int wh, int sw, int sh)
{
	int ret = 0;
	char str[256];
	const SDL_VideoInfo *info;

	//SetSDLWindowIcon();
	SDL_WM_SetCaption(jy_window_title, jy_iconic_title);
    _windowSurface.reset(SDL_SetVideoMode(ww, wh, 32, 
                SDL_SWSURFACE));
	if (!_windowSurface.get())
        throw HardwareException("SDL_SetVideoMode() failed.");
    SDL_Surface *surf = _windowSurface.get();
	_screenSurface.reset(SDL_CreateRGBSurface(0, sw, sh, 
                            surf->format->BitsPerPixel, 
                            surf->format->Rmask, 
                            surf->format->Gmask, 
                            surf->format->Bmask, 
                            surf->format->Amask));
    if (!_screenSurface.get())
        throw HardwareException("SDL_CreateRGBSurface() failed.");
	info = SDL_GetVideoInfo();
	SDL_VideoDriverName(str, sizeof(str));
	Log(va("Video Driver: %s", str));
	Log(va("hw_available=%d, wm_available=%d", info->hw_available, info->wm_available));
	Log(va("blit_hw=%d, blit_hw_CC=%d, blit_hw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A));
	Log(va("blit_sw=%d,blit_sw_CC=%d,blit_sw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A));
	Log(va("blit_fill=%d,videomem=%d", info->blit_fill, info->video_mem));
	Log(va("Color depth=%d", info->vfmt->BitsPerPixel));
}

int Video_GetWindowHeight(void)
{
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
        _instance = new SdlVideo(800, 500, 320, 200);
    }
    return _instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
// class SdlVideo
//////////////////////////////////////////////////////////////////////////////////////////

SdlVideo::SdlVideo(int ww, int wh, int sw, int sh)
{
    setVideoMode(ww, wh, sw, sh);
    _screenSize = Size(sw, sh);
    _windowSize = Size(ww, wh);
    calcDestBlitRect(sw, sh, ww, wh, &_destBlitRect);
    //_windowSurface = SDL_SetVideoMode(ww, wh, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
}


SdlVideo::~SdlVideo()
{
}

Uint32 SdlVideo::convertColor(Uint32 color)
{
    SDL_PixelFormat *format = _screenSurface.get()->format;
	return SDL_MapRGB(format, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

Size SdlVideo::getScreenSize()
{
    return _screenSize;
}

Size SdlVideo::getWindowSize()
{
    return _windowSize;
}

void SdlVideo::fillRect(int x1, int y1, int x2, int y2, Uint32 color)
{
	Uint32 c = convertColor(color);
	SDL_Rect rect;

	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
		SDL_FillRect(_screenSurface, NULL, c);
	} 
    else {
		rect.x = (Sint16) x1;
		rect.y = (Sint16) y1;
		rect.w = (Uint16) (x2 - x1);
		rect.h = (Uint16) (y2 - y1);
		SDL_FillRect(_screenSurface, &rect, c);
	}
}

void SdlVideo::darkenRect(int x1, int y1, int x2, int y2, int brightness)
{
	SDL_Rect r1, r2;
	if (x2 <= x1 || y2 <= y1)
		return;
	brightness = 256 - brightness;
	if (brightness > 255)
		brightness = 255;
	r1.x = (Sint16) x1;
	r1.y = (Sint16) y1;
	r1.w = (Uint16) (x2 - x1);
	r1.h = (Uint16) (y2 - y1);
	r2 = r1;
    SDL_Surface *screen = getSurface();
    Surface lps1(SDL_SWSURFACE, r2.w, r2.h, screen->format->BitsPerPixel,
            screen->format->Rmask,
            screen->format->Gmask,
            screen->format->Bmask,
            0);
	SDL_FillRect(lps1, NULL, 0);
	SDL_SetAlpha(lps1, SDL_SRCALPHA, (Uint8) brightness);
	SDL_BlitSurface(lps1, NULL, screen, &r2);
}

// 缓慢显示图形 
// delaytime 每次渐变延时毫秒数
// Flag=0 从暗到亮，1，从亮到暗
void SdlVideo::ShowSlow(int delaytime, int Flag)
{
	int i;
	int step;
	Uint32 t1, t2;
	int alpha;

    SDL_Surface *screen = _screenSurface; //_screenSurface;
    Surface tmps(_screenSurface);

	SDL_BlitSurface(screen, NULL, tmps, NULL);	//当前表面复制到临时表面
	for (i = 0; i <= 32; i++) {
		if (Flag == 0)
			step = i;
		else
			step = 32 - i;

		t1 = SDL_GetTicks();
		SDL_FillRect(screen, NULL, 0);	//当前表面变黑
		alpha = step << 3;
		if (alpha > 255)
			alpha = 255;
		SDL_SetAlpha(tmps, SDL_SRCALPHA, (Uint8) alpha);	//设置alpha
		SDL_BlitSurface(tmps, NULL, screen, NULL);
        updateScreen();
		t2 = SDL_GetTicks();
		if (delaytime > t2 - t1)
			SDL_Delay(delaytime - (t2 - t1));
	}
}

void SdlVideo::fadeIn(Uint32 ms)
{
	ShowSlow(ms, 0);
}

void SdlVideo::fadeOut(Uint32 ms)
{
	ShowSlow(ms, 1);
}

void SdlVideo::drawText(const char *str, int x, int y, Uint32 color)
{
    int width = 0;
    int height = 0;
    Font_GetSize(str, &width, &height);
    Font_SetColor(color);
    Font_DrawText(_screenSurface, str, x, y);
}

void SdlVideo::drawRect(int x1, int y1, int x2, int y2, Uint32 color)
{
	Uint8 *p;
	int lpitch = 0;
	Uint32 c;
	SDL_Rect rect1, rect2;
	int xmin, xmax, ymin, ymax;
    SDL_Surface *surface = _screenSurface;

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

void SdlVideo::updateScreen()
{
    SDL_SoftStretch(_screenSurface, NULL, _windowSurface, NULL);
    SDL_UpdateRect(_windowSurface, 0, 0, 0, 0);
    //SDL_Flip(_windowSurface);
}

SDL_Surface *SdlVideo::getSurface()
{
    return _screenSurface.get();
}

SDL_Surface *SdlVideo::convertSurface(SDL_Surface *surf)
{
	return SDL_ConvertSurface(surf, _screenSurface.get()->format, 0);
}

static void binarizationSurface(SDL_Surface *surface, Uint32 keycolor, Uint32 color)
{
    int i, j;
    for (j = 0; j < surface->h; j++) {
        Uint32 *p = (Uint32 *) ((Uint8 *) surface->pixels + j * surface->pitch);
        for (i = 0; i < surface->w; i++) {
            if (*p != keycolor) {
                *p = color;
            }
            p++;
        }
    }
}

void SdlVideo::blitSurface(SDL_Surface *lps, int x, int y, int flag, int alpha)
{
	SDL_Surface *tmps = NULL;
	SDL_Rect rect;
	int i, j;
	Uint32 colorKey = ConvertColor(COLOR_KEY);

	if (alpha > 255)
		alpha = 255;

	rect.x = (Sint16) x;
	rect.y = (Sint16) y;

	if ((flag & 0x2) == 0) {	// 没有alpla
		SDL_BlitSurface(lps, NULL, _screenSurface, &rect);
	}
    else {					// 有alpha
		if ((flag & 0x4) || (flag & 0x8)) {	// 黑白
            Surface tmps(SDL_SWSURFACE, lps->w, lps->h, 32,
                    lps->format->Rmask,
                    lps->format->Gmask,
                    lps->format->Bmask,
                    0);
			int bpp = lps->format->BitsPerPixel;
			//tmps = SDL_CreateRGBSurface(SDL_SWSURFACE, lps->w, lps->h, bpp, 
                //lps->format->Rmask, lps->format->Gmask, lps->format->Bmask, 0);
			SDL_FillRect(tmps, NULL, colorKey);
			SDL_SetColorKey(tmps, SDL_SRCCOLORKEY, colorKey);
			SDL_BlitSurface(lps, NULL, tmps, NULL);
			SDL_LockSurface(tmps);
            int i, j;

#if 0
			if (bpp == 32) {
				for (j = 0; j < tmps.get()->h; j++) {
					Uint32 *p = (Uint32 *) ((Uint8 *) tmps.get()->pixels + j * tmps.get()->pitch);
					for (i = 0; i < tmps.get()->w; i++) {
						if (*p != colorKey) {
							if (flag & 0x4)
								*p = 0;
							else
								*p = 0xffffffff;
						}
						p++;
					}
				}
			}
#endif

            binarizationSurface(tmps, colorKey, flag & 0x4? 0 : 0xffffffff);
			SDL_UnlockSurface(tmps);
			SDL_SetAlpha(tmps, SDL_SRCALPHA, (Uint8) alpha);
			SDL_BlitSurface(tmps, NULL, _screenSurface, &rect);
		}
        else {
			SDL_SetAlpha(lps, SDL_SRCALPHA, (Uint8) alpha);
			SDL_BlitSurface(lps, NULL, _screenSurface, &rect);
		}
	}
}

