#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
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
#define DEFAULT_BEATMODE 0 

#define BPM_MAX 300
#define BPM_MIN 40

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
static pthread_mutex_t fileLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t bpmModeLock = PTHREAD_MUTEX_INITIALIZER;

// function declarations
static void play_mode_zero(void);
static void play_mode_one(void);
static void* playingLoop(void*);
static void updateWaitTime(void);

// Start background thread for playing beats
// return 0 for success
// return an error number for error
int Beatbox_init(){

    bpm = DEFAULT_BPM;
    numBeatMode = DEFAULT_NUM_BPM_MODE;
    beatMode = DEFAULT_BEATMODE;
    running = true;
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
    int mode;
    while (running) {
        pthread_mutex_lock (&bpmModeLock);
        {
            mode = beatMode;
        }
        pthread_mutex_unlock (&bpmModeLock);
        switch(mode){
            case 0:
                play_mode_zero();
                break;
            case 1:    
                play_mode_one();
                break;
            case 2:
                break;
            default:
                break;
        }
    }
    return NULL;
}

static void play_mode_zero(void)
{
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);

    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);

    updateWaitTime();

    AudioMixer_queueSound(&snareFile);
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);
    
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);
}

static void play_mode_one(void)
{
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);

    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);

    updateWaitTime();

    AudioMixer_queueSound(&snareFile);
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);
    
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);

    updateWaitTime();
    // -----------------------------------------
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);
    
    AudioMixer_queueSound(&baseFile);
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);

    updateWaitTime();

    AudioMixer_queueSound(&snareFile);
    AudioMixer_queueSound(&hiHatFile);
    nanosleep(&reqtime, NULL);

    AudioMixer_queueSound(&coFile);
    nanosleep(&reqtime, NULL);

    updateWaitTime();
}

static void updateWaitTime(void)
{
    pthread_mutex_lock (&bpmLock);
    {
        reqtime.tv_nsec = (1000000000/bpm) * 30;
    }
    pthread_mutex_unlock (&bpmLock);
}

// End the background thread 
void Beatbox_end()
{
    running = false;
    pthread_join(playingThread, NULL);

    AudioMixer_cleanup();
    pthread_mutex_lock (&fileLock);
    {
        AudioMixer_freeWaveFileData(&hiHatFile);
        AudioMixer_freeWaveFileData(&baseFile);
        AudioMixer_freeWaveFileData(&snareFile);
        AudioMixer_freeWaveFileData(&coFile);
    }
    pthread_mutex_unlock (&fileLock);
}

int Beatbox_getBpm()
{
    int temp;
    pthread_mutex_lock (&bpmLock);
    {
       temp = bpm; 
    }
    pthread_mutex_unlock (&bpmLock);
    return temp;
}

// Set BPM of currently playing audio
// It should be between BPM_MIN and BPM_MAX (inclusive)
// return current BPM for success
// return 1 for an error which happens when @bpm is lower than BPM_MIN
// return 2 for an error which happens when @bpm is higher than BPM_MAX
int Beatbox_setBPM(int input_bpm)
{
    if (input_bpm < BPM_MIN)
        return 1;
    if (input_bpm > BPM_MAX)
        return 2;

    pthread_mutex_lock (&bpmLock);
    {
        bpm = input_bpm; 
    }
    pthread_mutex_unlock (&bpmLock);
    return bpm;
}

// Increase BPM by 5
// return current bpm
int Beatbox_increaseBPM()
{
    pthread_mutex_lock (&bpmLock);
    {
        bpm += 5;
        if (bpm > BPM_MAX)
            bpm = BPM_MAX;
    }
    pthread_mutex_unlock (&bpmLock);
    return bpm;
}

// Decrease BPM by 5
// return curreunt bpm 
int Beatbox_decreaseBPM()
{
    pthread_mutex_lock (&bpmLock);
    {
        bpm -= 5;
        if (bpm < BPM_MIN)
            bpm = BPM_MIN;

    }
    pthread_mutex_unlock (&bpmLock);
    return bpm;
}

int Beatbox_getMode()
{
    int mode;
    pthread_mutex_lock (&bpmModeLock);
    {
        mode = beatMode;
    }
    pthread_mutex_unlock (&bpmModeLock);
    return mode;
}

// Set the Mode of beats to i_th
// return the mode for success
// return -1 for an error when there is no @i_th beat
int Beatbox_setMode(int i)
{
    pthread_mutex_lock (&bpmModeLock);
    {
        if ( 0 <= i && i < numBeatMode){
            beatMode = i;
            pthread_mutex_unlock (&bpmModeLock);
            return i;
        }
    }
    pthread_mutex_unlock (&bpmModeLock);
    return -1;
}

// Chnage the beat mode to the right next (circular)
//  return the mode after change
int Beatbox_changeMode()
{
    int mode;
    pthread_mutex_lock (&bpmModeLock);
    {
        beatMode += 1;
        beatMode %= numBeatMode;
        mode = beatMode;
    }
    pthread_mutex_unlock(&bpmModeLock);
    return mode;
}

// return the available of modes (including no-sound mode)
int Beatbox_numMode()
{
    return numBeatMode;
}

// play BASE SOUND
void Beatbox_playBase()
{
    pthread_mutex_lock (&fileLock);
    {
        if (baseFile.numSamples > 0)
            AudioMixer_queueSound(&baseFile);
    }
    pthread_mutex_unlock (&fileLock);
}

// play SNARE_SOUND
void Beatbox_playSnare()
{
    pthread_mutex_lock (&fileLock);
    {
        if (snareFile.numSamples > 0) 
            AudioMixer_queueSound(&snareFile);
    }
    pthread_mutex_unlock (&fileLock);
}

// play Hi_HAT_SOUND
void Beatbox_playHihat()
{
    pthread_mutex_lock (&fileLock);
    {
        if (hiHatFile.numSamples > 0)
            AudioMixer_queueSound(&hiHatFile);
    }
    pthread_mutex_unlock (&fileLock);
}

