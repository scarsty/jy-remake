/*
====================================================================================================
Remake of the classic RPG Game Heros Of Jinyong
====================================================================================================
*/

#include "audio.h"
#include "video.h"
#include "script.h"
#include "image.h"
#include "mainmap.h"




namespace jy {

    // for handy use
    class SDL {
    public:
        SDL() { }

        ~SDL() { destroy(); }

        bool ready() const
        {
        }

        int create()
        {
            if (SDL_Init(0) < 0) {
                return 1;
            }
            return 0;
        }

        void destroy()
        {
            SDL_Quit();
        }
    };
    class Game {
    public:
        Game() { }

        ~Game() { destroy(); }

        int create()
        {
            if (Init_Cache())
                return 1;
            JY_PicInit();
            return 0;
        }

        void destroy()
        {
            JY_PicInit();
            JY_UnloadMMap();
            JY_UnloadSMap();
            JY_UnloadWarMap();
        }
    };

    // Script engine
    class Script {
    public:
        Script() {}

        ~Script() { destroy(); }

        int loadAndRun(const char *filename)
        {
            return Script_LoadAndRun(filename);
        }

        int create()
        {
            if (Script_Init())
                return 1;
            return 0;
        }

        void destroy()
        {
            Script_Quit();
        }
    };

    class Audio {
    public:
        Audio() {}
        ~Audio() { destroy(); }

        int create()
        {
            Audio_Init();
        }

        void destroy()
        {
            Audio_Quit();
        }
    };

    class Video {
    public:
        Video() {}
        ~Video() { destroy(); }

        int create()
        {
            return Video_Init(960, 600);
        }

        void destroy()
        {
            Video_Quit();
        }
    };
} // namespace jy

//========================================================================================
// The source of pain and chaos ;-)
//========================================================================================


int main(int argc, char *argv[])
{
    jy::SDL     sdl;
    jy::Video   video;
    jy::Audio   audio;
    jy::Game    game;
    jy::Script  script;

    sdl.create();
    video.create();
    audio.create();
    game.create();
    script.create();

    script.loadAndRun("script/main.lua");

	return 0;
}

