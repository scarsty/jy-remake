//////////////////////////////////////////////////////////////////////////////////////////
// World map, Scene Map, Battle Map operations
//////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <memory>
#include <algorithm>
#include "image.h"
#include "video.h"
#include "util.h"
#include "mainmap.h"


static const int kHalfTileWidth = 18;
static const int kHalfTileHeight = 9;


BattleMap theWarMap;
SceneMap theSceneMap;
WorldMap theWorldMap;


//////////////////////////////////////////////////////////////////////////////////////////
// SceneMap
//////////////////////////////////////////////////////////////////////////////////////////

int SceneMap::load(const char *sfname, const char *dfname)
{
    //_mapData->create(sfname);
    //_eventData->create(dfname);
    _mapData.reset(new MemoryBlock(sfname));
    _eventData.reset(new MemoryBlock(dfname));
}


int SceneMap::save(const char *sceneFilename, const char *eventFilename)
{
    std::auto_ptr<RWops> rw(new RWops(sceneFilename, "w"));
    rw->write(_mapData->getPtr(), 1, _mapData->getSize());


    rw.reset(new RWops(eventFilename, "w"));
    rw->write(_mapData->getPtr(), 1, _eventData->getSize());
    rw.reset(NULL);

	return 0;
}


void SceneMap::unload()
{
    //_mapData->destroy();
    //_eventData->destroy();
    _mapData.reset(NULL);
    _eventData.reset(NULL);
}


int SceneMap::getS(int id, int x, int y, int level)
{
	size_t s = 0;
	if (id < 0 || id >= kNumMaps || x < 0 || x >= kWidth || 
            y < 0 || y >= kHeight || level < 0 || level >= kNumLevels) {
        DLOG("data out of range! id=%d, x=%d, y=%d, level=%d", id, x, y, level);
		return 0;
	}
	s = kWidth * kHeight * (id * kNumLevels + level) + y * kWidth + x;
    return static_cast<Sint16 *>(_mapData->getPtr())[s];
}


int SceneMap::setS(int id, int x, int y, int level, int v)
{
	int s;
	short value = (short) v;
	if (id < 0 || id >= kNumMaps || x < 0 || x >= kWidth || 
            y < 0 || y >= kHeight || level < 0 || level >= kNumLevels) {
		DLOG("data out of range! id=%d, x=%d, y=%d, level=%d", id, x, y, level);
		return 0;
	}

	s = kWidth * kHeight * (id * kNumLevels + level) + y * kWidth + x;
    static_cast<Sint16 *>(_mapData->getPtr())[s] = value;
	return 0;
}


//存D*
int SceneMap::setD(int sceneid, int id, int i, int v)
{
	size_t s = 0;
	if (sceneid < 0 || sceneid >= kNumMaps) {
		DLOG("sceneid=%d, out of range!", sceneid);
		return 0;
	}
	s = kNumEvents * kNumEventArgs * sceneid + id * kNumEventArgs + i;
    static_cast<Sint16 *>(_eventData->getPtr())[s] = v;
	return 0;
}


//取D*
int SceneMap::getD(int sceneid, int id, int i)
{
	size_t s = 0;
	if (sceneid < 0 || sceneid >= kNumMaps) {
		Log(va("JY_GetD(): sceneid=%d out of range!", sceneid));
		return 0;
	}

    Sint16 * const &p = static_cast<Sint16 *>(_eventData->getPtr());

	s = kNumEvents * kNumEventArgs * sceneid + id * kNumEventArgs + i;

    //return reinterpret_cast<Sint16 *>(_eventData->getPtr())[s];
    //return static_cast<Sint16 *>(_eventData->getPtr())[s];
    return p[s];
}


