#ifndef AUDIO_H
#define AUDIO_H

#include "sdlxx.h"

int 	Audio_Init(void);
void 	Audio_Quit(void);
int		Audio_PlayMIDI(const char *filename);
int		Audio_PlayWAV(const char *filename);
void	Audio_FadeOut(float msec);
void    Audio_SetVolume(float volume);

class Audio {
public:
    static Audio * getInstance();
    virtual ~Audio() {}
    virtual int create() = 0;
    virtual void destroy() = 0;
    virtual int playMidi(const char *filename) = 0;
    virtual int playWav(const char *filename) = 0;
    virtual void fadeOut(float msec) = 0;
    virtual void stopMidi() = 0;
private:
    static Audio *instance;
};

#endif // AUDIO_H

