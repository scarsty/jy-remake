/*
 * 音频模块，使用BASS实现。
 *
 */

#include <SDL.h>
#include <math.h>
#include <BASS.h>
#include <BASSmidi.h>
#include "util.h"
#include "audio.h"


class BASSAudio : public Audio {
private:
    static const int MAX_SAMPLES = 5; 
    HSTREAM current_music;	//播放音乐数据，由于同时只播放一个，用一个变量
    HSAMPLE samples[MAX_SAMPLES];	//播放音效数据，可以同时播放几个，因此用数组
    int current_sample;		//当前播放的音效
    BASS_MIDI_FONT midfonts;
    bool g_sound_enabled;
    int g_MusicVolume;
    int g_SoundVolume;

public:
    BASSAudio();
    ~BASSAudio();

    virtual void destroy();
    virtual int create();

    virtual int playMidi(const char *filename);
    virtual int playWav(const char *filename);
    virtual void fadeOut(float msec);
    virtual void stopMidi();
};


// SDL_RW file procedures
static void CALLBACK RWclose(void *user)
{
	SDL_RWops *rw = (SDL_RWops *)user;
	SDL_RWclose(rw);
}


static QWORD CALLBACK RWlength(void *user)
{
	SDL_RWops *rw = (SDL_RWops *)user;
	Sint64 pos = SDL_RWtell(rw);
	Sint64 length = 0LL;
	SDL_RWseek(rw, 0, RW_SEEK_END);
	length = SDL_RWtell(rw);
	SDL_RWseek(rw, pos, RW_SEEK_SET);
	return (QWORD) length;
}


static DWORD CALLBACK RWread(void *buffer, DWORD length, void *user)
{
	SDL_RWops *rw = (SDL_RWops *)user;
	return SDL_RWread(rw, buffer, 1, length);
}


static BOOL CALLBACK RWseek(QWORD offset, void *user)
{
	SDL_RWops *rw = (SDL_RWops *)user;
	return SDL_RWseek(rw, offset, RW_SEEK_SET) != -1;
}


static BASS_FILEPROCS rwprocs = { RWclose, RWlength, RWread, RWseek };


#define ECHBUFLEN 1200
static float echbuf[ECHBUFLEN][2];
static int echpos;
static void CALLBACK MyDSPProc2(HDSP handle, DWORD channel, void *buffer, 
        DWORD length, void *user)
{
	float *d = (float *) buffer;
	DWORD i;

	for (i = 0; i < length / 4; i += 2) {
		float l = d[i] + (echbuf[echpos][1] / 2);
		float r = d[i + 1] + (echbuf[echpos][0] / 2);
		echbuf[echpos][0] = d[i] = l;
		echbuf[echpos][1] = d[i + 1] = r;
		echpos++;
		if (echpos == ECHBUFLEN)
			echpos = 0;
	}
}


static Uint32 start_time;
static float fade_out_time;
static HDSP hdsp;

static void CALLBACK FadeOutProc(HDSP handle, DWORD channel, void *buffer, 
        DWORD length, void *user)
{
#if 0
	float *d = (float *) buffer;
	DWORD a;
	float dc = (float) SDL_GetTicks() - start_time;
	if (dc > fade_out_time)
		dc = 0.0f;
	else
		dc = (fade_out_time - dc) / fade_out_time;
	for (a = 0; a < length / 4; a++) {
		d[a] = d[a] * dc;
	}
	if (dc == 0.0f)
		BASS_ChannelRemoveDSP(current_music, hdsp);
#endif
}

// there are only one channel used, so we don't need to specify channel.
void Audio_FadeOut(float time)
{
#if 0
	start_time = SDL_GetTicks();
	fade_out_time = time;
	hdsp = BASS_ChannelSetDSP(current_music, FadeOutProc, 0, 2);
#endif
}


SDL_RWops *soundfont_rw;

int Audio_Init(void)
{
#if 0
	int i;
	int so = 44100;
    const char *fname = "data/general.sf2";
    //SDL_RWops *rw = NULL;

	BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);
	if (!BASS_Init(-1, so, 0, 0, NULL)) {
		g_sound_enabled = SDL_FALSE;
		JY_ERROR("failed!");
        return 1;
	}
	current_sample = 0;

	//midfonts.font = BASS_MIDI_FontInit("data/general.sf2", 0);
#if 1
    soundfont_rw = SDL_RWFromFile(fname, "r");
    if (!soundfont_rw) {
        Log("cannot open file: %s", fname);
        return 1;
    }
    midfonts.font = BASS_MIDI_FontInitUser(&rwprocs, soundfont_rw, 0);
	if (!midfonts.font) {
		JY_ERROR(va("failed! code=%d", BASS_ErrorGetCode()));
        return 1;
	}
	midfonts.preset = -1;
	midfonts.bank = 0;
	BASS_MIDI_StreamSetFonts(0, &midfonts, 1);
    //SDL_RWclose(rw);
