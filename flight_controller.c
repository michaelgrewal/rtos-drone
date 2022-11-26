#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "flight_controller.h"
#include "propeller.h"

// type for receiving
typedef union
{
	uint16_t type;
	struct _pulse pulse;
	char rmsg[MAX_STRING_LEN];
	get_speed_msg_t msg_get;
	get_speed_resp_t resp_get;
	set_speeds_msg_t msg_set;
} recv_buf_t;

// Forward declarations
int create_shared_memory(int nbytes, void **ptr);
void calc_speed(direction_t direction, int *target_speeds);
void calculate_altitude(void *ptr);
void setup_altitude_periodic_updates(pid_t server_pid, int chid, struct itimerspec *it, struct sigevent *se, timer_t *tID);


int main(int argc, char* argv[])
{
	int rcvid;
	int target_speeds[NUM_PROPS] = { HOVER, HOVER, HOVER, HOVER };
	char reply_msg[MAX_STRING_LEN];
	recv_buf_t msg;
	get_speed_resp_t resp;
	void *ptr;
	struct sigevent sigevent;
	struct itimerspec itime;
	timer_t timerID;
	pid_t server_pid;

	// register server name
	name_attach_t *attach;
	attach = name_attach(NULL, SERVER_NAME, 0);

	// Timeout setup for periodic altitude updates
	server_pid = getpid();
	setup_altitude_periodic_updates(server_pid, attach->chid, &itime, &sigevent, &timerID);

	// create shared memory object and return ptr
	if (create_shared_memory(PAGE_SIZE, &ptr) != 0) {
		perror("create_shared_memory() failed");
		exit(EXIT_FAILURE);
	}

	// initialize drone data
	((int*)ptr)[0] = 0;		// Propeller 1 speed
	((int*)ptr)[1] = 0;		// Propeller 2 speed
	((int*)ptr)[2] = 0;		// Propeller 3 speed
	((int*)ptr)[3] = 0;		// Propeller 4 speed
	((int*)ptr)[4] = 0;		// Altitude

	while(1)
	{
		// receive
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		// got a pulse
		if (rcvid == 0) {
			switch (msg.pulse.code) {
				case _PULSE_CODE_DISCONNECT:
					ConnectDetach(msg.pulse.scoid);
					break;
				// pulse from Sensor 1 giving Propeller 1 speed
				case _PULSE_CODE_UPDATE_SPEED1:
					((int*)ptr)[0] = msg.pulse.value.sival_int;
					break;
				// pulse from Sensor 2 giving Propeller 2 speed
				case _PULSE_CODE_UPDATE_SPEED2:
					((int*)ptr)[1] = msg.pulse.value.sival_int;
					break;
				// pulse from Sensor 3 giving Propeller 3 speed
				case _PULSE_CODE_UPDATE_SPEED3:
					((int*)ptr)[2] = msg.pulse.value.sival_int;
					break;
				// pulse from Sensor 4 giving Propeller 4 speed
				case _PULSE_CODE_UPDATE_SPEED4:
					((int*)ptr)[3] = msg.pulse.value.sival_int;
					break;
				case _PULSE_CODE_TIMER_ALTITUDE_UPDATE:
					calculate_altitude(ptr);
					break;
				}
		}

		// got a message
		else {
		//	printf("[FC] Received message type: %d\n", msg.type);

			switch (msg.type)
			{
			case GET_SPEED_MSG_TYPE:
				resp.target = target_speeds[msg.msg_get.prop_index];
				MsgReply(rcvid, EOK, &resp, sizeof(resp));
				break;

			case SET_SPEEDS_MSG_TYPE:
				calc_speed(msg.msg_set.nav_data.direction, target_speeds);
				MsgReply(rcvid, EOK, NULL, 0);
				break;
			}
		}

		// reset buffers (only when using string msgs, currently unused).
		memset(reply_msg, 0, sizeof(reply_msg));
		memset(msg.rmsg, 0, sizeof(msg.rmsg));
	}

	// clean up
	name_detach(attach, 0);
	munmap(ptr, PAGE_SIZE);

    return EXIT_SUCCESS;
}


// Simulates altitude. Periodic check if altitude is going up or down
void calculate_altitude(void *ptr) {
	int speed1, speed2, speed3, speed4, altitude;
	speed1 = ((int*)ptr)[0];
	speed2 = ((int*)ptr)[1];
	speed3 = ((int*)ptr)[2];
	speed4 = ((int*)ptr)[3];
	altitude = ((int*)ptr)[4];

	// if all propellers are above HOVER then drone is ascending in altitude
	if (speed1 > HOVER && speed2 > HOVER && speed3 > HOVER && speed4 > HOVER) {
		altitude += ALTITUDE_RATE;
	}
	// else if all propellers are below HOVER then drone is descending in altitude
	else if (speed1 < HOVER && speed2 < HOVER && speed3 < HOVER && speed4 < HOVER && altitude > 0) {
		altitude -= ALTITUDE_RATE;
	}
	// otherwise drone is hovering and no change to altitude

	((int*)ptr)[4] = altitude;
}

