
// 输出汉字和字符集转换 


//为保证平台兼容性，自己生成了一个gbk简体/繁体/big5/unicode的码表文件
//通过此文件，即可进行各种格式的转换 

#include <SDL.h>
#include "util.h"


//字符集转换数组
static Uint16 gbk_unicode[128][256];
static Uint16 gbk_big5[128][256];
static Uint16 big5_gbk[128][256];
static Uint16 big5_unicode[128][256];

//加载码表文件
//码表文件顺序： 按GBK排列，unicode，big5。然后按big5排列，unicode和gbk。
static int LoadMB(const char *mbfile)
{
	int i, j;
	SDL_RWops *rwops = NULL;

	Uint16 gbk, big5, unicode;

	rwops = SDL_RWFromFile(mbfile, "r");
	if (!rwops) {
		DLOG("cannot open file: %s", mbfile);
		return 1;
	}

	for (i = 0; i < 128; i++) {
		for (j = 0; j < 256; j++) {
			gbk_unicode[i][j] = 0;
			gbk_big5[i][j] = 0;
			big5_gbk[i][j] = 0;
			big5_unicode[i][j] = 0;
		}
	}

	for (i = 0x81; i <= 0xfe; i++) {
		for (j = 0x40; j <= 0xfe; j++) {
			if (j != 0x7f) {
				SDL_RWread(rwops, &unicode, 2, 1);
				SDL_RWread(rwops, &big5, 2, 1);
				gbk_unicode[i - 128][j] = unicode;
				gbk_big5[i - 128][j] = big5;
			}
		}
	}

	for (i = 0xa0; i <= 0xfe; i++) {
		for (j = 0x40; j <= 0xfe; j++) {
			if (j <= 0x7e || j >= 0xa1) {
				SDL_RWread(rwops, &unicode, 2, 1);
				SDL_RWread(rwops, &gbk, 2, 1);
				big5_unicode[i - 128][j] = unicode;
				big5_gbk[i - 128][j] = gbk;
			}
		}
	}

	SDL_RWclose(rwops);

	return 0;
}
// 汉字字符集转换
// flag = 0   Big5 --> GBK     
//      = 1   GBK  --> Big5    
//      = 2   Big5 --> Unicode
//      = 3   GBK  --> Unicode
// 注意要保证dest有足够的空间，一般建议取src长度的两倍+2，保证全英文字符也能转化为unicode
int JY_CharSet(const char *src, char *dest, int flag)
{
    static bool virgin = true;

    if (virgin) {
        LoadMB("data/hzmb.dat");
        virgin = false;
    }

	Uint8 *psrc, *pdest;
	Uint8 b0, b1;
	int d0;
	Uint16 tmpchar;

	psrc = (Uint8 *) src;
	pdest = (Uint8 *) dest;

	for (;;) {
		b0 = *psrc;
		if (b0 == 0) {			//字符串结束
			if ((flag == 0) || (flag == 1)) {
				*pdest = 0;
				break;
			} else {			//unicode结束标志 0x0000?
				*pdest = 0;
				*(pdest + 1) = 0;
				break;
			}
		}
		if (b0 < 128) {			//英文字符
			if ((flag == 0) || (flag == 1)) {	//不转换
				*pdest = b0;
				pdest++;
				psrc++;
			} else {			//unicode 后面加个0
				*pdest = b0;
				pdest++;
				*pdest = 0;
				pdest++;
				psrc++;
			}
		} else {				//中文字符
			b1 = *(psrc + 1);
			if (b1 == 0) {		// 非正常结束
				*pdest = '?';
				*(pdest + 1) = 0;
				break;
			} else {
				d0 = b0 + b1 * 256;
				switch (flag) {
				case 0:		//Big5 --> GBK    
					tmpchar = big5_gbk[b0 - 128][b1];
					break;
				case 1:		//GBK  --> Big5  
					tmpchar = gbk_big5[b0 - 128][b1];
					break;
				case 2:		//Big5 --> Unicode
					tmpchar = big5_unicode[b0 - 128][b1];
					break;
				case 3:		//GBK  --> Unicode
					tmpchar = gbk_unicode[b0 - 128][b1];
					break;
				default:
					tmpchar = 0;
					break;
				}

				if (tmpchar != 0) {
					*(Uint16 *) pdest = tmpchar;
				} else {
					*pdest = '?';
					*(pdest + 1) = '?';
				}

				pdest = pdest + 2;
				psrc = psrc + 2;
			}
		}
	}

	return 0;
}


