#ifndef FLIGHT_CONTROLLER_H_
#define FLIGHT_CONTROLLER_H_

#include <stdint.h>
#include <sys/iomsg.h>
#include <sys/mman.h>
#include "navigation.h"

#define SERVER_NAME "flight_controller"
#define MAX_STRING_LEN 4096
#define SHM_NAME "data"
#define PAGE_SIZE 4096
#define STAY_ALIVE_TIME	3600	//TODO temp variable for now, need to implement proper cleanup, procs and thread returns (pthread joins)
#define ALTITUDE_RATE		10
#define ALTITUDE_TIMEOUT	2

// SPEED (RPM) Values
#define STOP	0
#define DESCEND	2500
#define HOVER	5000
#define ASCEND	7500
#define MAX_RPM	10000


// PULSE CODES
#define _PULSE_CODE_UPDATE_SPEED1 			(_PULSE_CODE_MINAVAIL + 1)
#define _PULSE_CODE_UPDATE_SPEED2 			(_PULSE_CODE_MINAVAIL + 2)
#define _PULSE_CODE_UPDATE_SPEED3 			(_PULSE_CODE_MINAVAIL + 3)
#define _PULSE_CODE_UPDATE_SPEED4 			(_PULSE_CODE_MINAVAIL + 4)
#define _PULSE_CODE_TIMER_ALTITUDE_UPDATE	(_PULSE_CODE_MINAVAIL + 5)

// MESSAGE CODES
#define GET_SPEED_MSG_TYPE	(_IO_MAX + 1)
#define SET_SPEEDS_MSG_TYPE	(_IO_MAX + 2)

typedef struct get_speed_msg {
	uint16_t type;
	uint8_t prop_index;
} get_speed_msg_t;

typedef struct get_speed_resp {
	unsigned target;
} get_speed_resp_t;

typedef struct set_speeds_msg {
	uint16_t type;
	nav_data_t nav_data;
} set_speeds_msg_t;

//no response needed for set_speeds

#endif /* FLIGHT_CONTROLLER_H_ */
