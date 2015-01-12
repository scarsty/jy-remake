/*
================================================================================
script.cpp - 导出给Lua的C函数。
使用Lua 5.1.5做为脚本语言。
================================================================================
*/
// 与lua库的交互函数,使用lua5.1.5版



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif

#include "audio.h"
#include "video.h"
#include "util.h"
#include "input.h"
#include "charset.h"
#include "image.h"
#include "mainmap.h"

//以下为所有包装的lua接口函数，对应于每个实际的函数

static int HAPI_GetDialogString(lua_State *pL)
{
    char buf[1024];
    Uint32 offset = 0;

    RWops file("data/oldtalk.grp", "r");
    offset = lua_tonumber(pL, 1);
    file.seek(offset, RW_SEEK_SET);
    file.read(buf, 1, sizeof(buf));
    *strstr(buf, "\xD\xA") = 0;
    lua_pushstring(pL, buf);
    return 1;
}


static int HAPI_GetFileLength(lua_State *pL)
{
    const char *str = lua_tostring(pL, 1);
    lua_Number length = Util_GetFileLength(str);
    lua_pushnumber(pL, length);
    return 1;
}


static int HAPI_DrawStr(lua_State * pL)
{
	int x = (int) lua_tonumber(pL, 1);
	int y = (int) lua_tonumber(pL, 2);
	const char *str = lua_tostring(pL, 3);
	int color = (int) lua_tonumber(pL, 4);
	int size = (int) lua_tonumber(pL, 5);
	const char *s = lua_tostring(pL, 6);
	int charset = (int) lua_tonumber(pL, 7);
	int OScharset = (int) lua_tonumber(pL, 8);
	Video_DrawText(x, y, str, color, size, "", charset, OScharset);

	return 0;
}

static int HAPI_AudioFadeOut(lua_State * pL)
{
	float msec = (float) lua_tonumber(pL, 1);
	Audio_FadeOut(msec);
	return 0;
}

static int HAPI_FillColor(lua_State * pL)
{
	int x1 = (int) lua_tonumber(pL, 1);
	int y1 = (int) lua_tonumber(pL, 2);
	int x2 = (int) lua_tonumber(pL, 3);
	int y2 = (int) lua_tonumber(pL, 4);
	int color = (int) lua_tonumber(pL, 5);

	Video_FillColor(x1, y1, x2, y2, color);
	return 0;
}

static int HAPI_DarkenRect(lua_State * pL)
{
	int x1 = (int) lua_tonumber(pL, 1);
	int y1 = (int) lua_tonumber(pL, 2);
	int x2 = (int) lua_tonumber(pL, 3);
	int y2 = (int) lua_tonumber(pL, 4);
	int Bright = (int) lua_tonumber(pL, 5);
	Video_DarkenRect(x1, y1, x2, y2, Bright);
	return 0;
}

static int HAPI_DrawRect(lua_State * pL)
{
	int x1 = (int) lua_tonumber(pL, 1);
	int y1 = (int) lua_tonumber(pL, 2);
	int x2 = (int) lua_tonumber(pL, 3);
	int y2 = (int) lua_tonumber(pL, 4);
	int color = (int) lua_tonumber(pL, 5);

	Video_DrawRect(x1, y1, x2, y2, color);
	return 0;
}


static int HAPI_FadeIn(lua_State * pL)
{
	int delay = (int) lua_tonumber(pL, 1);
	Video_FadeIn(delay);
	return 0;
}


static int HAPI_FadeOut(lua_State * pL)
{
	int delay = (int) lua_tonumber(pL, 1);
	Video_FadeOut(delay);
	return 0;
}


static int HAPI_DrawImage(lua_State * pL)
{
	const char *str = lua_tostring(pL, 1);

	int x = (int) lua_tonumber(pL, 2);
	int y = (int) lua_tonumber(pL, 3);

	JY_DrawImage(str, x, y);

	return 0;
}


