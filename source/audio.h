#ifndef JY_AUDIO_H
#define JY_AUDIO_H

#include "sdlxx.h"

int 	Audio_Init(void);
void 	Audio_Quit(void);
int		Audio_PlayMIDI(const char *filename);
int		Audio_PlayWAV(const char *filename);
void	Audio_FadeOut(Uint32 msec);
void    Audio_SetVolume(float volume);

class Audio {
public:
    static Audio * getInstance();
    Audio() {}
    virtual ~Audio() {}
    virtual void playMidi(const char *filename) = 0;
    virtual void playWave(const char *filename) = 0;
    virtual void fadeOut(Uint32 msec) = 0;
    virtual void stopMidi() = 0;
private:
    static Audio *_instance;
};

#endif // JY_AUDIO_H

