#ifndef NAVIGATION_H_
#define NAVIGATION_H_

// binary codes representing each command
typedef enum {
    MAINTAIN    = 0b00000000,
    UP          = 0b00000001,
    DOWN        = 0b00000010,
    LEFT        = 0b00000100,
    RIGHT       = 0b00001000,
    FORWARD     = 0b00010000,
    BACKWARD    = 0b00100000,
    CLOCKWISE   = 0b01000000,
    CCLOCKWISE  = 0b10000000
} direction_t;


#endif /* NAVIGATION_H_ */