static int HAPI_GetKey(lua_State * pL)
{
	int key = JY_GetKey();
	if (key == -3) {
		lua_pushstring(pL, "I'm Quiting!");
		lua_error(pL); // 产生异常，从而强制退出。
		return 1;
	}
	lua_pushnumber(pL, key);
	return 1;
}


static int HAPI_EnableKeyRepeat(lua_State * pL)
{
	int delay = (int) lua_tonumber(pL, 1);
	int interval = (int) lua_tonumber(pL, 2);
	//SDL_EnableKeyRepeat(delay, interval);
	return 0;
}


static int HAPI_UpdateScreen(lua_State * pL)
{
	Video_UpdateScreen();
	return 0;
}

static int HAPI_GetTime(lua_State * pL)
{
	double t;
	t = SDL_GetTicks();
	lua_pushnumber(pL, t);
	return 1;
}

static int HAPI_Delay(lua_State * pL)
{
	int x = (int) lua_tonumber(pL, 1);
	SDL_Delay(x);
	return 0;
}

static int HAPI_Log(lua_State * pL)
{
	const char *str = lua_tostring(pL, 1);
	Log(str);
	return 0;
}

static int HAPI_CharSet(lua_State * pL)
{
	size_t length;
	const char *src = lua_tostring(pL, 1);
	int flag = (int) lua_tonumber(pL, 2);
	length = strlen(src);
    MemoryBlock dest(length + 2);

	JY_CharSet(src, (char *)dest.getPtr(), flag);
	lua_pushstring(pL, (char *)dest.getPtr());

	return 1;
}

static int HAPI_SetClip(lua_State * pL)
{

	if (lua_isnoneornil(pL, 1) == 0) {
		int x1 = (int) lua_tonumber(pL, 1);
		int y1 = (int) lua_tonumber(pL, 2);
		int x2 = (int) lua_tonumber(pL, 3);
		int y2 = (int) lua_tonumber(pL, 4);
		JY_SetClip(x1, y1, x2, y2);
	} else {
		JY_SetClip(0, 0, 0, 0);
	}

	return 0;
}

static int HAPI_PlayMIDI(lua_State * pL)
{
	const char *filename = lua_tostring(pL, 1);
	Audio_PlayMIDI(filename);
	return 0;
}

static int HAPI_PlayWAV(lua_State * pL)
{
	const char *filename = lua_tostring(pL, 1);
	Audio_PlayWAV(filename);
	return 0;
}

static int HAPI_PicInit(lua_State * pL)
{
	/*
	   const char *filename;
	   if(lua_isnoneornil(pL,1)==0 )
	   filename=(char*)lua_tostring(pL,1);      
	   else
	   filename="\0";
	 */

	JY_PicInit();

	return 0;
}

static int HAPI_PicLoadFile(lua_State * pL)
{

	const char *idx = lua_tostring(pL, 1);
	const char *grp = lua_tostring(pL, 2);
	int id = (int) lua_tonumber(pL, 3);

	JY_PicLoadFile(idx, grp, id);

	return 0;
}

static int HAPI_DrawCachedImage(lua_State * pL)
{

	int fileid = (int) lua_tonumber(pL, 1);
	int picid = (int) lua_tonumber(pL, 2);
	int x = (int) lua_tonumber(pL, 3);
	int y = (int) lua_tonumber(pL, 4);
	int nooffset = 0;
	int bright = 0;

	if (lua_isnoneornil(pL, 5) == 0)
		nooffset = (int) lua_tonumber(pL, 5);

	if (lua_isnoneornil(pL, 6) == 0)
		bright = (int) lua_tonumber(pL, 6);

	Image_DrawCachedImage(fileid, picid, x, y, nooffset, bright);

	return 0;
}

static int HAPI_GetPicSize(lua_State * pL)
{
	int fileid = (int) lua_tonumber(pL, 1);
	int picid = (int) lua_tonumber(pL, 2);

	int w, h, xoff, yoff;

	JY_GetPicSize(fileid, picid, &w, &h, &xoff, &yoff);
	lua_pushnumber(pL, w);
	lua_pushnumber(pL, h);
	lua_pushnumber(pL, xoff);
	lua_pushnumber(pL, yoff);

	return 4;
}

