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
    if (Init_Cache()) {
        throw GameException("Init_Cache() failed.");
    }
    JY_PicInit();
}

GameIniter::~GameIniter()
{
    JY_PicInit();
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
    catch (const SDLException& e) {
        DLOG(e.what());
    }
	return 0;
}

