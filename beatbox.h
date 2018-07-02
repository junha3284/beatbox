// beatbox.h
// Module to support playing drum beats
// It spawns a background thread which keep filling in the buffer for ALIA audio output

#ifndef _BEATBOX_H
#define _BEATBOX_H

// Start background thread for playing beats
// return 0 for success
// return an error number for error
int Beatbox_init();

// End the background thread 
void Beatbox_end();

// Set BPM of currently playing audio
// It should be between 50 and 200 (inclusive)
// return the current bpm for success
// return 1 for an error which happens when @bpm is lower than 50
// return 2 for an error which happens when @bpm is higher than 200
int Beatbox_setBPM(int bpm);

// Increase BPM by 5
// return the current bpm for success
// return 1 for an error which happens when the curretn bpm is already maximum
int Beatbox_increaseBPM();

// Decrease BPM by 5
// return the current bpm for sucess
// return 1 for an error which happens when the current bpm is already minimum
int Beatbox_decreaseBPM();

// Set the Mode of beats to i_th
// return the mode for success
// return 1 for an error when there is no @i_th beat
int Beatbox_setMode(int i);

// Chnage the beat mode to the right next (circular)
//  return the mode after change
int Beatbox_changeMode();

// return the available of modes (including no-sound mode)
int Beatbox_numMode();

// play BASE SOUND
void Beatbox_playBase();

// play SNARE_SOUND
void Beatbox_playSnare();

// play Hi_HAT_SOUND
void Beatbox_playHihat();

#endif