// 绘制场景地图
int SceneMap::draw(int sceneid, int x, int y, int xoff, int yoff, int mypic)
{

	//int rangex=Video_GetScreenWidth()/(2*kHalfTileWidth)/2+1+kExtraWidth;
	//int rangey=(Video_GetScreenHeight())/(2*kHalfTileHeight)/2+1;
	int i, j;
	int i1, j1;
	int x1, y1;
	int xx, yy;
	int istart, iend, jstart, jend;
	SDL_Rect rect;

	rect = Video_GetCanvas()->clip_rect;
	//根据Video_GetCanvas()的剪裁来确定循环参数。提高绘制速度
	istart = (rect.x - Video_GetScreenWidth() / 2) / 
        (2 * kHalfTileWidth) - 1 - kExtraWidth;
	iend = (rect.x + rect.w - Video_GetScreenWidth() / 2) / 
        (2 * kHalfTileWidth) + 1 + kExtraWidth;

	jstart = (rect.y - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) - 1;
	jend = (rect.y + rect.h - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) + 1;

	Video_FillColor(0, 0, 0, 0, 0);

	for (j = 0; j <= 2 * jend - 2 * jstart + kExtraHeight; j++) {
		for (i = istart; i <= iend; i++) {
			i1 = i + j / 2 + jstart;
			j1 = -i + j / 2 + j % 2 + jstart;


			x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
			y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;

			xx = x + i1 + xoff;
			yy = y + j1 + yoff;

			if ((xx >= 0) && (xx < kWidth) && (yy >= 0) && (yy < kHeight)) {
				int d0 = JY_GetS(sceneid, xx, yy, 0);
				if (d0 > 0) {
					Image_DrawCachedImage(0, d0, x1, y1, PIC_JUST_DRAW, 0);	//地面
				}
			}
		}
	}

	for (j = 0; j <= 2 * jend - 2 * jstart + kExtraHeight; j++) {
		for (i = istart; i <= iend; i++) {
			i1 = i + j / 2 + jstart;
			j1 = -i + j / 2 + j % 2 + jstart;

			x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
			y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;

			xx = x + i1 + xoff;
			yy = y + j1 + yoff;

			if ((xx >= 0) && (xx < kWidth) && (yy >= 0) && (yy < kHeight)) {
				int d1 = JY_GetS(sceneid, xx, yy, 1);
				int d2 = JY_GetS(sceneid, xx, yy, 2);
				int d3 = JY_GetS(sceneid, xx, yy, 3);
				int d4 = JY_GetS(sceneid, xx, yy, 4);
				int d5 = JY_GetS(sceneid, xx, yy, 5);

				if (d1 > 0) {
					Image_DrawCachedImage(0, d1, x1, y1 - d4, PIC_JUST_DRAW, 0);	//建筑
				}
				if (d2 > 0) {
					Image_DrawCachedImage(0, d2, x1, y1 - d5, PIC_JUST_DRAW, 0);	//空中
				}
				if (d3 >= 0) {
					int picnum = JY_GetD(sceneid, d3, 7);
					if (picnum > 0) {
						Image_DrawCachedImage(0, picnum, x1, y1 - d4, PIC_JUST_DRAW, 0);
					}
				}

				if ((i1 == -xoff) && (j1 == -yoff)) {	//主角
					Image_DrawCachedImage(0, mypic * 2, x1, y1 - d4, PIC_JUST_DRAW, 0);
				}
			}
		}
	}

	return 0;
}


SceneMap::SceneMap()
{
}


SceneMap::~SceneMap()
{
    unload();
}

//////////////////////////////////////////////////////////////////////////////////////////
// BattleMap
//////////////////////////////////////////////////////////////////////////////////////////

//加载战斗地图
// WarIDXfilename/WarGRPfilename 战斗地图idx/grp文件名
// mapid 战斗地图编号
// num 战斗地图数据层数   应为6
//         0层 地面数据
//         1层 建筑
//         2层 战斗人战斗编号
//         3层 移动时显示可移动的位置
//         4层 命中效果
//         5层 战斗人对应的贴图
// x_max,x_max   地图大小
int BattleMap::load(const char *idxFilename, const char *grpFilename, int mapid)
{
    unload();

	int p;

	if (pWar == NULL)
		pWar = (Sint16 *) Util_malloc(kWidth * kHeight * kNumLevels * 2);

	if (mapid == 0) {			//第0个地图，从0开始读
		p = 0;
	} else {
        RWops file(idxFilename, "r");
        file.seek(4 * (mapid - 1), RW_SEEK_SET);
        file.read(&p, 4, 1);
	};

    RWops file(grpFilename, "r");
    file.seek(p, RW_SEEK_SET);
    file.read(pWar, 2, kWidth * kHeight * 2);

	return 0;
}


void BattleMap::unload()
{
    Util_free(pWar);
}


BattleMap::BattleMap() : pWar(0)
{
}


BattleMap::~BattleMap()
{
    unload();
}