static int HAPI_LoadMMap(lua_State * pL)
{
	const char *earth = lua_tostring(pL, 1);
	const char *surface = lua_tostring(pL, 2);
	const char *building = lua_tostring(pL, 3);
	const char *buildx = lua_tostring(pL, 4);
	const char *buildy = lua_tostring(pL, 5);
	int xmax = (int) lua_tonumber(pL, 6);
	int ymax = (int) lua_tonumber(pL, 7);
	int x = (int) lua_tonumber(pL, 8);
	int y = (int) lua_tonumber(pL, 9);
	JY_LoadMMap(earth, surface, building, buildx, buildy);

	return 0;
}

static int HAPI_DrawMMap(lua_State * pL)
{
	int x = (int) lua_tonumber(pL, 1);
	int y = (int) lua_tonumber(pL, 2);
	int mypic = (int) lua_tonumber(pL, 3);

	JY_DrawMMap(x, y, mypic);
	return 0;
}

static int HAPI_GetMMap(lua_State * pL)
{
	int x = (int) lua_tonumber(pL, 1);
	int y = (int) lua_tonumber(pL, 2);
	int flag = (int) lua_tonumber(pL, 3);
	int v;
	v = JY_GetMMap(x, y, (BuildingType) flag);
	lua_pushnumber(pL, v);
	return 1;
}

static int HAPI_UnloadMMap(lua_State * pL)
{
	JY_UnloadMMap();
	return 0;
}

#if 0
static int HAPI_FullScreen(lua_State * pL)
{
	JY_FullScreen();
	return 0;
}
#endif

static int HAPI_LoadSMap(lua_State * pL)
{
	const char *Sfilename = lua_tostring(pL, 1);
	const char *Dfilename = lua_tostring(pL, 2);

	//JY_LoadSMap(Sfilename, tempfilename, num, x_max, y_max, Dfilename, d_num1, d_num2);
    JY_LoadSMap(Sfilename, Dfilename);

	return 0;
}

static int HAPI_SaveSMap(lua_State * pL)
{
	const char *Sfilename = lua_tostring(pL, 1);
	const char *Dfilename = lua_tostring(pL, 2);

	JY_SaveSMap(Sfilename, Dfilename);
	return 0;
}

static int HAPI_GetS(lua_State * pL)
{

	int id = (int) lua_tonumber(pL, 1);
	int x = (int) lua_tonumber(pL, 2);
	int y = (int) lua_tonumber(pL, 3);
	int level = (int) lua_tonumber(pL, 4);

	int v;
	v = JY_GetS(id, x, y, level);

	lua_pushnumber(pL, v);
	return 1;

}

static int HAPI_SetS(lua_State * pL)
{

	int id = (int) lua_tonumber(pL, 1);
	int x = (int) lua_tonumber(pL, 2);
	int y = (int) lua_tonumber(pL, 3);
	int level = (int) lua_tonumber(pL, 4);
	int v = (int) lua_tonumber(pL, 5);

	JY_SetS(id, x, y, level, v);

	return 0;

}

static int HAPI_GetD(lua_State * pL)
{

	int Sceneid = (int) lua_tonumber(pL, 1);
	int id = (int) lua_tonumber(pL, 2);
	int i = (int) lua_tonumber(pL, 3);


	int v;
	v = JY_GetD(Sceneid, id, i);

	lua_pushnumber(pL, v);
	return 1;

}

static int HAPI_SetD(lua_State * pL)
{

	int Sceneid = (int) lua_tonumber(pL, 1);
	int id = (int) lua_tonumber(pL, 2);
	int i = (int) lua_tonumber(pL, 3);
	int v = (int) lua_tonumber(pL, 4);


	JY_SetD(Sceneid, id, i, v);

	return 0;

}

