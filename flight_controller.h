#ifndef FLIGHT_CONTROLLER_H_
#define FLIGHT_CONTROLLER_H_

#include <stdint.h>
#include <sys/iomsg.h>
#include <sys/mman.h>

#define SERVER_NAME "flight_controller"
#define MAX_STRING_LEN 4096
#define SHM_NAME "data"
#define PAGE_SIZE 4096
#define STAY_ALIVE_TIME	3600	//TODO temp variable for now, need to implement proper cleanup, procs and thread returns (pthread joins)

// SPEED (RPM) Values
#define STOP	0
#define DESCEND	2500
#define HOVER	5000
#define ASCEND	7500
#define MAX_RPM	10000


// PULSE CODES
#define _PULSE_CODE_UPDATE_SPEED1 (_PULSE_CODE_MINAVAIL + 1)
#define _PULSE_CODE_UPDATE_SPEED2 (_PULSE_CODE_MINAVAIL + 2)
#define _PULSE_CODE_UPDATE_SPEED3 (_PULSE_CODE_MINAVAIL + 3)
#define _PULSE_CODE_UPDATE_SPEED4 (_PULSE_CODE_MINAVAIL + 4)

// MESSAGE CODES
#define GET_TARGET_SPEED_MSG_TYPE	(_IO_MAX + 1)


typedef struct get_target_speed_msg {
	uint16_t type;
} get_target_speed_msg_t;

typedef struct get_target_speed_resp {
	unsigned target;
} get_target_speed_resp_t;

#endif /* FLIGHT_CONTROLLER_H_ */
