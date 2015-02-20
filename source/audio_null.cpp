/*
==========================================================================================
This file exists for my personal purpose.
==========================================================================================
*/

#include <SDL_stdinc.h>
#include "audio.h"

class NullAudio : public Audio {
public:
    NullAudio();
    ~NullAudio();
    virtual void playMidi(const char *filename);
    virtual void playWave(const char *filename);
    virtual void fadeOut(Uint32 msec);
    virtual void stopMidi();
};

NullAudio::~NullAudio()
{
}

NullAudio::NullAudio()
{
}

void NullAudio::stopMidi()
{
}

void NullAudio::fadeOut(Uint32 msec)
{
}

void NullAudio::playMidi(const char *filename)
{
}

void NullAudio::playWave(const char *filename)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// class Audio
//////////////////////////////////////////////////////////////////////////////////////////

Audio *Audio::_instance = NULL;

Audio *Audio::getInstance()
{
    if (_instance == NULL) {
        _instance = new NullAudio();
    }
    return _instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////


int Audio_PlayMIDI(const char *filename)
{
    Audio::getInstance()->playMidi(filename);
}

int Audio_PlayWAV(const char *filename)
{
    Audio::getInstance()->playWave(filename);
}

void Audio_FadeOut(Uint32 msec)
{
    Audio::getInstance()->fadeOut(msec);
}