// 绘制战斗地图
// flag=0  绘制基本战斗地图
//     =1  显示可移动的路径，(v1,v2)当前移动坐标，白色背景(雪地战斗)
//     =2  显示可移动的路径，(v1,v2)当前移动坐标，黑色背景
//     =3  命中的人物用白色轮廓显示
//     =4  战斗动作动画  v1 战斗人物pic, v2贴图所属的加载文件id
//                       v3 武功效果pic  -1表示没有武功效果

int BattleMap::draw(int flag, int x, int y, int v1, int v2, int v3)
{
	int i, j;
	int i1, j1;
	int x1, y1;
	int xx, yy;
	int istart, iend, jstart, jend;
	SDL_Rect rect;

	rect = Video_GetCanvas()->clip_rect;
	istart = (rect.x - Video_GetScreenWidth() / 2) / (2 * kHalfTileWidth) - 1 - kExtraWidth;
	iend = (rect.x + rect.w - Video_GetScreenWidth() / 2) / (2 * kHalfTileWidth) + 1 + kExtraWidth;
	jstart = (rect.y - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) - 1;
	jend = (rect.y + rect.h - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) + 1;
	Video_FillColor(0, 0, 0, 0, 0);

	// 绘战斗地面
	for (j = 0; j <= 2 * jend - 2 * jstart + kExtraHeight; j++) {
		for (i = istart; i <= iend; i++) {
			i1 = i + j / 2 + jstart;
			j1 = -i + j / 2 + j % 2 + jstart;

			x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
			y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;
			xx = x + i1;
			yy = y + j1;
			if ((xx >= 0) && (xx < kWidth) && (yy >= 0) && (yy < kHeight)) {
				int num = JY_GetWarMap(xx, yy, 0);
				if (num > 0)
					Image_DrawCachedImage(0, num, x1, y1, PIC_JUST_DRAW, 0);	//地面
			}
		}
	}

	if ((flag == 1) || (flag == 2)) {	//在地面上绘制移动范围
		for (j = 0; j <= 2 * jend - 2 * jstart + kExtraHeight; j++) {
			for (i = istart; i <= iend; i++) {
				i1 = i + j / 2 + jstart;
				j1 = -i + j / 2 + j % 2 + jstart;

				x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
				y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;
				xx = x + i1;
				yy = y + j1;
				if ((xx >= 0) && (xx < kWidth) && (yy >= 0) && (yy < kHeight)) {
					if (JY_GetWarMap(xx, yy, 3) < 128) {
						int showflag;

						if (flag == 1)
							showflag = PIC_DO_ALPHA_BLENDING | PIC_BLACK;
						else
							showflag = PIC_DO_ALPHA_BLENDING | PIC_WHITE;

						if ((xx == v1) && (yy == v2))

							Image_DrawCachedImage(0, 0, x1, y1, showflag, 128);
						else
							Image_DrawCachedImage(0, 0, x1, y1, showflag, 64);
					}
				}
			}
		}
	}
	// 绘战斗建筑和人
	for (j = 0; j <= 2 * jend - 2 * jstart + kExtraHeight; j++) {
		for (i = istart; i <= iend; i++) {
			i1 = i + j / 2 + jstart;
			j1 = -i + j / 2 + j % 2 + jstart;

			x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
			y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;
			xx = x + i1;
			yy = y + j1;
			if ((xx >= 0) && (xx < kWidth) && (yy >= 0) && (yy < kHeight)) {
				int num = JY_GetWarMap(xx, yy, 1);	//  建筑      
				if (num > 0)
					Image_DrawCachedImage(0, num, x1, y1, 0, 0);

				num = JY_GetWarMap(xx, yy, 2);	// 战斗人
				if (num >= 0) {
					int pic = JY_GetWarMap(xx, yy, 5);	// 人贴图
					if (pic >= 0) {
						switch (flag) {
						case 0:
						case 1:
						case 2:
						case 5:	//人物常规显示
							Image_DrawCachedImage(0, pic, x1, y1, PIC_JUST_DRAW, 0);
							break;
						case 3:
							if (JY_GetWarMap(xx, yy, 4) > 1)	//命中
								Image_DrawCachedImage(0, pic, x1, y1, PIC_BLACK | PIC_DO_ALPHA_BLENDING, 255);	//变黑
							else
								Image_DrawCachedImage(0, pic, x1, y1, PIC_JUST_DRAW, 0);

							break;
						case 4:
							if ((xx == x) && (yy == y)) {
								if (v2 == 0)
									Image_DrawCachedImage(0, pic, x1, y1, PIC_JUST_DRAW, 0);
								else
									Image_DrawCachedImage(v2, v1, x1, y1, PIC_JUST_DRAW, 0);
							} else {
								Image_DrawCachedImage(0, pic, x1, y1, PIC_JUST_DRAW, 0);
							}

							break;
						}
					}
				}

				if (flag == 4 && v3 >= 0) {	//武功效果
					int effect = JY_GetWarMap(xx, yy, 4);
					if (effect > 0) {
						Image_DrawCachedImage(3, v3, x1, y1, PIC_JUST_DRAW, 0);
					}
				}
			}
		}
	}
	return 0;
}


