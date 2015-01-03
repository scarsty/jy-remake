#ifndef JY_FONT_H
#define JY_FONT_H

#include <SDL_stdinc.h>

void Font_DrawText(SDL_Surface *surface, const char *str, int x, int y);
void Font_SetColor(Uint32 color);
void Font_GetSize(const char *str, int *width, int *height);

#endif // JY_FONT_H

