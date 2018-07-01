#include "beatbox.h"
#include "joystick.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    int i = Beatbox_init() || Joystick_init();
    if (i != 0){
        printf("hello\n");
        return 0;
    }
    Joystick_input userInput = None;
    while (userInput == None){
        userInput = Joystick_read();
        sleep(1);
    }
    Beatbox_end();
    Joystick_end();
    return 0;
}
