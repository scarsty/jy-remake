#ifndef JY_IMAGE_H
#define JY_IMAGE_H

#include <SDL_video.h>
#include <SDL_stdinc.h>

static const int PIC_JUST_DRAW  = 0;
static const int PIC_IGNORE_OFFSET = 1;
static const int PIC_DO_ALPHA_BLENDING = 1<<1;
static const int PIC_BLACK = 1<<2;
static const int PIC_WHITE = 1<<3;

SDL_Surface 	*Image_LoadPNG(const char *filename);
SDL_Surface 	*Image_LoadPNG_Mem(Uint8 *mem, size_t length);
SDL_Surface		*Image_LoadJYBMP_Mem(Uint8 *raw, int w, int h, int length);
bool    		Image_IsPNG_Mem(Uint8 *raw);
void 			Image_Free(SDL_Surface *image);
int 			Init_Cache(void);
int 			JY_PicInit(void);
int 			JY_PicLoadFile(const char*idxfilename, const char* grpfilename, int id);
int 			Image_DrawCachedImage(int fileid, int picid, int x,int y,int flag,int value);
int 			JY_GetPicSize(int fileid, int picid, int *w,int *h,int *xoff,int *yoff);
int 			JY_DrawImage(const char *filename,int x,int y);
int 			JY_SetClip(int x1,int y1,int x2,int y2);
int 			BlitSurface(SDL_Surface* surface, int x, int y ,int flag, int value);

#endif // JY_IMAGE_H

