#ifndef FLIGHT_CONTROLLER_H_
#define FLIGHT_CONTROLLER_H_

#include <stdint.h>
#include <sys/iomsg.h>
#include <sys/mman.h>

#include "propeller.h"
#include "navigation.h"

#define SERVER_NAME "flight_controller"
#define SHM_NAME "data"
#define PAGE_SIZE 4096
#define ALTITUDE_RATE		10
#define ALTITUDE_TIMEOUT	2
#define PROP_OFFSET 0
#define ALTITUDE_OFFSET PROP_OFFSET + NUM_PROPS * sizeof(int)
#define COMMAND_OFFSET PAGE_SIZE / 2

// SPEED (RPM) Values
typedef enum {
	STOP	= 0,
	MOVE	= 1000,
	DESCEND	= 2500,
	HOVER   = 5000,
	ASCEND	= 7500,
	MAX_RPM	= 10000
} propeller_rpm_states_t;

// PULSE CODES
#define _PULSE_CODE_UPDATE_SPEED1 			(_PULSE_CODE_MINAVAIL + 1)
#define _PULSE_CODE_UPDATE_SPEED2 			(_PULSE_CODE_MINAVAIL + 2)
#define _PULSE_CODE_UPDATE_SPEED3 			(_PULSE_CODE_MINAVAIL + 3)
#define _PULSE_CODE_UPDATE_SPEED4 			(_PULSE_CODE_MINAVAIL + 4)
#define _PULSE_CODE_TIMER_UPDATE_ALTITUDE	(_PULSE_CODE_MINAVAIL + 5)

// MESSAGE CODES
#define GET_SPEED_MSG_TYPE	(_IO_MAX + 1)
#define SET_SPEEDS_MSG_TYPE	(_IO_MAX + 2)


// type for sending navigation messages
typedef struct {
    direction_t direction;
    unsigned duration;
} nav_data_t;

// type for get speed msg
typedef struct get_speed_msg {
	uint16_t type;
	uint8_t prop_index;
} get_speed_msg_t;

// type for get speed response
typedef struct get_speed_resp {
	unsigned target;
} get_speed_resp_t;

// type for set speeds msg
typedef struct set_speeds_msg {
	uint16_t type;
	nav_data_t nav_data;
} set_speeds_msg_t;

//no response needed for set_speeds


#endif /* FLIGHT_CONTROLLER_H_ */
