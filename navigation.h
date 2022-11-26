#ifndef NAVIGATION_H_
#define NAVIGATION_H_

typedef enum {
    NAV_HOVER       = 0b00000000,
    NAV_UP          = 0b00000001,
    NAV_DOWN        = 0b00000010,
    NAV_LEFT        = 0b00000100,
    NAV_RIGHT       = 0b00001000,
    NAV_FORWARD     = 0b00010000,
    NAV_BACKWARD    = 0b00100000,
    NAV_CLOCKWISE   = 0b01000000,
    NAV_CCLOCKWISE  = 0b10000000
} direction_t;

typedef struct {
    direction_t direction;
    unsigned duration;
} nav_data_t;

#endif /* NAVIGATION_H_ */