#endif

#endif
    return Audio::getInstance()->create();
}


void Audio_Quit(void)
{
#if 0
	int i;

	stopMidi();
	Log("stopMidi()");
	if (midfonts.font) {
		BASS_MIDI_FontFree(midfonts.font);
	}
	Log("BASS_MIDI_FontFree()");

	for (i = 0; i < MAX_SAMPLES; i++) {
		BASS_SampleFree(samples[i]);
		samples[i] = 0;
	}
	Log("Free Samples");

    //SDL_RWclose(soundfont_rw);

    //Log("Free soundfont RW");

	BASS_Free();

	Log("BASS_Free()");
#endif
    Audio::getInstance()->destroy();
}


int Audio_PlayMIDI(const char *filename)
{
#if 0
	static char currentfile[255] = "";
	void *mem = NULL;
	size_t memSize = 0;
    SDL_RWops *rw = NULL;

	if (!g_sound_enabled) {
		JY_ERROR("No Sound Available!");
		return 1;
	}
	if (strlen(filename) == 0)	//文件名为空，停止播放
	{
		stopMidi();
		strcpy(currentfile, filename);
		return 0;
	}
	if (strcmp(currentfile, filename) == 0)	//与当前播放文件相同，直接返回
		return 0;
	stopMidi();

	//current_music = BASS_StreamCreateFile(0, filename, 0, 0, 0);
	//current_music = BASS_MIDI_StreamCreateFile(0, filename, 0, 0, BASS_SAMPLE_LOOP|1, 0);
    rw = SDL_RWFromFile(filename, "r");
    if (!rw) {
        Log("Cannot open file: %s", filename);
        return 1;
    }
    current_music = BASS_MIDI_StreamCreateFileUser(STREAMFILE_NOBUFFER,
            BASS_SAMPLE_LOOP | 1, &rwprocs, rw, 0);
    //SDL_RWclose(rw);
	//mem = Util_MemoryFromFile(filename, &memSize);
	//current_music = BASS_MIDI_StreamCreateFile(TRUE, mem, 0, memSize, BASS_SAMPLE_LOOP | 1, 0);
	//free(mem);
	//rw = SDL_RWFromFile(filename, "r");
	//if (!rw) {
	//    Log(va("cannot open file %s", filename));
	//    return 1;
	//}
	//current_music = BASS_MIDI_StreamCreateFileUser(
	//        STREAMFILE_NOBUFFER, BASS_SAMPLE_LOOP|1, &rwprocs, rw, 0);
	if (!current_music) {
		JY_ERROR(va("cannot open file: %s. code=%d", filename, BASS_ErrorGetCode()));
		return 1;
	}
	BASS_ChannelSetAttribute(current_music, BASS_ATTRIB_VOL, (float) (g_MusicVolume / 100.0));
	//BASS_ChannelFlags(current_music, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
	BASS_ChannelPlay(current_music, FALSE);
	//fade_out_time = 2000.0f; // 2 seconds
	//start_time = Sys_GetTime();
	//BASS_ChannelSetDSP(current_music, FadeOutProc, 0, 2);
	strcpy(currentfile, filename);
#endif
    return Audio::getInstance()->playMidi(filename);
}


int Audio_PlayWAV(const char *filename)
{
#if 0
	HCHANNEL channel;
	void *mem = NULL;
	size_t memSize = 0;

	if (!g_sound_enabled)
		return 1;

	if (samples[current_sample]) {	//释放当前音效
		BASS_SampleStop(samples[current_sample]);
		samples[current_sample] = 0;
	}
	mem = Util_MemoryFromFile(filename, &memSize);
	//samples[current_sample] = BASS_SampleLoad(0, filename, 0, 0, 1, 0); // 加载到当前音效
	samples[current_sample] = BASS_SampleLoad(TRUE, mem, 0, memSize, 1, 0);	// 加载到当前音效
	free(mem);
	if (samples[current_sample]) {
		channel = BASS_SampleGetChannel(samples[current_sample], 0);
		BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, (float) (g_SoundVolume / 100.0));
		BASS_ChannelPlay(channel, 0);
		current_sample++;
		if (current_sample >= MAX_SAMPLES)
			current_sample = 0;
	} else {
		JY_ERROR(va("cannot open file: %s", filename));
	}
#endif
    return Audio::getInstance()->playWav(filename);
}

