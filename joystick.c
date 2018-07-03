#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include "joystick.h"
#include "beatbox.h"
#include "audioMixer.h"

#define GPIO "/sys/class/gpio"
#define GPIO_PRE "/sys/class/gpio/gpio"
#define GPIO_VALUE_SUF "/value"
#define GPIO_DIRECTION_SUF "/direction"
#define GPIO_EXPORT_SUF "/export"

// Joystick Up, Right, Down, Left GPIO numbers
#define GPIO_UP 26
#define GPIO_RT 47
#define GPIO_DN 46
#define GPIO_JSLFT 65

#define JOYSTICK_SIZE 4
#define PATH_MAX_LENGTH 32 

#define INOTIFY_BUF_LENGTH 100

static pthread_t joystickListeningThread;

static bool running;

static FILE *pJoystickVal[JOYSTICK_SIZE];
static int gpio_nums[JOYSTICK_SIZE] = {GPIO_UP, GPIO_RT, GPIO_DN, GPIO_JSLFT};
static char gpio_value_paths[JOYSTICK_SIZE][64];


//Function declarations
static void* listeningLoop(void*);

int Joystick_init()
{
    running = true;
    // Export GPIO pins
    char buf[64];
    snprintf(buf, 64, "%s%s", GPIO, GPIO_EXPORT_SUF);
    FILE *pJoystickExport = fopen(buf, "w");
    for (int i=0; i < JOYSTICK_SIZE; i++){
        int charWritten = fprintf(pJoystickExport,"%d", gpio_nums[i]);
        if (charWritten <= 0){
            printf("ERROR EXPORTING GPIO pins\n");
            return 1;
        }
        rewind(pJoystickExport);
    }
    fclose(pJoystickExport);

    // Setting Direction to in 
    for (int i=0; i < JOYSTICK_SIZE; i++){
        snprintf(buf, 64, "%s%d%s", GPIO_PRE, gpio_nums[i], GPIO_DIRECTION_SUF);
        FILE *pJoystickDirection = fopen(buf, "w");
        for (int i=0; i < JOYSTICK_SIZE; i++){
            int charWritten = fprintf(pJoystickDirection, "%s", "in"); 
            if (charWritten <= 0){
                printf("ERROR SETTING DIRECTION GPIO pins\n");
                return 1;
            }
        }
        fclose(pJoystickDirection);
    }

    // make file pointer for value files of gpios 
    for (int i=0; i < JOYSTICK_SIZE; i++){
        snprintf(gpio_value_paths[i], 64, "%s%d%s", GPIO_PRE, gpio_nums[i], GPIO_VALUE_SUF);
    }

    int threadCreateResult = pthread_create (&joystickListeningThread, NULL, listeningLoop, NULL);
    return threadCreateResult;
}

static void* listeningLoop (void *empty)
{
    
    struct timespec reqtime;
    reqtime.tv_sec = 0;
    reqtime.tv_nsec = 500000000;
    
    Joystick_input user_input = None;
    while (running){
 
        /* Initialize Inotify*/
        printf("getting in busy wait\n");
        while (user_input == None && running){ 
            user_input = Joystick_read();
        }
        printf("executing!\n");
        
        switch(user_input){
            case None:
                break;
            case Up:
                user_input = None;
                AudioMixer_setVolume( AudioMixer_getVolume() + 5);
                nanosleep(&reqtime, NULL);
                break;
            case Right:
                user_input = None;
                Beatbox_increaseBPM();
                nanosleep(&reqtime, NULL);
                break;
            case Down:
                user_input = None;
                AudioMixer_setVolume( AudioMixer_getVolume() - 5);
                nanosleep(&reqtime, NULL);
                break;
            case Left:
                user_input = None;
                Beatbox_decreaseBPM();
                nanosleep(&reqtime, NULL);
                break;
            default:
                break;
        }
    }
    

    return NULL;
}

void Joystick_end()
{
    running = false;
    pthread_join(joystickListeningThread, NULL);
}


// 0: Up
// 1: Right
// 2: Down 
// 3: Left
// -1: None 
Joystick_input Joystick_read()
{
    pthread_join(joystickListeningThread, NULL);
    for (int i=0; i < JOYSTICK_SIZE; i++){
        pJoystickVal[i] = fopen(gpio_value_paths[i], "r");
        if (pJoystickVal[i] == NULL){
            printf("ERROR: Unable to open file (%s) for read\n", gpio_value_paths[i]);
            return 1; 
        }
    }

    int result = -1;

    char c;
    for (int i=0; i < JOYSTICK_SIZE; i++){
        c = fgetc(pJoystickVal[i]);
        if (c == '0'){
           result = i;
           break;
        }
    }
    for (int i=0; i < JOYSTICK_SIZE; i++){
        fclose(pJoystickVal[i]);
    }

    return result;
}

