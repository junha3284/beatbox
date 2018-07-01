#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "audioMixer.h"

#define FILE_HI_HAT "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define FILE_BASE "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define FILE_SNARE "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
#define FILE_CO "beatbox-wav-files/100055__menegass__gui-drum-co.wav"

int main() {
    int BPM = 120;
    AudioMixer_init();

    struct timespec reqtime;
    reqtime.tv_sec = 0;
    reqtime.tv_nsec = (1000000000/BPM) * 30;
    nanosleep(&reqtime, NULL);

    printf("Volume is %d\n", AudioMixer_getVolume());

    wavedata_t hiHatFile;
    wavedata_t baseFile;
    wavedata_t snareFile;
    wavedata_t coFile;

    AudioMixer_readWaveFileIntoMemory(FILE_HI_HAT, &hiHatFile);
    AudioMixer_readWaveFileIntoMemory(FILE_BASE, &baseFile);
    AudioMixer_readWaveFileIntoMemory(FILE_SNARE, &snareFile);
    AudioMixer_readWaveFileIntoMemory(FILE_CO, &coFile);
    
    while (1){
        AudioMixer_queueSound(&baseFile);
        AudioMixer_queueSound(&hiHatFile);
        nanosleep(&reqtime, NULL);
        AudioMixer_queueSound(&hiHatFile);
        nanosleep(&reqtime, NULL);
        AudioMixer_queueSound(&snareFile);
        AudioMixer_queueSound(&hiHatFile);
        nanosleep(&reqtime, NULL);
        AudioMixer_queueSound(&hiHatFile);
        nanosleep(&reqtime, NULL);
        // -----------------------------------------
        AudioMixer_queueSound(&baseFile);
        AudioMixer_queueSound(&hiHatFile);
        nanosleep(&reqtime, NULL);
        AudioMixer_queueSound(&baseFile);
        AudioMixer_queueSound(&hiHatFile);
        nanosleep(&reqtime, NULL);
        AudioMixer_queueSound(&snareFile);
        AudioMixer_queueSound(&hiHatFile);
        nanosleep(&reqtime, NULL);
        AudioMixer_queueSound(&coFile);
        nanosleep(&reqtime, NULL);
    }

    AudioMixer_cleanup();
    AudioMixer_freeWaveFileData(&hiHatFile);
    AudioMixer_freeWaveFileData(&baseFile);
    AudioMixer_freeWaveFileData(&snareFile);
    printf("Done!\n");

    return 0;
}