// Periodic timeout for pulse altitude updates
void setup_altitude_periodic_updates(pid_t server_pid, int chid, struct itimerspec *it, struct sigevent *se, timer_t *tID) {
	struct itimerspec itime = *it;
	struct sigevent sigevent = *se;
	timer_t timerID = *tID;
	int coid;

	coid = ConnectAttach(0, server_pid, chid, _NTO_SIDE_CHANNEL, 0);
	SIGEV_PULSE_INIT(&sigevent, coid, 0, _PULSE_CODE_TIMER_ALTITUDE_UPDATE, 0);
	timer_create(CLOCK_REALTIME, &sigevent, &timerID);
	itime.it_value.tv_sec = 2;
	itime.it_interval.tv_sec = 2;
	timer_settime(timerID, 0, &itime, NULL);
}

// creates shared memory to store all propeller speed data
int create_shared_memory(int nbytes, void **ptr) {
	int fd;

	// create named shared memory object
	fd = shm_open(SHM_NAME, O_RDWR|O_CREAT|O_TRUNC, 0600);
	if (fd == -1) {
		perror("shm_open in fc failed\n");
		return EXIT_FAILURE;
	}

	/* allocate the memory for the object */
	if (ftruncate(fd, nbytes) == -1) {
		perror("ftruncate failed\n");
		close(fd);
		return EXIT_FAILURE;
	}

	/* get a local mapping to the object */
	*ptr = mmap(NULL, nbytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (*ptr == MAP_FAILED) {
		perror("mmap failed\n");
		close(fd);
		return EXIT_FAILURE;
	}

	/* we no longer need the fd, so cleanup */
	close(fd);

	return 0;
}

void calc_speed(direction_t direction, int *target_speeds) {
	int x_dir = (direction & LEFT	  ) - (direction & RIGHT);
	int y_dir = (direction & FORWARD  ) - (direction & BACKWARD);
	int z_dir = (direction & UP		  ) - (direction & DOWN);
	int rot	  = (direction & CLOCKWISE) - (direction & CCLOCKWISE);

	target_speeds[FRONT_LEFT]  = HOVER;
	target_speeds[FRONT_RIGHT] = HOVER;
	target_speeds[BACK_LEFT]   = HOVER;
	target_speeds[BACK_RIGHT]  = HOVER;

	if (x_dir > 0) {
		// Move left, decrease left RPM
		target_speeds[FRONT_LEFT]  -= 1000;
		target_speeds[BACK_LEFT]   -= 1000;
	} else if (x_dir < 0) {
		// Move right, decrease right RPM
		target_speeds[FRONT_RIGHT] -= 1000;
		target_speeds[BACK_RIGHT]  -= 1000;
	}

	if (y_dir > 0) {
		// Move forwards, decrease front RPM
		target_speeds[FRONT_LEFT]  -= 1000;
		target_speeds[FRONT_RIGHT] -= 1000;
	} else if (x_dir < 0) {
		// Move backwards, decrease back RPM
		target_speeds[BACK_LEFT]   -= 1000;
		target_speeds[BACK_RIGHT]  -= 1000;
	}

	if (z_dir > 0) {
		// Move up, increase all RPM
		target_speeds[FRONT_LEFT]  += 1000;
		target_speeds[FRONT_RIGHT] += 1000;
		target_speeds[BACK_LEFT]   += 1000;
		target_speeds[BACK_RIGHT]  += 1000;
	} else if (x_dir < 0) {
		// Move down, decrease all RPM
		target_speeds[FRONT_LEFT]  -= 1000;
		target_speeds[FRONT_RIGHT] -= 1000;
		target_speeds[BACK_LEFT]   -= 1000;
		target_speeds[BACK_RIGHT]  -= 1000;
	}

	if (rot > 0) {
		// Rotate clockwise, decrease front right and back left,
		// increase front left and back right
		target_speeds[FRONT_LEFT]  += 1000;
		target_speeds[FRONT_RIGHT] -= 1000;
		target_speeds[BACK_LEFT]   -= 1000;
		target_speeds[BACK_RIGHT]  += 1000;
	} else if (x_dir < 0) {
		// Rotate counter clockwise, increase front right and back left,
		// decrease front left and back right
		target_speeds[FRONT_LEFT]  -= 1000;
		target_speeds[FRONT_RIGHT] += 1000;
		target_speeds[BACK_LEFT]   += 1000;
		target_speeds[BACK_RIGHT]  -= 1000;
	}
}
