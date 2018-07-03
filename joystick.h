#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

enum Joystick_input {
    None = -1,
    Up,
    Right,
    Down,
    Left,
    Push,
};

typedef enum Joystick_input Joystick_input;

int Joystick_init();

void Joystick_end();

// read user input from joystick
// 0: up
// 1: right
// 2: down
// 3: left
// 4: no input yet
Joystick_input Joystick_read();

#endif