static int HAPI_DrawSMap(lua_State * pL)
{
	int sceneid = (int) lua_tonumber(pL, 1);
	int x = (int) lua_tonumber(pL, 2);
	int y = (int) lua_tonumber(pL, 3);
	int xoff = (int) lua_tonumber(pL, 4);
	int yoff = (int) lua_tonumber(pL, 5);
	int mypic = (int) lua_tonumber(pL, 6);

	JY_DrawSMap(sceneid, x, y, xoff, yoff, mypic);

	return 0;
}

static int HAPI_LoadWarMap(lua_State * pL)
{
	const char *WarIDXfilename = lua_tostring(pL, 1);
	const char *WarGRPfilename = lua_tostring(pL, 2);
	int mapid = (int) lua_tonumber(pL, 3);

	JY_LoadWarMap(WarIDXfilename, WarGRPfilename, mapid);

	return 0;
}

static int HAPI_GetWarMap(lua_State * pL)
{

	int x = (int) lua_tonumber(pL, 1);
	int y = (int) lua_tonumber(pL, 2);
	int level = (int) lua_tonumber(pL, 3);

	int v;
	v = JY_GetWarMap(x, y, level);

	lua_pushnumber(pL, v);
	return 1;

}

static int HAPI_SetWarMap(lua_State * pL)
{

	int x = (int) lua_tonumber(pL, 1);
	int y = (int) lua_tonumber(pL, 2);
	int level = (int) lua_tonumber(pL, 3);
	int v = (int) lua_tonumber(pL, 4);
	JY_SetWarMap(x, y, level, v);

	return 0;

}

static int HAPI_CleanWarMap(lua_State * pL)
{

	int level = (int) lua_tonumber(pL, 1);
	int v = (int) lua_tonumber(pL, 2);
	JY_CleanWarMap(level, v);

	return 0;

}

static int HAPI_DrawWarMap(lua_State * pL)
{
	int flag = (int) lua_tonumber(pL, 1);
	int x = (int) lua_tonumber(pL, 2);
	int y = (int) lua_tonumber(pL, 3);
	int v1 = (int) lua_tonumber(pL, 4);
	int v2 = (int) lua_tonumber(pL, 5);
	int v3 = (int) lua_tonumber(pL, 6);
	JY_DrawWarMap(flag, x, y, v1, v2, v3);
	return 0;
}

// byte数组lua函数

/*  lua 调用形式：(注意，位置都是从0开始
     handle=Byte_create(size);
	 Byte_release(h);
	 handle=Byte_loadfile(h,filename,start,length);
     Byte_savefile(h,filename,start,length);
     v=Byte_get16(h,start);
	 Byte_set16(h,start,v);
     v=Byte_getu16(h,start);
	 Byte_setu16(h,start,v);
     v=Byte_get32(h,start);
	 Byte_set32(h,start,v);
	 str=Byte_getstr(h,start,length);
	 Byte_setstr(h,start,length,str);
  */

static int Byte_create(lua_State * pL)
{
	int x = (int) lua_tonumber(pL, 1);
	char *p = (char *) lua_newuserdata(pL, x);	//创建userdata，不需要释放了。
	int i;

	if (p == NULL) {
		DLOG("out of memory");
		return 1;
	}
	for (i = 0; i < x; i++)
		p[i] = 0;

	return 1;
}

static int Byte_loadfile(lua_State * pL)
{
	Uint8 *pData = (Uint8 *) lua_touserdata(pL, 1);
	const char *filename = lua_tostring(pL, 2);
	int start = (int) lua_tonumber(pL, 3);
	int length = (int) lua_tonumber(pL, 4);

    RWops file(filename, "r");
    file.seek(start, RW_SEEK_SET);
    file.read(pData, 1, length);

	return 0;
}

static int Byte_savefile(lua_State * pL)
{
	char *pData = (char *) lua_touserdata(pL, 1);
	const char *filename = lua_tostring(pL, 2);
	int start = (int) lua_tonumber(pL, 3);
	int length = (int) lua_tonumber(pL, 4);

    RWops file(filename, "r");
    file.seek(start, RW_SEEK_SET);
    file.write(pData, 1, length);
	return 0;
}

