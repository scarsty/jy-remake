#ifndef JY_MAINMAP_H
#define JY_MAINMAP_H

#include "util.h"
#include <list>

typedef enum
{
	kLayerEarth,
	kLayerSurface,
	kLayerBuilding,
	kLayerBuildx,
	kLayerBuildy
} BuildingType;


//////////////////////////////////////////////////////////////////////////////////////////
// WorldMap
//////////////////////////////////////////////////////////////////////////////////////////


class WorldMap {
public:
    static const int kExtraWidth = 8;
    static const int kExtraHeight = 8;
    static const int kMaxBuildings = 2000;
    static const int kWidth = 480;
    static const int kHeight = 480;

    struct TBuilding {
        int x;
        int y;
        int num;
        TBuilding() : x(0), y(0), num(0) {}
        TBuilding(int x, int y, int num) : x(x), y(y), num(num) {}
        bool operator<(const TBuilding& rhs) const 
        {
            if (x == rhs.x)
                return y < rhs.y;
            if (y == rhs.y)
                return x < rhs.x;
            return (x < rhs.x && y < rhs.y);
        }
    };

    WorldMap();
    ~WorldMap();

    void draw(int x, int y, int mypic);
    int load(const char *earthname, const char *surfacename, const char *buildingname,
            const char *buildxname, const char *buildyname);
    int unload();
    int get(int x, int y, BuildingType flag);
    int set(short x, short y, BuildingType flag, short v);
    void sortBuildings(short x, short y, short mypic, int screen_w, int screen_h, 
        int extra_x, int extra_y, int x_scale, int y_scale);
    size_t coord2Index(int x, int y);

private:
    std::list<TBuilding> _sortedBuildings;
    std::auto_ptr<MemoryBlock> _earthLayerData;
    std::auto_ptr<MemoryBlock> _surfaceLayerData;
    std::auto_ptr<MemoryBlock> _buildingLayerData;
    std::auto_ptr<MemoryBlock> _buildxLayerData;
    std::auto_ptr<MemoryBlock> _buildyLayerData;
};


//////////////////////////////////////////////////////////////////////////////////////////
// class SceneMap
//////////////////////////////////////////////////////////////////////////////////////////


class SceneMap {
public:
    static const int kExtraWidth = 2;
    static const int kExtraHeight = 16;
    static const int kWidth = 64;
    static const int kHeight = 64;
    static const int kNumLevels = 6;
    static const int kNumMaps = 200;
    static const int kNumEvents = 200; // 每个场景中最多有200个事件
    static const int kNumEventArgs = 11; // 每个事件有11个参数

    SceneMap();
    ~SceneMap();

    int load(const char *s, const char *d);
    int save(const char *s, const char *d);
    void unload();
    int getS(int id, int x, int y, int level);
    int setS(int id, int x, int y, int level, int value);
    int getD(int scene, int id, int i);
    int setD(int scene, int id, int i, int value);
    int draw(int sceneid, int x, int y, int xoff, int yoff, int Mypic);

private:
    //MemoryBlock mapData;
    //MemoryBlock eventData;

    std::auto_ptr<MemoryBlock> _mapData;
    std::auto_ptr<MemoryBlock> _eventData;
};

//////////////////////////////////////////////////////////////////////////////////////////
// class BattleMap
//////////////////////////////////////////////////////////////////////////////////////////

class BattleMap {
public:
    static const int kExtraWidth = 2;
    static const int kExtraHeight = 18;
    static const int kWidth = 64;
    static const int kHeight = 64;
    static const int kNumLevels = 6; 

    BattleMap();
    ~BattleMap();
    int draw(int flag, int x, int y, int v1, int v2, int v3);
    int clean(int level, int v);
    int set(int x, int y, int level, int v);
    int get(int x, int y, int level);
    int load(const char *idxname, const char *grpname, int id);
    void unload();
private:
    int War_Num;
    Sint16 *pWar;
};

int 	JY_LoadMMap(const char* earthname, const char* surfacename, const char*buildingname, 
                const char* buildxname, const char* buildyname);
int 	JY_UnloadMMap(void);
int 	JY_GetMMap(int x, int y , BuildingType type);
int 	JY_SetMMap(short x, short y , BuildingType flag, short v);
int 	JY_DrawMMap(int x, int y, int Mypic);
//int 	JY_LoadSMap(const char *Sfilename,const char*tmpfilename, int num,int x_max,int y_max, const char *Dfilename,int d_num1,int d_num2);
int 	JY_LoadSMap(const char *Sfilename, const char *Dfilename);
int 	JY_SaveSMap(const char *Sfilename,const char *Dfilename);
int 	JY_UnloadSMap();
int 	JY_GetS(int id,int x,int y,int level);
int 	JY_SetS(int id,int x,int y,int level,int v);
int 	JY_GetD(int Sceneid,int id,int i);
int 	JY_SetD(int Sceneid,int id,int i,int v);
int 	JY_DrawSMap(int sceneid,int x, int y,int xoff,int yoff, int Mypic);
int 	JY_LoadWarMap(const char *WarIDXfilename,const char *WarGRPfilename, int mapid);
int 	JY_UnloadWarMap();
int 	JY_GetWarMap(int x,int y,int level);
int 	JY_SetWarMap(int x,int y,int level,int v);
int 	JY_CleanWarMap(int level,int v);
int 	JY_DrawWarMap(int flag, int x, int y, int v1,int v2,int v3);

#endif // JY_MAINMAP_H

