////////////////////////////////////////////////////////////////////////////////////////////////////
// SDL 1.2 接口
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include "video.h"
#include "util.h"
#include "system.h"
#include "image.h"
#include "charset.h"

#define VIDEO_WIDTH  	640
#define VIDEO_HEIGHT 	400
#define MAX_RECTS  		20
#define MAX_FONTS 		10		//定义同时打开的字体个数

typedef struct SUseFont
{								// 定义当前使用的字体结构
	int size;					//字号，单位像素
	char *name;					//字体文件名
	TTF_Font *font;				//打开的字体
} TUseFont;

// 显示TTF 字符串
// 为快速显示，程序将保存已经打开的相应字号的字体结构。这样做可以加快程序速度
// 为简化代码，没有用链表，而是采用数组来保存打开的字体。
// 用先进先出的方法，循环关闭已经打开的字体。
// 考虑到一般打开的字体不多，比如640*480模式实际上只用了16*24*32三种字体。
// 设置数组为10已经足够。

static TUseFont all_fonts[MAX_FONTS];	//保存已打开的字体
static int current_font = 0;
// screen width and height set by config.lua
int g_ScreenW;
int g_ScreenH;
SDL_Surface *g_screen;
// 原版《金庸群侠传》的分辨率为320x200。
SDL_Surface *g_fake_screen;
static int current_rect = 0;
static SDL_Rect clip_rects[MAX_RECTS];	// 当前设置的剪裁矩形



// “金庸群侠传”的UTF-8编码
static const char *jy_window_title = "\xe9\x87\x91\xe5\xba\xb8\xe7\xbe\xa4\xe4\xbe\xa0\xe4\xbc\xa0";
static const char *jy_iconic_title = "\xe9\x87\x91\xe5\xba\xb8\xe7\xbe\xa4\xe4\xbe\xa0\xe4\xbc\xa0";

#include "jylogo.c"
// 设置窗口图标
static SDL_Surface *the_logo = NULL;
#if 0
static void SetSDLWindowIcon(void)
{
	SDL_RWops *rw;

	rw = SDL_RWFromMem(logo_data, sizeof(logo_data));
	the_logo = SDL_LoadBMP_RW(rw, 0);
	SDL_FreeRW(rw);
	SDL_WM_SetIcon(the_logo, NULL);
}
#endif
static void SetSDLWindowIcon(void)
{
	SDL_RWops *rw;
	SDL_Surface *icon = NULL;

	rw = SDL_RWFromMem(logo_data, sizeof(logo_data));
	icon = SDL_LoadBMP_RW(rw, 0);
	SDL_FreeRW(rw);
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format, 255, 0, 255));
	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);

}

//绘水平线
static void HLine32(int x1, int x2, int y, int color, unsigned char *vbuffer, int lpitch)
{

	int temp;
	int i;
	int max_x, max_y, min_x, min_y;
	uint8 *vbuffer2;
	int bpp;

	bpp = g_fake_screen->format->BytesPerPixel;

	//手工剪裁
	min_x = g_fake_screen->clip_rect.x;
	min_y = g_fake_screen->clip_rect.y;
	max_x = g_fake_screen->clip_rect.x + g_fake_screen->clip_rect.w - 1;
	max_y = g_fake_screen->clip_rect.y + g_fake_screen->clip_rect.h - 1;

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
			*(uint16 *) vbuffer2 = (uint16) color;
			vbuffer2 += 2;
		}
		break;
	case 3:					//24位色彩
		for (i = 0; i <= x2 - x1; i++) {
			uint8 *p = (uint8 *) (&color);
			*vbuffer2 = *p;
			*(vbuffer2 + 1) = *(p + 1);
			*(vbuffer2 + 2) = *(p + 2);
			vbuffer2 += 3;
		}
		break;
	case 4:					//32位色彩
		for (i = 0; i <= x2 - x1; i++) {
			*(uint32 *) vbuffer2 = (uint32) color;
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
	uint8 *vbuffer2;
	int bpp;

	bpp = g_fake_screen->format->BytesPerPixel;

	min_x = g_fake_screen->clip_rect.x;
	min_y = g_fake_screen->clip_rect.y;
	max_x = g_fake_screen->clip_rect.x + g_fake_screen->clip_rect.w - 1;
	max_y = g_fake_screen->clip_rect.y + g_fake_screen->clip_rect.h - 1;


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
			*(uint16 *) vbuffer2 = (uint16) color;
			vbuffer2 += lpitch;
		}
		break;
	case 3:
		for (i = 0; i <= y2 - y1; i++) {
			uint8 *p = (uint8 *) (&color);
			*vbuffer2 = *p;
			*(vbuffer2 + 1) = *(p + 1);
			*(vbuffer2 + 2) = *(p + 2);

			vbuffer2 += lpitch;
		}
		break;
	case 4:
		for (i = 0; i <= y2 - y1; i++) {
			*(uint32 *) vbuffer2 = (uint32) color;
			vbuffer2 += lpitch;
		}
		break;
	}

}