static int Byte_get16(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);

	short v = *(short *) (p + start);
	lua_pushnumber(pL, v);
	return 1;
}

static int Byte_set16(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);
	short v = (short) lua_tonumber(pL, 3);
	*(short *) (p + start) = v;
	return 0;
}

static int Byte_getu16(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);

	unsigned short v = *(unsigned short *) (p + start);
	lua_pushnumber(pL, v);
	return 1;
}

static int Byte_setu16(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);
	unsigned short v = (unsigned short) lua_tonumber(pL, 3);
	*(unsigned short *) (p + start) = v;
	return 0;

}

static int Byte_get32(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);

	int v = *(int *) (p + start);
	lua_pushnumber(pL, v);
	return 1;
}

static int Byte_set32(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);
	int v = (int) lua_tonumber(pL, 3);
	*(int *) (p + start) = v;
	return 0;
}

static int Byte_getstr(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);
	int length = (int) lua_tonumber(pL, 3);
	//char *s = (char *) Util_malloc(length + 1);
    MemoryBlock smb(length + 1);
    char *s = (char *) smb.getPtr();
	int i;
	for (i = 0; i < length; i++)
		s[i] = p[start + i];

	s[length] = '\0';
	lua_pushstring(pL, s);
	return 1;
}

static int Byte_setstr(lua_State * pL)
{
	char *p = (char *) lua_touserdata(pL, 1);
	int start = (int) lua_tonumber(pL, 2);
	int length = (int) lua_tonumber(pL, 3);
	const char *s = lua_tostring(pL, 4);
	int i;
	int l = (int) strlen(s);
	for (i = 0; i < length; i++)
		p[start + i] = 0;

	if (l > length)
		l = length;

	for (i = 0; i < l; i++)
		p[start + i] = s[i];


	lua_pushstring(pL, s);

	return 1;
}


//定义的lua接口函数名
static const struct luaL_reg jylib[] = {
	{"Log", HAPI_Log},
	{"GetKey", HAPI_GetKey},
	{"EnableKeyRepeat", HAPI_EnableKeyRepeat},
	{"Delay", HAPI_Delay},
	{"GetTime", HAPI_GetTime},
	{"CharSet", HAPI_CharSet},
	{"DrawStr", HAPI_DrawStr},
	{"SetClip", HAPI_SetClip},
	{"FillColor", HAPI_FillColor},
	{"DarkenRect", HAPI_DarkenRect},
	{"DrawRect", HAPI_DrawRect},
	{"UpdateScreen", HAPI_UpdateScreen},
	{"FadeIn", HAPI_FadeIn},
	{"FadeOut", HAPI_FadeOut},
	{"PicInit", HAPI_PicInit},
	{"PicGetXY", HAPI_GetPicSize},
	{"DrawCachedImage", HAPI_DrawCachedImage},
	{"PicLoadFile", HAPI_PicLoadFile},

	//{"FullScreen",      HAPI_FullScreen},

	{"DrawImage", HAPI_DrawImage},

	{"PlayMIDI", HAPI_PlayMIDI},
	{"PlayWAV", HAPI_PlayWAV},

	{"LoadMMap", HAPI_LoadMMap},
	{"DrawMMap", HAPI_DrawMMap},
	{"GetMMap", HAPI_GetMMap},
	{"UnloadMMap", HAPI_UnloadMMap},

	{"LoadSMap", HAPI_LoadSMap},
	{"SaveSMap", HAPI_SaveSMap},
	{"GetS", HAPI_GetS},
	{"SetS", HAPI_SetS},
	{"GetD", HAPI_GetD},
	{"SetD", HAPI_SetD},
	{"DrawSMap", HAPI_DrawSMap},

	{"LoadWarMap", HAPI_LoadWarMap},
	{"GetWarMap", HAPI_GetWarMap},
	{"SetWarMap", HAPI_SetWarMap},
	{"CleanWarMap", HAPI_CleanWarMap},

	{"DrawWarMap", HAPI_DrawWarMap},

	{"AudioFadeOut", HAPI_AudioFadeOut},
    {"GetFileLength", HAPI_GetFileLength},
    {"GetDialogString", HAPI_GetDialogString},

	{NULL, NULL}
};