int BattleMap::clean(int level, int v)
{
	Sint16 *p = pWar + kWidth * kHeight * level;
	int i;
	for (i = 0; i < kWidth * kHeight; i++) {
		*p = (Sint16) v;
		p++;
	}
	return 0;
}


int BattleMap::set(int x, int y, int level, int v)
{
	size_t s = 0;

	if (x < 0 || x >= kWidth || y < 0 || y >= kHeight || level < 0 || level >= 6) {
		DLOG("data out of range! x=%d, y=%d, level=%d", x, y, level);
		return 0;
	}
	s = kWidth * kHeight * level + y * kWidth + x;
	//*(pWar + s) = (short) v;
    pWar[s] = static_cast<Sint16>(v);
	return 0;
}


int BattleMap::get(int x, int y, int level)
{
	size_t s = 0;

	if (x < 0 || x >= kWidth || y < 0 || y >= kHeight || level < 0 || level >= 6) {
		DLOG("data out of range");
		return 0;
	}
	s = kWidth * kHeight * level + y * kWidth + x;
	//return *(pWar + s);
    return pWar[s];
}


//////////////////////////////////////////////////////////////////////////////////////////
// WorldMap
//////////////////////////////////////////////////////////////////////////////////////////


WorldMap::WorldMap()
{
}


WorldMap::~WorldMap()
{
    unload();
}

// 释放主地图数据
int JY_UnloadMMap(void)
{
    return theWorldMap.unload();
}


size_t WorldMap::coord2Index(int x, int y)
{
	size_t s = y * kWidth + x;
	return s;
}


int WorldMap::unload()
{
    _earthLayerData.reset(NULL);
    _surfaceLayerData.reset(NULL);
    _buildingLayerData.reset(NULL);
    _buildxLayerData.reset(NULL);
    _buildyLayerData.reset(NULL);
    return 0;
}


/**
 * 加载地图时，把地图需要用到的贴图也加载，这样更符合人的思维习惯。
 */

int WorldMap::load(const char *earthname, const char *surfacename, const char *buildingname,
        const char *buildxname, const char *buildyname)
{
    //unload();
    if (_earthLayerData.get() && _earthLayerData->getPtr()) {
        DLOG("_earthLayerData already loaded.");
        return 1;
    }
    _earthLayerData.reset(new MemoryBlock(earthname));
    _surfaceLayerData.reset(new MemoryBlock(surfacename));
    _buildingLayerData.reset(new MemoryBlock(buildingname));
    _buildxLayerData.reset(new MemoryBlock(buildxname));
    _buildyLayerData.reset(new MemoryBlock(buildyname));


    
#if 0
    if (_earthLayerData->getPtr() != NULL) {
        DLOG("_earthLayerData already loaded.");
    }

    if (_earthLayerData.create(earthname) ||
            _surfaceLayerData.create(surfacename) ||
            _buildingLayerData.create(buildingname) ||
            _buildxLayerData.create(buildxname) ||
            _buildyLayerData.create(buildyname))
        goto out;
    return 0;

out:
    Log("Error loading MMap.");
    unload();
    return 1;
#endif
}


int WorldMap::get(int x, int y, BuildingType flag)
{
	size_t s = 0;
	int v = 0;

	s = coord2Index(x, y);

	switch (flag) {
	case kLayerEarth:
		v = static_cast<Sint16 *>(_earthLayerData->getPtr())[s];
		break;

	case kLayerSurface:
		v = static_cast<Sint16 *>(_surfaceLayerData->getPtr())[s];
		break;
	case kLayerBuilding:
		v = static_cast<Sint16 *>(_buildingLayerData->getPtr())[s];
		break;
	case kLayerBuildx:
		v = static_cast<Sint16 *>(_buildxLayerData->getPtr())[s];
		break;
	case kLayerBuildy:
		v = static_cast<Sint16 *>(_buildyLayerData->getPtr())[s];
		break;
	}
	return v;
}


