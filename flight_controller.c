#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include "flight_controller.h"
#include "propeller.h"



/* Forward declarations */
int create_shared_memory(int nbytes, void **ptr);
void update_direction(direction_t direction, int *target_speeds, char *command_buf);
void calculate_altitude(int *prop_speeds, int *altitude);
void setup_altitude_periodic_updates(pid_t server_pid, int chid, struct itimerspec *it, struct sigevent *se, timer_t *tID);

/* Commands */
inline void write_command(const char* command, char *command_buf, size_t *offset);
inline void hover(int *target_speeds, char *command_buf, size_t *offset);
inline void up(int *target_speeds, char *command_buf, size_t *offset);
inline void down(int *target_speeds, char *command_buf, size_t *offset);
inline void left(int *target_speeds, char *command_buf, size_t *offset);
inline void right(int *target_speeds, char *command_buf, size_t *offset);
inline void forwards(int *target_speeds, char *command_buf, size_t *offset);
inline void backwards(int *target_speeds, char *command_buf, size_t *offset);
inline void clockwise(int *target_speeds, char *command_buf, size_t *offset);
inline void cclockwise(int *target_speeds, char *command_buf, size_t *offset);
void shutdown(char *command_buf);

// type for receiving
typedef union
{
	uint16_t type;
	struct _pulse pulse;
	get_speed_msg_t msg_get;
	get_speed_resp_t resp_get;
	set_speeds_msg_t msg_set;
} recv_buf_t;




int main(int argc, char* argv[])
{
	int rcvid;
	int target_speeds[NUM_PROPS] = { HOVER, HOVER, HOVER, HOVER };
	recv_buf_t msg;
	get_speed_resp_t resp;

	void *ptr;
	int *prop_speeds;
	int *altitude;

	char *command_buf;
	size_t offset;

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

	// initialize drone data: 4 propellers, altitude and command
	prop_speeds = ptr + PROP_OFFSET;
	prop_speeds[0] = 0;
	prop_speeds[1] = 0;
	prop_speeds[2] = 0;
	prop_speeds[3] = 0;

	altitude = ptr + ALTITUDE_OFFSET;
	*altitude = 0;

	command_buf = ptr + COMMAND_OFFSET;
	offset = 0;
	write_command("Starting", command_buf, &offset);



	while(1)
	{
		// receive
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		// got a pulse
		if (rcvid == 0) {
			switch (msg.pulse.code) {

				// disconnected client pulse
				case _PULSE_CODE_DISCONNECT:
					ConnectDetach(msg.pulse.scoid);
					break;

				// pulse from Sensor 1 giving Propeller 1 speed
				case _PULSE_CODE_UPDATE_SPEED1:
					prop_speeds[0] = msg.pulse.value.sival_int;
					break;

				// pulse from Sensor 2 giving Propeller 2 speed
				case _PULSE_CODE_UPDATE_SPEED2:
					prop_speeds[1] = msg.pulse.value.sival_int;
					break;

				// pulse from Sensor 3 giving Propeller 3 speed
				case _PULSE_CODE_UPDATE_SPEED3:
					prop_speeds[2] = msg.pulse.value.sival_int;
					break;

				// pulse from Sensor 4 giving Propeller 4 speed
				case _PULSE_CODE_UPDATE_SPEED4:
					prop_speeds[3] = msg.pulse.value.sival_int;
					break;

				// pulse from Flight Controller for periodic altitude update
				case _PULSE_CODE_TIMER_UPDATE_ALTITUDE:
					calculate_altitude(prop_speeds, altitude);
					break;
				}
		}

		// got a message
		else {
			switch (msg.type)
			{

			// msg from Propellers to get target speed
			case GET_SPEED_MSG_TYPE:
				resp.target = target_speeds[msg.msg_get.prop_index];
				MsgReply(rcvid, EOK, &resp, sizeof(resp));
				break;

			// msg from Navigation to travel in specific direction
			case SET_SPEEDS_MSG_TYPE:
				update_direction(msg.msg_set.nav_data.direction, target_speeds, command_buf);
				MsgReply(rcvid, EOK, NULL, 0);
				break;
			}
		}

	}

	// clean up
	name_detach(attach, 0);
	munmap(ptr, PAGE_SIZE);

    return EXIT_SUCCESS;
}



// Simulates altitude. Periodic check if altitude is going up or down
void calculate_altitude(int *prop_speeds, int* altitude) {
	int speed1, speed2, speed3, speed4;
	speed1 = prop_speeds[0];
	speed2 = prop_speeds[1];
	speed3 = prop_speeds[2];
	speed4 = prop_speeds[3];

	// if all propellers are above HOVER then drone is ascending in altitude
	if (speed1 > HOVER && speed2 > HOVER && speed3 > HOVER && speed4 > HOVER) {
		*altitude += ALTITUDE_RATE;
	}
	// else if all propellers are below HOVER then drone is descending in altitude
	else if (*altitude > 0 && speed1 < HOVER && speed2 < HOVER && speed3 < HOVER && speed4 < HOVER) {
		*altitude -= ALTITUDE_RATE;
	}
}