static const struct luaL_reg bytelib[] = {
	{"create", Byte_create},
	{"loadfile", Byte_loadfile},
	{"savefile", Byte_savefile},
	{"get16", Byte_get16},
	{"set16", Byte_set16},
	{"getu16", Byte_getu16},
	{"setu16", Byte_setu16},
	{"get32", Byte_get32},
	{"set32", Byte_set32},
	{"getstr", Byte_getstr},
	{"setstr", Byte_setstr},
	{NULL, NULL}
};


static void RegisterFunctions(lua_State * pL)
{
	luaL_register(pL, "lib", jylib);
	luaL_register(pL, "Byte", bytelib);
}


static lua_State *theLuaState;


static const char *GetErrorString(int code)
{
	switch (code) {
	case LUA_ERRSYNTAX:
		return "syntax error!";
		break;
	case LUA_ERRMEM:
		return "memory allocation error!";
		break;
	case LUA_ERRFILE:
		return "cannot open file!";
		break;
	default:
		return "Unknown Error";
		break;
	}
}


//Lua主函数
int Script_CallFunction(const char *func_name)
{
	int ret;
	//调用lua的主函数JY_Main    
	lua_getglobal(theLuaState, func_name);
	ret = lua_pcall(theLuaState, 0, 0, 0);

	return 0;
}


//读取lua表中的整型
static int getfield(lua_State * pL, const char *key)
{
	int ret;
	lua_getfield(pL, -1, key);
	ret = (int) lua_tonumber(pL, -1);
	lua_pop(pL, 1);
	return ret;
}


//读取lua表中的字符串
static int getfieldstr(lua_State * pL, const char *key, char *str)
{

	const char *tmp;
	lua_getfield(pL, -1, key);
	tmp = (const char *) lua_tostring(pL, -1);
	strcpy(str, tmp);
	lua_pop(pL, 1);
	return 0;
}


static SDL_bool getfieldboolean(lua_State * pL, const char *name)
{
	SDL_bool ret;
	lua_getfield(pL, -1, name);
	ret = (SDL_bool) lua_toboolean(pL, -1);
	lua_pop(pL, 1);
	return ret;
}


SDL_bool Script_GetBoolean(const char *name)
{
	return getfieldboolean(theLuaState, name);
}


void Script_GetString(const char *name, char *str)
{
	getfieldstr(theLuaState, name, str);
}


int Script_GetInteger(const char *name)
{
	return getfield(theLuaState, name);
}


//Lua读取配置信息
int Script_LoadAndRun(const char *filename)
{
	lua_State *pL = theLuaState;
	int ret = 0;
    MemoryBlock buf(Util_GetFileLength(filename) + 1);
    buf.readFile(filename);
    static_cast<char *>(buf.getPtr())[buf.getSize()] = 0; // add null terminator

	ret = luaL_dostring(pL, static_cast<char *>(buf.getPtr()));

	if (ret) {
		DLOG("Reason: %s", filename, GetErrorString(ret));
        return 1;
	}

	return 0;
}


void Script_GetGlobal(const char *name)
{
	lua_getglobal(theLuaState, name);	//读取config定义的值
}


int Script_Init(void)
{
	if (theLuaState) {
		Log("Script system has been initialized already!");
		return 1;
	}
	theLuaState = lua_open();
    if (!theLuaState) {
        Log("Script_Init() failed.");
        return 1;
    }
	luaL_openlibs(theLuaState);
	RegisterFunctions(theLuaState);

	return 0;
}


void Script_Quit(void)
{
	lua_close(theLuaState);
	theLuaState = NULL;
}

