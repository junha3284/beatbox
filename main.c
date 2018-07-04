#include "beatbox.h"
#include "joystick.h"
#include "accelerometer.h"
#include "network.h"
#include "audioMixer.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    int i = Beatbox_init()
        || Joystick_init()
        || Accelerometer_init()
        || Network_start();

    if (i != 0){
        printf("ERROR: INITIALIZING FAILS\n");
        return 0;
    }

    CommandType currentCommand = NoCommand;
    int num;

    while (currentCommand != Stop){
        Network_checkCommand(&currentCommand, &num);
        switch (currentCommand){
            case SetBpm:
            {
                int result = Beatbox_setBPM(num);
                if (result >= 40){
                    char buf[32];
                    snprintf(buf, 32, "BPM is set to %d!", result);
                    Network_replyToCommand (buf);
                    continue;
                }
                Network_replyToCommand("Requested BPM is out of boundary\n");
                break;

            }
            case SetVolume:
            {
                AudioMixer_setVolume(num);
                char buf[32];
                snprintf(buf, 32, "Volume is set to %d", AudioMixer_getVolume());
                Network_replyToCommand (buf);
                break;
            }
            case SetMode:
            {
                int result = Beatbox_setMode(num);
                if (result != -1){
                    char buf[32];
                    snprintf(buf, 32, "Mode is set to %d", result);
                    Network_replyToCommand (buf);
                    continue;
                }
                Network_replyToCommand("Requested Mode is out of boundary\n");
                break;
            }
            case PlayBase:
            {
                Beatbox_playBase();
                Network_replyToCommand("played base!");
                break;
            }
            case PlaySnare:
            {
                Beatbox_playSnare();
                Network_replyToCommand("played Snare!");
                break;
            }
            case PlayHihat:
            {
                Beatbox_playHihat();
                Network_replyToCommand("played Hihat!");
                break;
            }
            default:
                break;
        }
    }
    

    Network_end();
    Accelerometer_end();
    Joystick_end();
    Beatbox_end();

    return 0;
}
