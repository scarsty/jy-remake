/*
 * GB2312编码, 16 x 16 点阵汉字支持。
 *
 *
 */

#include <SDL.h>
#include <string.h>

static Uint8 fontData[] =
#include "hzkdata.cpp"
;

static Uint8 asciiData[] = 
#include "asciidata.cpp"
;

// 32 - 127
static void DrawAsciiChar2(SDL_Renderer *renderer, const Uint8 *ptr, int x, int y)
{
    int i, j, k, offset;
    int flag;
    //unsigned char buffer[32];
    Uint8 *buffer = NULL;
    Uint8 key[8] = {
        1<<7, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2, 1<<1, 1
    };
    SDL_Point points[128]; // 每个字模最多128个点。
    int numPts = 0;

    offset = (*ptr - 32) * 16;
    buffer = &asciiData[offset];

    for(k=0; k<16; k++) {
        for(i=0; i<8; i++){
            flag = buffer[k] & key[i];
            if (flag) {
                points[numPts].x = i + x;
                points[numPts].y = k + y;
                numPts++;
            }
        }
    }
    SDL_RenderDrawPoints(renderer, points, numPts);
}

static Uint32 drawColor = 0;

void Font_SetColor(Uint32 color)
{
    drawColor = color;
}

static inline void putpixel32(SDL_Surface *surface, int x, int y)
{
    if (x>0 && x<surface->w && y>0 && y<surface->h) {
        Uint8 *vaddr = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
        *(Uint32 *)vaddr = drawColor;
    }
}

static void DrawAsciiChar(SDL_Surface *surface, const Uint8 *ptr, int x, int y)
{
    int i, j, k, offset;
    int flag;
    //unsigned char buffer[32];
    Uint8 *buffer = NULL;
    Uint8 key[8] = {
        1<<7, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2, 1<<1, 1
    };
    SDL_Point points[128]; // 每个字模最多128个点。
    int numPts = 0;

    offset = (*ptr - 32) * 16;
    buffer = &asciiData[offset];

    for(k=0; k<16; k++) {
        for(i=0; i<8; i++){
            flag = buffer[k] & key[i];
            if (flag) {
                putpixel32(surface, i + x, k + y);
            }
        }
    }
}

static void DrawGbkChar2(SDL_Renderer *renderer, const Uint8 *word, int x, int y)
{
    int i, j, k, offset;
    int flag;
    //unsigned char buffer[32];
    unsigned char *buffer = NULL;
    //unsigned char word[3] = "Ａ";
    unsigned char key[8] = {
        1<<7, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2, 1<<1, 1
    };
    SDL_Point points[256];
    int numPts;

    //offset = (94*(unsigned int)(word[0]-0xa0-1)+(word[1]-0xa0-1))*32;
    offset = (94*(Uint32)(word[0]-0xa0-1)+(word[1]-0xa0-1))*32;
    buffer = &fontData[offset];

    for(k=0; k<16; k++) {
        for(j=0; j<2; j++){
            for(i=0; i<8; i++){
                flag = buffer[k*2+j] & key[i];
                if (flag) {
                    points[numPts].x = j*8 + i + x;
                    points[numPts].y = k + y;
                    numPts++;
                }
            }
        }
    }

    SDL_RenderDrawPoints(renderer, points, numPts);
}

static void DrawGbkChar(SDL_Surface *surface, const Uint8 *word, int x, int y)
{
    int i, j, k, offset;
    int flag;
    //unsigned char buffer[32];
    unsigned char *buffer = NULL;
    //unsigned char word[3] = "Ａ";
    unsigned char key[8] = {
        1<<7, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2, 1<<1, 1
    };

    //offset = (94*(unsigned int)(word[0]-0xa0-1)+(word[1]-0xa0-1))*32;
    offset = (94*(Uint32)(word[0]-0xa0-1)+(word[1]-0xa0-1))*32;
    buffer = &fontData[offset];

    for(k=0; k<16; k++) {
        for(j=0; j<2; j++){
            for(i=0; i<8; i++){
                flag = buffer[k*2+j] & key[i];
                if (flag) {
                    putpixel32(surface, j*8 + i + x, k + y);
                }
            }
        }
    }
}


void Font_DrawText2(SDL_Renderer *renderer, const char *str, int x, int y)
{
    const unsigned char *p = (const unsigned char *) str;
    while (*p) {
        if (*p<128) {
            DrawAsciiChar2(renderer, p, x, y);
            p++;
            x += 8;
        } else {
            DrawGbkChar2(renderer, p, x, y);
            p+=2;
            x += 16;
        }
    }
}


void Font_DrawText(SDL_Surface *surface, const char *str, int x, int y)
{
    const unsigned char *p = (const unsigned char *)str;
    while (*p) {
        if (*p<128) {
            DrawAsciiChar(surface, p, x, y);
            p++;
            x += 8;
        } else {
            DrawGbkChar(surface, p, x, y);
            p+=2;
            x += 16;
        }
    }
}


void Font_GetSize(const char *str, int *width, int *height)
{
    *width = strlen(str) * 8;
    *height = 16;
}


