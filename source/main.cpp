/*
====================================================================================================
Remake of the classic RPG Game Heros Of Jinyong
====================================================================================================
*/

#include <memory>
#include <SDL.h>
#include "audio.h"
#include "video.h"
#include "script.h"
#include "image.h"
#include "mainmap.h"

namespace jy {

class Sdl2Initer {
public:
    Sdl2Initer();
    ~Sdl2Initer();
    bool inited() const;
};

Sdl2Initer::Sdl2Initer()
{
    if (!inited()) {
        if (SDL_Init(0) < 0) {
            throw HardwareException("SDL_Init() failed.");
        }
    }
}

Sdl2Initer::~Sdl2Initer()
{
    SDL_Quit();
}

bool Sdl2Initer::inited() const
{
    return SDL_WasInit(0);
}


class GameIniter {
public:
    GameIniter();
    ~GameIniter();
};

GameIniter::GameIniter()
{
    ImageCache_Init();
}

GameIniter::~GameIniter()
{
    ImageCache_Quit();
    JY_UnloadMMap();
    JY_UnloadSMap();
    JY_UnloadWarMap();
}

class ScriptIniter {
public:
    ScriptIniter();
    ~ScriptIniter();
    int loadAndRun(const char *filename);
};

ScriptIniter::ScriptIniter()
{
    if (Script_Init()) {
        throw HardwareException("Script_Init() failed.");
    }
}

ScriptIniter::~ScriptIniter()
{
    Script_Quit();
}

int ScriptIniter::loadAndRun(const char *filename)
{
    return Script_LoadAndRun(filename);
}

} // namespace jy

//========================================================================================
// The source of pain and chaos ;-)
//========================================================================================


#if 1
int main(int argc, char *argv[])
{
    try {
        jy::Sdl2Initer    sdl2;
        std::auto_ptr<Video> video(Video::getInstance());
        std::auto_ptr<Audio> audio(Audio::getInstance());
        jy::GameIniter    game;
        jy::ScriptIniter  script;
        script.loadAndRun("script/main.lua");
    }
    catch (const std::exception& e) {
        Log(e.what());
    }
	return 0;
}

#else

int x = 1;
int y = 1;


enum ScreenState {
    ssNormal,
    ssFadingOut,
    ssFadingIn
};


ScreenState ss;


void MoveEveryOne()
{
}


void RenderScreen()
{
    JY_DrawMMap(480, 0, 1);
    Video_UpdateScreen();
}


void GameLoop(void)
{
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
        }
        MoveEveryOne();
        RenderScreen();
        SDL_Delay(33);
    }
}


int main(int argc, char *argv[])
{
    try {
        jy::Sdl2Initer sdl2;
        std::auto_ptr<Video> video(Video::getInstance());
        jy::GameIniter game;
        JY_LoadMMap("data/earth.002",
                "data/surface.002",
                "data/building.002",
                "data/buildx.002",
                "data/buildy.002");
        JY_PicLoadFile("data/mmap.idx", "data/mmap.grp", 0);
        JY_PicLoadFile("data/hdgrp.idx", "data/hdgrp.grp", 1);
        GameLoop();
    }
    catch (const std::exception& e) {
        Log(e.what());
    }
    return 0;
}

#endif