void Audio_SetVolume(float volume)
{
    BASS_SetVolume(volume);
}


//////////////////////////////////////////////////////////////////////////////////////////
// class Audio
//////////////////////////////////////////////////////////////////////////////////////////

Audio *Audio::instance = 0;

Audio *Audio::getInstance()
{
    if (!instance) {
        instance = new BASSAudio();
    }
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
// class BASSAudio
//////////////////////////////////////////////////////////////////////////////////////////

int BASSAudio::create()
{
	int i;
	int so = 44100;
    const char *fname = "data/sb.sf2";

	BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);
	if (!BASS_Init(-1, so, 0, 0, NULL)) {
		g_sound_enabled = SDL_FALSE;
		JY_ERROR("failed!");
        return 1;
	}
	current_sample = 0;

    // 不要对soundfont_rw调用SDL_RWclose(), BASS会做这些。
    soundfont_rw = SDL_RWFromFile(fname, "r");
    if (!soundfont_rw) {
        Log("cannot open file: %s", fname);
        return 1;
    }
    midfonts.font = BASS_MIDI_FontInitUser(&rwprocs, soundfont_rw, 0);
	if (!midfonts.font) {
		JY_ERROR(va("failed! code=%d", BASS_ErrorGetCode()));
        return 1;
	}
	midfonts.preset = -1;
	midfonts.bank = 0;
	BASS_MIDI_StreamSetFonts(0, &midfonts, 1);

	return 0;
}

BASSAudio::BASSAudio() : g_sound_enabled(true), g_MusicVolume(100), g_SoundVolume(100)
{
}

BASSAudio::~BASSAudio()
{
}

void BASSAudio::destroy()
{
	int i;

	stopMidi();

	if (midfonts.font) {
		BASS_MIDI_FontFree(midfonts.font);
	}

	for (i = 0; i < MAX_SAMPLES; i++) {
		BASS_SampleFree(samples[i]);
		samples[i] = 0;
	}
	BASS_Free();
}


int BASSAudio::playWav(const char *filename)
{
	HCHANNEL channel;
	//void *mem = NULL;
	size_t memSize = 0;

	if (!g_sound_enabled)
		return 1;

	if (samples[current_sample]) {	//释放当前音效
		BASS_SampleStop(samples[current_sample]);
		samples[current_sample] = 0;
	}
	//mem = Util_MemoryFromFile(filename, &memSize);
    MemoryBlock mem;
    //mem.fromFile(filename);
    mem.create(filename);
	//samples[current_sample] = BASS_SampleLoad(0, filename, 0, 0, 1, 0); // 加载到当前音效
	samples[current_sample] = BASS_SampleLoad(TRUE, mem.ptr(), 0, mem.getSize(), 1, 0);	// 加载到当前音效
	//free(mem);
	if (samples[current_sample]) {
		channel = BASS_SampleGetChannel(samples[current_sample], 0);
		BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 1.0);
		BASS_ChannelPlay(channel, 0);
		current_sample++;
		if (current_sample >= MAX_SAMPLES)
			current_sample = 0;
	} else {
		JY_ERROR(va("cannot open file: %s", filename));
	}

	return 0;
}


int BASSAudio::playMidi(const char *filename)
{
	if (!g_sound_enabled) {
        Log("No sound available.");
		return 1;
	}
	static char currentfile[255] = "";
	void *mem = NULL;
	size_t memSize = 0;
    SDL_RWops *rw = NULL;
	//文件名为空，停止播放
	if (strlen(filename) == 0) {
		stopMidi();
		strcpy(currentfile, filename);
		return 0;
	}
	if (strcmp(currentfile, filename) == 0)	//与当前播放文件相同，直接返回
		return 0;
	stopMidi();

    rw = SDL_RWFromFile(filename, "r");
    if (!rw) {
        Log("Cannot open file: %s", filename);
        return 1;
    }
    current_music = BASS_MIDI_StreamCreateFileUser(STREAMFILE_NOBUFFER,
            BASS_SAMPLE_LOOP | 1, &rwprocs, rw, 0);
	if (!current_music) {
		JY_ERROR(va("cannot open file: %s. code=%d", filename, BASS_ErrorGetCode()));
		return 1;
	}
	BASS_ChannelSetAttribute(current_music, BASS_ATTRIB_VOL, 1.0);
	BASS_ChannelPlay(current_music, FALSE);
	strcpy(currentfile, filename);

	return 0;
}


void BASSAudio::stopMidi()
{
	if (current_music) {
		BASS_ChannelStop(current_music);
		BASS_StreamFree(current_music);
		current_music = 0;
	}
}


void BASSAudio::fadeOut(float msec)
{
}