// Periodic timeout for pulse altitude updates
void setup_altitude_periodic_updates(pid_t server_pid, int chid, struct itimerspec *it, struct sigevent *se, timer_t *tID) {
	struct itimerspec itime = *it;
	struct sigevent sigevent = *se;
	timer_t timerID = *tID;
	int coid;

	coid = ConnectAttach(0, server_pid, chid, _NTO_SIDE_CHANNEL, 0);
	SIGEV_PULSE_INIT(&sigevent, coid, 0, _PULSE_CODE_TIMER_UPDATE_ALTITUDE, 0);
	timer_create(CLOCK_REALTIME, &sigevent, &timerID);
	itime.it_value.tv_sec = 2;
	itime.it_value.tv_nsec = 0;
	itime.it_interval.tv_sec = 2;
	itime.it_interval.tv_nsec = 0;
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


// Send the correct movement command based on the Navigation's given direction code
void update_direction(direction_t direction, int *target_speeds, char *command_buf) {

	// shutdown
	if (direction == UINT8_MAX) {
		shutdown(command_buf);
		return;
	}

	int x_dir = 0; 
	int y_dir = 0;
	int z_dir = 0;
	int rot	  = 0;

	// check what direction bits were supplied
	if (direction & LEFT      ) ++x_dir;
	if (direction & RIGHT     ) --x_dir;

	if (direction & FORWARD   ) ++y_dir;
	if (direction & BACKWARD  ) --y_dir;

	if (direction & UP        ) ++z_dir;
	if (direction & DOWN      ) --z_dir;

	if (direction & CLOCKWISE ) ++rot;
	if (direction & CCLOCKWISE) --rot;

	size_t offset = 0;

	// execute the correct movement commands (z, x, y, rot)
	if (z_dir > 0) {
		up(target_speeds, command_buf, &offset);
	} else if (z_dir < 0) {
		down(target_speeds, command_buf, &offset);
	} else {
		hover(target_speeds, command_buf, &offset);
	}

	if (x_dir > 0) {
		left(target_speeds, command_buf, &offset);
	} else if (x_dir < 0) {
		right(target_speeds, command_buf, &offset);
	}

	if (y_dir > 0) {
		forwards(target_speeds, command_buf, &offset);
	} else if (y_dir < 0) {
		backwards(target_speeds, command_buf, &offset);
	}

	if (rot > 0) {
		clockwise(target_speeds, command_buf, &offset);
	} else if (rot < 0) {
		cclockwise(target_speeds, command_buf, &offset);
	}
}


// write the command name to buffer so Display can output it
void write_command(const char* command, char *command_buf, size_t *offset) {
	size_t len = strlen(command);
	snprintf(command_buf + *offset, PAGE_SIZE - *offset, "%s", command);
	*offset += len;
}

/* COMMANDS */
// change Propeller speeds to HOVER
void hover(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  = HOVER;
	target_speeds[FRONT_RIGHT] = HOVER;
	target_speeds[BACK_LEFT]   = HOVER;
	target_speeds[BACK_RIGHT]  = HOVER;
	write_command("Hover ", command_buf, offset);
}
// change Propeller speeds to go UP
void up(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  = ASCEND;
	target_speeds[FRONT_RIGHT] = ASCEND;
	target_speeds[BACK_LEFT]   = ASCEND;
	target_speeds[BACK_RIGHT]  = ASCEND;
	write_command("Up ", command_buf, offset);
}
// change Propeller speeds to go DOWN
void down(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  = DESCEND;
	target_speeds[FRONT_RIGHT] = DESCEND;
	target_speeds[BACK_LEFT]   = DESCEND;
	target_speeds[BACK_RIGHT]  = DESCEND;
	write_command("Down ", command_buf, offset);
}
// change Propeller speeds to go LEFT
void left(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  -= MOVE;
	target_speeds[FRONT_RIGHT] += MOVE;
	target_speeds[BACK_LEFT]   -= MOVE;
	target_speeds[BACK_RIGHT]  += MOVE;
	write_command("Left ", command_buf, offset);
}
// change Propeller speeds to go RIGHT
void right(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  += MOVE;
	target_speeds[FRONT_RIGHT] -= MOVE;
	target_speeds[BACK_LEFT]   += MOVE;
	target_speeds[BACK_RIGHT]  -= MOVE;
	write_command("Right ", command_buf, offset);
}
// change Propeller speeds to go FORWARDS
void forwards(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  -= MOVE;
	target_speeds[FRONT_RIGHT] -= MOVE;
	target_speeds[BACK_LEFT]   += MOVE;
	target_speeds[BACK_RIGHT]  += MOVE;
	write_command("Forwards ", command_buf, offset);
}
// change Propeller speeds to go BACKWARDS
void backwards(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  += MOVE;
	target_speeds[FRONT_RIGHT] += MOVE;
	target_speeds[BACK_LEFT]   -= MOVE;
	target_speeds[BACK_RIGHT]  -= MOVE;
	write_command("Backwards ", command_buf, offset);
}
// change Propeller speeds to rotate CLOCKWISE
void clockwise(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  -= MOVE;
	target_speeds[FRONT_RIGHT] += MOVE;
	target_speeds[BACK_LEFT]   -= MOVE;
	target_speeds[BACK_RIGHT]  += MOVE;
	write_command("CW ", command_buf, offset);
}
// change Propeller speeds to rotate COUNTER CLOCKWISE
void cclockwise(int *target_speeds, char *command_buf, size_t *offset) {
	target_speeds[FRONT_LEFT]  += MOVE;
	target_speeds[FRONT_RIGHT] -= MOVE;
	target_speeds[BACK_LEFT]   += MOVE;
	target_speeds[BACK_RIGHT]  -= MOVE;
	write_command("CCW ", command_buf, offset);
}
// shutdown command
void shutdown(char *command_buf) {
	size_t offset = 0;
	write_command("Shutting Down", command_buf, &offset);
}