int WorldMap::set(short x, short y, BuildingType flag, short v)
{
	size_t s = 0;

	s = coord2Index(x, y);

	switch (flag) {
	case kLayerEarth:
        static_cast<Sint16 *>(_earthLayerData->getPtr())[s] = v;
		break;
	case kLayerSurface: static_cast<Sint16 *>(_surfaceLayerData->getPtr())[s] = v;
		break;
	case kLayerBuilding:
        static_cast<Sint16 *>(_buildingLayerData->getPtr())[s] = v;
		break;
	case kLayerBuildx:
        static_cast<Sint16 *>(_buildxLayerData->getPtr())[s] = v;
		break;
	case kLayerBuildy:
        static_cast<Sint16 *>(_buildyLayerData->getPtr())[s] = v;
		break;
	}
	return 0;
}


// 主地图建筑排序 
// x,y 主角坐标
// mypic 主角贴图编号
// screen_w, screen_h: Screen width and height in pixel.
// extra_x, extra_y: extra map width and height in tiles.

void WorldMap::sortBuildings(short x, short y, short mypic, int screen_w, int screen_h, 
        int extra_x, int extra_y, int half_tile_w, int half_tile_h)
{

	int rangex = screen_w / (2 * half_tile_w) / 2 + 1 + extra_x;
	int rangey = screen_h / (2 * half_tile_h) / 2 + 1;

	int range = rangex + rangey + extra_y;

	short bak = (short) JY_GetMMap(x, y, kLayerBuilding);
	short bakx = (short) JY_GetMMap(x, y, kLayerBuildx);
	short baky = (short) JY_GetMMap(x, y, kLayerBuildy);

	int xmin = clamp(x - range, 1, kWidth - 1);
	int xmax = clamp(x + range, 1, kWidth - 1);
	int ymin = clamp(y - range, 1, kHeight - 1);
	int ymax = clamp(y + range, 1, kHeight - 1);

	int i, j, k, m;
	int dy;
	bool repeat = false;
    std::list<TBuilding>& sb = _sortedBuildings;
	//int p = 0;
    sb.clear();

	JY_SetMMap(x, y, kLayerBuilding, (short) (mypic * 2));
	JY_SetMMap(x, y, kLayerBuildx, x);
	JY_SetMMap(x, y, kLayerBuildy, y);

	for (i = xmin; i <= xmax; i++) {
		dy = ymin;
		for (j = ymin; j <= ymax; j++) {
			int xoff = JY_GetMMap(i, j, kLayerBuildx);
			int yoff = JY_GetMMap(i, j, kLayerBuildy);
			if ((xoff != 0) && (yoff != 0)) {
				repeat = false;
                std::list<TBuilding>::iterator it;
                for (it = sb.begin(); it != sb.end(); ++it) {
                    if (it->x == xoff && it->y == yoff) {
                        repeat = true;
                        if (it == --sb.end())
                            break;
                        for (m = j - 1; m >= dy; m--) {
                            int im3 = JY_GetMMap(i, m, kLayerBuildx);
                            int im4 = JY_GetMMap(i, m, kLayerBuildy);
                            if (im3 != 0 && im4 != 0) {
                                if (im3 != xoff || im4 != yoff) {
                                    if (im3 != it->x || im4 != it->y) {
                                        std::list<TBuilding>::iterator last = --sb.end();
                                        sb.insert(it, *last);
                                        sb.erase(last);
                                        it = --it;
                                    }
                                }
                            }
                        }
                        dy = j + 1;
                        break;
                    }
                }
                if (!repeat) {
                    sb.insert(sb.end(),
                        TBuilding(xoff, yoff, JY_GetMMap(xoff, yoff, 
                                kLayerBuilding)));
                }
			}
		}
	}
	JY_SetMMap(x, y, kLayerBuilding, bak);
	JY_SetMMap(x, y, kLayerBuildx, bakx);
	JY_SetMMap(x, y, kLayerBuildy, baky);
}