//初始化
int Font_Init(void)
{
	int i;

	TTF_Init();					// 初始化sdl_ttf

	for (i = 0; i < MAX_FONTS; i++) {	//字体数据初值
		all_fonts[i].size = 0;
		all_fonts[i].name = NULL;
		all_fonts[i].font = NULL;
	}

	return 0;
}

//释放字体结构
void Font_Quit(void)
{
	int i;

	for (i = 0; i < MAX_FONTS; i++) {	//释放字体数据
		if (all_fonts[i].font) {
			TTF_CloseFont(all_fonts[i].font);
		}
		SAFE_FREE(all_fonts[i].name);
	}

	TTF_Quit();
}


// 转换0RGB到当前屏幕颜色
uint32 ConvertColor(uint32 color)
{
	return SDL_MapRGB(g_fake_screen->format, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}


int Video_GetScreenWidth(void)
{
	return g_ScreenW;
}

int Video_GetScreenHeight(void)
{
	return g_ScreenH;
}

int Video_SetMode(int width, int height, SDL_bool fullscreen)
{
	int ret = 0;
	char str[256];
	int w, h;
	const SDL_VideoInfo *info;

	SetSDLWindowIcon();
	SDL_WM_SetCaption(jy_window_title, jy_iconic_title);

	w = g_ScreenW;
	h = g_ScreenH;

	if (fullscreen)
		g_screen = SDL_SetVideoMode(VIDEO_WIDTH, VIDEO_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
	else
		g_screen = SDL_SetVideoMode(VIDEO_WIDTH, VIDEO_HEIGHT, 0, SDL_HWSURFACE);

	if (!g_screen)
		JY_ERROR("cannot set video mode");

	g_fake_screen = SDL_CreateRGBSurface(g_screen->flags & ~SDL_HWSURFACE,
										 w, h, g_screen->format->BitsPerPixel, g_screen->format->Rmask, g_screen->format->Gmask, g_screen->format->Bmask, g_screen->format->Amask);

	if (g_fake_screen == NULL)
		JY_ERROR("cannot create virtual screen");

	info = SDL_GetVideoInfo();
	SDL_VideoDriverName(str, sizeof(str));
	Log(va("Video Driver: %s", str));
	Log(va("hw_available=%d, wm_available=%d", info->hw_available, info->wm_available));
	Log(va("blit_hw=%d, blit_hw_CC=%d, blit_hw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A));
	Log(va("blit_sw=%d,blit_sw_CC=%d,blit_sw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A));
	Log(va("blit_fill=%d,videomem=%d", info->blit_fill, info->video_mem));
	Log(va("Color depth=%d", info->vfmt->BitsPerPixel));
	return 0;
}


int Video_Init(void)
{
	// WinDIB driver doesn't support hardware blitting
	// SDL use WinDIB as the default video driver since version 1.2.10, if you want use
	// DirectX driver, you should set environment variable SDL_VIDEODRIVER=directx
	// SDL_putenv("SDL_VIDEODRIVER=directx");

	SDL_putenv("SDL_VIDEO_CENTERED=center");
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		return -1;
	if (Video_SetMode(VIDEO_WIDTH, VIDEO_HEIGHT, SDL_FALSE))
		return -1;
	if (Font_Init())
		return -1;
	return 0;
}


void Video_Quit(void)
{
	Font_Quit();
	if (the_logo) {
		SDL_FreeSurface(the_logo);
		the_logo = NULL;
	}
	if (g_fake_screen) {
		SDL_FreeSurface(g_fake_screen);
		g_fake_screen = NULL;
	}
}




// 绘制矩形框
// (x1,y1)--(x2,y2) 框的左上角和右下角坐标
// color 颜色
int Video_DrawRect(int x1, int y1, int x2, int y2, int color)
{
	uint8 *p;
	int lpitch = 0;
	uint32 c;
	SDL_Rect rect1, rect2;
	int xmin, xmax, ymin, ymax;

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

	rect1.x = (sint16) xmin;
	rect1.y = (sint16) ymin;
	rect1.w = (uint16) (xmax - xmin + 1);
	rect1.h = (uint16) (ymax - ymin + 1);

	SDL_LockSurface(g_fake_screen);
	p = (uint8 *) g_fake_screen->pixels;
	lpitch = g_fake_screen->pitch;

	c = ConvertColor(color);

	rect2 = rect1;

	x1 = rect2.x;
	y1 = rect2.y;
	x2 = rect2.x + rect2.w - 1;
	y2 = rect2.y + rect2.h - 1;

	HLine32(x1, x2, y1, c, p, lpitch);
	HLine32(x1, x2, y2, c, p, lpitch);
	VLine32(y1, y2, x1, c, p, lpitch);
	VLine32(y1, y2, x2, c, p, lpitch);

	SDL_UnlockSurface(g_fake_screen);

	return 0;
}

// 图形填充
// 如果x1,y1,x2,y2均为0，则填充整个表面
// color, 填充色，用RGB表示，从高到低字节为0RGB
int Video_FillColor(int x1, int y1, int x2, int y2, int color)
{
	int c = ConvertColor(color);

	SDL_Rect rect;

	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
		SDL_FillRect(g_fake_screen, NULL, c);
	} else {
		rect.x = (sint16) x1;
		rect.y = (sint16) y1;
		rect.w = (uint16) (x2 - x1);
		rect.h = (uint16) (y2 - y1);

		SDL_FillRect(g_fake_screen, &rect, c);
	}

	return 0;

}


//显示表面
//flag = 0 显示全部表面  =1 按照JY_SetClip设置的矩形显示，如果没有矩形，则不显示
#if 0
int Video_UpdateScreen(int flag)
{
	if (flag == 1) {
		if (current_rect > 0) {
			SDL_UpdateRects(g_fake_screen, current_rect, clip_rects);
		}
	} else {
		SDL_UpdateRect(g_fake_screen, 0, 0, 0, 0);
	}
	return 0;
}
#endif

void Video_UpdateScreen(void)
{
	SDL_SoftStretch(g_fake_screen, NULL, g_screen, NULL);
	//SDL_BlitSurface(g_fake_screen, NULL, g_screen, NULL);
	SDL_UpdateRect(g_screen, 0, 0, 0, 0);
}



// 缓慢显示图形 
// delaytime 每次渐变延时毫秒数
// Flag=0 从暗到亮，1，从亮到暗
int ShowSlow(int delaytime, int Flag)
{
	int i;
	int step;
	Uint32 t1, t2;
	int alpha;


	SDL_Surface *lps1;			// 建立临时表面
	lps1 = SDL_CreateRGBSurface(SDL_SWSURFACE, g_fake_screen->w, g_fake_screen->h, g_fake_screen->format->BitsPerPixel,
								g_fake_screen->format->Rmask, g_fake_screen->format->Gmask, g_fake_screen->format->Bmask, 0);

	if (lps1 == NULL) {
		JY_ERROR("cannot create surface");
		return 1;
	}

	SDL_BlitSurface(g_fake_screen, NULL, lps1, NULL);	//当前表面复制到临时表面

	for (i = 0; i <= 32; i++) {
		if (Flag == 0)
			step = i;
		else
			step = 32 - i;

		t1 = SDL_GetTicks();
		SDL_FillRect(g_fake_screen, NULL, 0);	//当前表面变黑
		alpha = step << 3;
		if (alpha > 255)
			alpha = 255;
		SDL_SetAlpha(lps1, SDL_SRCALPHA, (uint8) alpha);	//设置alpha
		SDL_BlitSurface(lps1, NULL, g_fake_screen, NULL);
		Video_UpdateScreen();
		t2 = SDL_GetTicks();
		if (delaytime > t2 - t1)
			SDL_Delay(delaytime - (t2 - t1));
	}
	SDL_FreeSurface(lps1);		//释放表面
	return 0;
}

void Video_FadeOut(int ms)
{
	ShowSlow(ms, 1);
}

void Video_FadeIn(int ms)
{
	ShowSlow(ms, 0);
}

//设置裁剪
int JY_SetClip(int x1, int y1, int x2, int y2)
{
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
		SDL_SetClipRect(g_fake_screen, NULL);
		current_rect = 0;
	} else {
		SDL_Rect rect;
		rect.x = (sint16) x1;
		rect.y = (sint16) y1;
		rect.w = (uint16) (x2 - x1);
		rect.h = (uint16) (y2 - y1);

		clip_rects[current_rect] = rect;

		SDL_SetClipRect(g_fake_screen, &clip_rects[current_rect]);
		current_rect = current_rect + 1;
		if (current_rect >= MAX_RECTS) {
			current_rect = 0;
		}
	}

	return 0;
}

// 全屏切换
int JY_FullScreen()
{
	SDL_Surface *tmpsurface;
	const SDL_VideoInfo *info;

	uint32 flag = g_screen->flags;

	tmpsurface = SDL_CreateRGBSurface(g_fake_screen->flags,
									  g_fake_screen->w,
									  g_fake_screen->h,
									  g_fake_screen->format->BitsPerPixel, g_fake_screen->format->Rmask, g_fake_screen->format->Gmask, g_fake_screen->format->Bmask, g_fake_screen->format->Amask);

	SDL_BlitSurface(g_fake_screen, NULL, tmpsurface, NULL);
	SDL_FreeSurface(g_fake_screen);

	if (flag & SDL_FULLSCREEN) {	//全屏，设置窗口
		g_screen = SDL_SetVideoMode(g_screen->w, g_screen->h, 0, SDL_SWSURFACE);
		SDL_ShowCursor(SDL_ENABLE);
	} else {
		g_screen = SDL_SetVideoMode(g_screen->w, g_screen->h, 32, SDL_HWSURFACE | SDL_FULLSCREEN);
		SDL_ShowCursor(SDL_DISABLE);
	}

	g_fake_screen = SDL_CreateRGBSurface(g_screen->flags & ~SDL_HWSURFACE,
										 g_ScreenW, g_ScreenH, g_screen->format->BitsPerPixel, g_screen->format->Rmask, g_screen->format->Gmask, g_screen->format->Bmask, g_screen->format->Amask);

	SDL_BlitSurface(tmpsurface, NULL, g_fake_screen, NULL);

	Video_UpdateScreen();

	SDL_FreeSurface(tmpsurface);

	info = SDL_GetVideoInfo();

	Log(va("hw_available=%d, wm_available=%d", info->hw_available, info->wm_available));
	Log(va("blit_hw=%d, blit_hw_CC=%d, blit_hw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A));
	Log(va("blit_sw=%d,blit_sw_CC=%d,blit_sw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A));
	Log(va("blit_fill=%d,videomem=%d", info->blit_fill, info->video_mem));
	Log(va("Color depth=%d", info->vfmt->BitsPerPixel));

	return 0;
}

// 把表面blt到背景或者前景表面
// x,y 要加载到表面的左上角坐标
int BlitSurface(SDL_Surface * lps, int x, int y, int flag, int alpha)
{

	SDL_Surface *tmps;
	SDL_Rect rect;
	int i, j;
	uint32 color = ConvertColor(COLOR_KEY);

	if (alpha > 255)
		alpha = 255;

	rect.x = (sint16) x;
	rect.y = (sint16) y;

	if ((flag & 0x2) == 0) {	// 没有alpla
		SDL_BlitSurface(lps, NULL, g_fake_screen, &rect);
	} else {					// 有alpha
		if ((flag & 0x4) || (flag & 0x8)) {	// 黑白
			int bpp = lps->format->BitsPerPixel;
			//创建临时表面
			tmps = SDL_CreateRGBSurface(SDL_SWSURFACE, lps->w, lps->h, bpp, lps->format->Rmask, lps->format->Gmask, lps->format->Bmask, 0);

			SDL_FillRect(tmps, NULL, color);
			SDL_SetColorKey(tmps, SDL_SRCCOLORKEY, color);
			SDL_BlitSurface(lps, NULL, tmps, NULL);
			SDL_LockSurface(tmps);

			if (bpp == 16) {
				for (j = 0; j < tmps->h; j++) {
					uint16 *p = (uint16 *) ((uint8 *) tmps->pixels + j * tmps->pitch);
					for (i = 0; i < tmps->w; i++) {
						if (*p != (uint16) color) {
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
					uint32 *p = (uint32 *) ((uint8 *) tmps->pixels + j * tmps->pitch);
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
					uint8 *p = (uint8 *) tmps->pixels + j * tmps->pitch;
					for (i = 0; i < tmps->w; i++) {
						if ((*p != *(uint8 *) & color) && (*(p + 1) != *((uint8 *) & color + 1)) && (*(p + 2) != *((uint8 *) & color + 2))) {
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
			SDL_SetAlpha(tmps, SDL_SRCALPHA, (uint8) alpha);
			SDL_BlitSurface(tmps, NULL, g_fake_screen, &rect);
			SDL_FreeSurface(tmps);
		} else {
			SDL_SetAlpha(lps, SDL_SRCALPHA, (uint8) alpha);
			SDL_BlitSurface(lps, NULL, g_fake_screen, &rect);
		}
	}
	return 0;
}


// 背景变暗
// 把源表面(x1,y1,x2,y2)矩形内的所有点亮度降低
// bright 亮度等级 0-256 
int Video_DarkenRect(int x1, int y1, int x2, int y2, int brightness)
{
	SDL_Surface *lps1;
	SDL_Rect r1, r2;

	if (x2 <= x1 || y2 <= y1)
		return 0;

	brightness = 256 - brightness;
	if (brightness > 255)
		brightness = 255;

	r1.x = (sint16) x1;
	r1.y = (sint16) y1;
	r1.w = (uint16) (x2 - x1);
	r1.h = (uint16) (y2 - y1);

	r2 = r1;


	lps1 = SDL_CreateRGBSurface(SDL_SWSURFACE, r2.w, r2.h, g_fake_screen->format->BitsPerPixel, g_fake_screen->format->Rmask, g_fake_screen->format->Gmask, g_fake_screen->format->Bmask, 0);

	SDL_FillRect(lps1, NULL, 0);
	SDL_SetAlpha(lps1, SDL_SRCALPHA, (uint8) brightness);
	SDL_BlitSurface(lps1, NULL, g_fake_screen, &r2);
	SDL_FreeSurface(lps1);

	return 1;
}


void Video_SaveScreenshot(void)
{
	int iNumBMP = 0;
	FILE *fp;

	//
	// Find a usable BMP filename.
	//
	for (iNumBMP = 0; iNumBMP <= 9999; iNumBMP++) {
		fp = fopen(va("%sscrn%.4d.bmp", "JY_", iNumBMP), "rb");
		if (fp == NULL) {
			break;
		}
		fclose(fp);
	}

	if (iNumBMP > 9999) {
		return;
	}
	//
	// Save the screenshot.
	//
	SDL_SaveBMP(g_screen, va("%sscrn%.4d.bmp", "JY_", iNumBMP));
}



// 根据字体文件名和字号打开字体
// size 为按像素大小的字号
static TTF_Font *GetFont(const char *filename, int size)
{
	int i;
	TTF_Font *myfont = NULL;

	for (i = 0; i < MAX_FONTS; i++) {	//  判断字体是否已打开
		if ((all_fonts[i].size == size) && (all_fonts[i].name) && (strcmp(filename, all_fonts[i].name) == 0)) {
			myfont = all_fonts[i].font;
			break;
		}
	}
	if (myfont == NULL) {		//没有打开
		myfont = TTF_OpenFont(filename, size);	//打开新字体
		if (myfont == NULL) {
			JY_ERROR(va("cannot open file: %s", filename));
			return NULL;
		}
		all_fonts[current_font].size = size;
		if (all_fonts[current_font].font)	//直接关闭当前字体。
			TTF_CloseFont(all_fonts[current_font].font);

		all_fonts[current_font].font = myfont;

		SAFE_FREE(all_fonts[current_font].name);
		all_fonts[current_font].name = (char *) malloc(strlen(filename) + 1);
		strcpy(all_fonts[current_font].name, filename);

		current_font++;			// 增加队列入口计数
		if (current_font == MAX_FONTS)
			current_font = 0;
	}
	return myfont;
}

// 写字符串
// x,y 坐标
// str 字符串
// color 颜色
// size 字体大小，字形为宋体。 
// fontname 字体名
// charset 字符集 0 GBK 1 big5
// OScharset 0 简体显示 1 繁体显示
//
//#include "myiconv.c"

int Video_DrawText(int x, int y, const char *str, int color, int size, const char *fontname, int charset, int OScharset)
{
	SDL_Color c, c2;
	SDL_Surface *font_surface = NULL;
	int w, h;
	SDL_Rect rect1, rect2, rect_dest;
	SDL_Rect rect;
	char tmp1[256], tmp2[256];
	TTF_Font *myfont;
	SDL_Surface *tempSurface;


	if (strlen(str) > 127) {
		JY_ERROR("string length greater than 127");
		return 0;
	}
	myfont = GetFont(fontname, size);
	if (myfont == NULL)
		return 1;
	c.r = (uint8) ((color & 0xff0000) >> 16);
	c.g = (uint8) ((color & 0xff00) >> 8);
	c.b = (uint8) ((color & 0xff));

#if 0
	c2.r = c.r >> 2;			//阴影色
	c2.b = c.b >> 2;
	c2.g = c.g >> 2;
#endif

	if (charset == 0 && OScharset == 0) {	//GBK -->unicode简体
		JY_CharSet(str, tmp2, 3);
	} else if (charset == 0 && OScharset == 1) {	//GBK -->unicode繁体
		JY_CharSet(str, tmp1, 1);
		JY_CharSet(tmp1, tmp2, 2);
	} else if (charset == 1 && OScharset == 0) {	//big5-->unicode简体
		JY_CharSet(str, tmp1, 0);
		JY_CharSet(tmp1, tmp2, 3);
	} else if (charset == 1 && OScharset == 1) {	////big5-->unicode繁体
		JY_CharSet(str, tmp2, 2);
	} else {
		strcpy(tmp2, str);
	}

	rect = g_fake_screen->clip_rect;

	TTF_SizeUNICODE(myfont, (uint16 *) tmp2, &w, &h);

	if ((x >= rect.x + rect.w) || (x + w + 1) <= rect.x || (y >= rect.y + rect.h) || (y + h + 1) <= rect.y) {	// 超出裁剪范围则不显示
		return 1;
	}

	if (g_ScreenW == 320)
		font_surface = TTF_RenderUNICODE_Solid(myfont, (uint16 *) tmp2, c);	//生成表面
	else
		font_surface = TTF_RenderUNICODE_Blended(myfont, (uint16 *) tmp2, c);	//生成表面

	if (font_surface == NULL)
		return 1;

	rect1.x = (sint16) x;
	rect1.y = (sint16) y;
	rect1.w = (uint16) font_surface->w;
	rect1.h = (uint16) font_surface->h;

	rect2 = rect1;
#if 0
	/* 字体阴影效果 */
	rect2 = rect1;

	rect_dest.x = rect2.x + 1;
	rect_dest.y = rect2.y + 1;
	SDL_SetColors(font_surface, &c2, 1, 1);
	SDL_BlitSurface(font_surface, NULL, g_fake_screen, &rect_dest);	//表面写到游戏表面--阴影色
#else

	rect_dest.x = rect2.x;
	rect_dest.y = rect2.y;
	//SDL_SetColors(font_surface,&c,1,1);
	SDL_BlitSurface(font_surface, NULL, g_fake_screen, &rect_dest);	//表面写到游戏表面 
#endif

	SDL_FreeSurface(font_surface);
	return 0;
}
