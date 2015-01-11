#ifndef JY_VIDEO_H
#define JY_VIDEO_H

//#include <SDL2/SDL_video.h>
#include "sdlxx.h"

//#define COLOR_KEY 0x706020

static const Uint32 COLOR_KEY = 0x70620;

// Video interface
class Video {
public:
    Video() {}
    virtual ~Video() {}
    static Video * getInstance();
    virtual Uint32 convertColor(Uint32 color) = 0;
    virtual Size getWindowSize() = 0;
    virtual Size getScreenSize() = 0;
    virtual void fillRect(int x1, int y1, int x2, int y2, Uint32 color) = 0;
    virtual void drawRect(int x1, int y1, int x2, int y2, Uint32 color) = 0;
    virtual void darkenRect(int x1, int y1, int x2, int y2, int brightness) = 0;
    virtual void drawText(const char *str, int x, int y, Uint32 color) = 0;
    virtual void fadeIn(Uint32 ms) = 0;
    virtual void fadeOut(Uint32 ms) = 0;
    virtual void updateScreen() = 0;
    virtual SDL_Surface * getSurface() = 0;
    virtual void blitSurface(SDL_Surface *dest, int x, int y, int flag, int value) = 0;
    virtual SDL_Surface *convertSurface(SDL_Surface *surface) = 0;
private:
    static Video* _instance;
};



int		Video_GetWindowWidth(void);
int		Video_GetWindowHeight(void);
int 	Video_Init(int width, int height);
void 	Video_Quit(void);
int		Video_SetMode(int width, int height, SDL_bool fullscreen);
int 	Video_FillColor(int x1,int y1, int x2, int y2, int color);
int 	Video_DrawRect(int x1, int y1, int x2, int y2, int color);
int 	Video_DarkenRect(int x1,int y1,int x2, int y2, int brightness);
int		Video_GetWidth(void);
int		Video_GetHeight(void);
int		Video_GetScreenWidth(void);
int		Video_GetScreenHeight(void);
int 	Video_DrawText(int x, int y, const char *str,int color,int size,const char *fontname, int charset, int OScharset);
void	Video_FadeIn(int ms);
void	Video_FadeOut(int ms);
void	Video_UpdateScreen(void);
Uint32	ConvertColor(Uint32 color);
SDL_Surface *Video_GetCanvas(void);
SDL_Surface *Video_ConvertSurface(SDL_Surface *surface);


#endif // JY_VIDEO_H

