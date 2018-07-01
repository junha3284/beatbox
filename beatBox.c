#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "audioMixer.h"
#include "beatbox.h"

#define FILE_HI_HAT "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define FILE_BASE "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define FILE_SNARE "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
#define FILE_CO "beatbox-wav-files/100055__menegass__gui-drum-co.wav"

#define DEFAULT_BPM 100
#define DEFAULT_NUM_BPM_MODE 3
#define DEFAULT_BEATMODE 0;

static int bpm;
static int beatMode;
static int numBeatMode;

struct timespec reqtime;

static bool running;

static wavedata_t hiHatFile;
static wavedata_t baseFile;
static wavedata_t snareFile;
static wavedata_t coFile;

static pthread_t playingThread;

static pthread_mutex_t bpmLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t bpmModeLock = PTHREAD_MUTEX_INITIALIZER;

// function declarations
static void play_mode_zero();
static void stopForBpm(void);


// Start background thread for playing beats
// return 0 for success
// return an error number for error
int BeatBox_init(){

    bpm = DEFAULT_BPM;
    beatMode = DEFAULT_NUM_BPM;
    numBeatMode = DEFAULT_BEATMODE;
    reqtime.tv_sec = 0;
    reqtime.tv_nsec = (1000000000/bpm) * 30;
    AudioMixer_init();

    AudioMixer_readWaveFileIntoMemory(FILE_HI_HAT, &hiHatFile);
    AudioMixer_readWaveFileIntoMemory(FILE_BASE, &baseFile);
    AudioMixer_readWaveFileIntoMemory(FILE_SNARE, &snareFile);
    AudioMixer_readWaveFileIntoMemory(FILE_CO, &coFile);

    int threadCreateResult = pthread_create (&playingThread, NULL, playingLoop, NULL);
    return threadCreateResult;
}

static void* playingLoop(void *empty){
    while (running) {
        switch(mode){
            case 0:
                play_mode_zero();
                break;
            case 1:    
                play_mode_one();
                break;
            case 2:
                break;
        }
    }
}

static void play_mode_zero(void)
{
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();

    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();

    AudioMixer_queueSound(&snareFile);
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();
    
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();
}


static void play_mode_one(void)
{
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();

    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();

    AudioMixer_queueSound(&snareFile);
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();
    
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();

    // -----------------------------------------
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();
    
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();

    AudioMixer_queueSound(&snareFile);
    AudioMixer_queueSound(&hiHatFile);
    stopForBpm();

    AudioMixer_queueSound(&coFile);
    stopForBpm();
}

static void stopForBpm(void)
{
    pthread_lock_mutex (&BPMLock);
    {
        nanosleep(&reqtime, NULL);
    }
    pthread_unlock_mutex (&BPMLock);
}

// End the background thread 
void Beatbox_end()
{
    running = false;

    AudioMixer_cleanup();
    AudioMixer_freeWaveFileData(&hiHatFile);
    AudioMixer_freeWaveFileData(&baseFile);
    AudioMixer_freeWaveFileData(&snareFile);
    AudioMixer_freeWaveFileData(&coFile);

    pthread_join(playingThread, NULL);
}

// Set BPM of currently playing audio
// It should be between 50 and 200 (inclusive)
// return current BPM for success
// return 1 for an error which happens when @bpm is lower than 40
// return 2 for an error which happens when @bpm is higher than 300
int Beatbox_setBPM(int input_bpm)
{
    if (input_bpm < 40)
        return 1;
    if (input_bpm > 300)
        return 2;

    bpm = input_bpm; 
    pthread_lock_mutex (&BPMLock);
    {
        reqtime.tv_nsec = (1000000000/bpm) * 30;
    }
    pthread_unlock_mutex (&BPMLock);
    return bpm;
}

// Increase BPM by 5
// return current bpm
int Beatbox_increaseBPM()
{
    bpm += 5;
    if (bpm > 300)
        bpm = 300;

    pthread_lock_mutex (&BPMLock);
    {
        reqtime.tv_nsec = (1000000000/bpm) * 30;
    }
    pthread_unlock_mutex (&BPMLock);
    return bpm;
}

// Decrease BPM by 5
// return curreunt bpm 
int Beatbox_decreaseBPM()
{
    bpm -= 5;
    if (bpm < 40)
        bpm = 40;

    pthread_lock_mutex (&BPMLock);
    {
        reqtime.tv_nsec = (1000000000/bpm) * 30;
    }
    pthread_unlock_mutex (&BPMLock);
    return bpm;
}

// Set the Mode of beats to i_th
// return the mode for success
// return 1 for an error when there is no @i_th beat
int Beatbox_setMode(int i)
{
    // even though bpmMode is critical section
    // because there is no harm for violating 
    // So, didn't do mutex (Can be changed if it becomes
    // critical to give correct bpmMode for the current mode
    // without any delay
    if ( 0 <= i && i < numBeatMode){
        bpmMode = i;
        return bpmMode;
    }
    return 1;
}

// Chnage the beat mode to the right next (circular)
//  return the mode after change
int Beatbox_changeMode()
{
    bpmMode += 1;
    bpmMode %= numBeatMode;
    return bpmMode;
}

// return the available of modes (including no-sound mode)
int Beatbox_numMode()
{
    return numBeatMode;
}

