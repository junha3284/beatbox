#include "beatbox.h"
#include "joystick.h"
#include "accelerometer.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    int i = Beatbox_init() || Joystick_init() || Accelerometer_init();
    if (i != 0){
        printf("hello\n");
        return 0;
    }
    sleep(20);
    Beatbox_end();
    Joystick_end();
    Accelerometer_end();
    return 0;
}