void WorldMap::draw(int x, int y, int mypic)
{

	int i, j;
	int i1, j1;
	int x1, y1;
	int picnum;
	int istart, iend, jstart, jend;
	SDL_Rect rect;

	rect = Video_GetCanvas()->clip_rect;
	//根据Video_GetCanvas()的clip来确定循环参数。提高绘制速度
	istart = (rect.x - Video_GetScreenWidth() / 2) / (2 * kHalfTileWidth) - 1 - kExtraWidth;
	iend = (rect.x + rect.w - Video_GetScreenWidth() / 2) / (2 * kHalfTileWidth) + 1 + kExtraWidth;
	jstart = (rect.y - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) - 1;
	jend = (rect.y + rect.h - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) + 1;

	//建筑排序
	sortBuildings((short) x, (short) y, (short) mypic, 
            Video_GetScreenWidth(), Video_GetScreenHeight(), 
            kExtraWidth, kExtraHeight, kHalfTileWidth, kHalfTileHeight);
	Video_FillColor(0, 0, 0, 0, 0);
	for (j = 0; j <= 2 * jend - 2 * jstart + kExtraHeight; j++) {
		for (i = istart; i <= iend; i++) {
			i1 = i + j / 2 + jstart;
			j1 = -i + j / 2 + j % 2 + jstart;

			x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
			y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;

			if (((x + i1) >= 0) && ((x + i1) < kWidth) && ((y + j1) >= 0) && ((y + j1) < kHeight)) {
				picnum = JY_GetMMap(x + i1, y + j1, kLayerEarth);
				if (picnum > 0) {
					Image_DrawCachedImage(0, picnum, x1, y1, PIC_JUST_DRAW, 0);
				}
				picnum = JY_GetMMap(x + i1, y + j1, kLayerSurface);
				if (picnum > 0) {
					Image_DrawCachedImage(0, picnum, x1, y1, PIC_JUST_DRAW, 0);
				}
			}
		}
	}

    std::list<TBuilding>& sb = _sortedBuildings;
    std::list<TBuilding>::iterator it;
    for (it = sb.begin(); it != sb.end(); ++it) {
        i1 = it->x - x;
        j1 = it->y - y;
		x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
		y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;
		picnum = it->num;
		if (picnum > 0) {
			Image_DrawCachedImage(0, picnum, x1, y1, PIC_JUST_DRAW, 0);
		}
	}
}
#if 0 
void WorldMap::draw(int x, int y, int mypic)
{

	int i, j;
	int i1, j1;
	int x1, y1;
	int picnum;
	int istart, iend, jstart, jend;
	SDL_Rect rect;

	rect = Video_GetCanvas()->clip_rect;
	//根据Video_GetCanvas()的clip来确定循环参数。提高绘制速度
	istart = (rect.x - Video_GetScreenWidth() / 2) / (2 * kHalfTileWidth) - 1 - kExtraWidth;
	iend = (rect.x + rect.w - Video_GetScreenWidth() / 2) / (2 * kHalfTileWidth) + 1 + kExtraWidth;
	jstart = (rect.y - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) - 1;
	jend = (rect.y + rect.h - Video_GetScreenHeight() / 2) / (2 * kHalfTileHeight) + 1;

	//建筑排序
	sortBuildings((short) x, (short) y, (short) mypic, 
            Video_GetScreenWidth(), Video_GetScreenHeight(), 
            kExtraWidth, kExtraHeight, kHalfTileWidth, kHalfTileHeight);
	Video_FillColor(0, 0, 0, 0, 0);
	for (j = 0; j <= 2 * jend - 2 * jstart + kExtraHeight; j++) {
		for (i = istart; i <= iend; i++) {
			i1 = i + j / 2 + jstart;
			j1 = -i + j / 2 + j % 2 + jstart;

			x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
			y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;

			if (((x + i1) >= 0) && ((x + i1) < kWidth) && ((y + j1) >= 0) && ((y + j1) < kHeight)) {
				picnum = JY_GetMMap(x + i1, y + j1, kLayerEarth);
				if (picnum > 0) {
					Image_DrawCachedImage(0, picnum, x1, y1, PIC_JUST_DRAW, 0);
				}
				picnum = JY_GetMMap(x + i1, y + j1, kLayerSurface);
				if (picnum > 0) {
					Image_DrawCachedImage(0, picnum, x1, y1, PIC_JUST_DRAW, 0);
				}
			}
		}
	}

    std::list<TBuilding>& sb = _sortedBuildings;
    std::list<TBuilding>::iterator it;
    for (it = sb.begin(); it != sb.end(); ++it) {
        i1 = it->x - x;
        j1 = it->y - y;
		x1 = kHalfTileWidth * (i1 - j1) + Video_GetScreenWidth() / 2;
		y1 = kHalfTileHeight * (i1 + j1) + Video_GetScreenHeight() / 2;
		picnum = it->num;
		if (picnum > 0) {
			Image_DrawCachedImage(0, picnum, x1, y1, PIC_JUST_DRAW, 0);
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////////////////////

//读取S*D*
int JY_LoadSMap(const char *sceneFilename, const char *eventFilename)
{
    JY_UnloadSMap();
    return theSceneMap.load(sceneFilename, eventFilename);
}


//保存S*D*
int JY_SaveSMap(const char *sceneFilename, const char *eventFilename)
{
    return theSceneMap.save(sceneFilename, eventFilename);
}


int JY_UnloadSMap()
{
    theSceneMap.unload();
    return 0;
}


//从临时文件中读取场景id的S数据到内存
//取s的值
int JY_GetS(int id, int x, int y, int level)
{
    return theSceneMap.getS(id, x, y, level);

}


//存S的值
int JY_SetS(int id, int x, int y, int level, int v)
{
    return theSceneMap.setS(id, x, y, level, v);

}


//取D*
int JY_GetD(int sceneid, int id, int i)
{
    return theSceneMap.getD(sceneid, id, i);
}


//存D*
int JY_SetD(int sceneid, int id, int i, int v)
{
    return theSceneMap.setD(sceneid, id, i, v);
}


// 绘制场景地图
int JY_DrawSMap(int sceneid, int x, int y, int xoff, int yoff, int mypic)
{
    return theSceneMap.draw(sceneid, x, y, xoff, yoff, mypic);
}


int JY_UnloadWarMap()
{
    theWarMap.unload();
    return 0;
}


//加载战斗地图
// WarIDXfilename/WarGRPfilename 战斗地图idx/grp文件名
// mapid 战斗地图编号
// num 战斗地图数据层数   应为6
//         0层 地面数据
//         1层 建筑
//         2层 战斗人战斗编号
//         3层 移动时显示可移动的位置
//         4层 命中效果
//         5层 战斗人对应的贴图
// x_max,x_max   地图大小
int JY_LoadWarMap(const char *warIdxFilename, const char *warGrpFilename, int mapid)
{
    JY_UnloadWarMap();
    theWarMap.load(warIdxFilename, warGrpFilename, mapid);
    return 0;
}


//取战斗地图数据
int JY_GetWarMap(int x, int y, int level)
{
    return theWarMap.get(x, y, level);
}


//存战斗地图数据
int JY_SetWarMap(int x, int y, int level, int v)
{
    theWarMap.set(x, y, level, v);
    return 0;
}


//设置某层战斗地图为给定值
int JY_CleanWarMap(int level, int v)
{
    theWarMap.clean(level, v);
    return 0;
}

// 绘制战斗地图
// flag=0  绘制基本战斗地图
//     =1  显示可移动的路径，(v1,v2)当前移动坐标，白色背景(雪地战斗)
//     =2  显示可移动的路径，(v1,v2)当前移动坐标，黑色背景
//     =3  命中的人物用白色轮廓显示
//     =4  战斗动作动画  v1 战斗人物pic, v2贴图所属的加载文件id
//                       v3 武功效果pic  -1表示没有武功效果

int JY_DrawWarMap(int flag, int x, int y, int v1, int v2, int v3)
{
    theWarMap.draw(flag, x, y, v1, v2, v3);
    return 0;
}


// 读取主地图数据
int JY_LoadMMap(const char *earthname, const char *surfacename, const char *buildingname, 
        const char *buildxname, const char *buildyname)
{
    JY_UnloadMMap();
    theWorldMap.load(earthname, surfacename, buildingname, buildxname, buildyname);
}


// 取主地图数据 
// flag  0 _earthLayerData, 1 _surfaceLayerData, 2 buildings, 3 _buildxLayerData, 4 _buildyLayerData
int JY_GetMMap(int x, int y, BuildingType flag)
{
    return theWorldMap.get(x, y, flag);
}


// 存主地图数据 
// flag  0 _earthLayerData, 1 _surfaceLayerData, 2 buildings, 3 _buildxLayerData, 4 _buildyLayerData
int JY_SetMMap(short x, short y, BuildingType flag, short v)
{
    return theWorldMap.set(x, y, flag, v);
}


// 绘制主地图
int JY_DrawMMap(int x, int y, int mypic)
{
    theWorldMap.draw(x, y, mypic);
    return 0;
}


