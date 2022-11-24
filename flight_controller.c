#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "flight_controller.h"

// type for receiving
typedef union
{
	uint16_t type;
	struct _pulse pulse;
	char rmsg[MAX_STRING_LEN];
	get_target_speed_msg_t msg_target;
	get_target_speed_resp_t resp_target;
} recv_buf_t;



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
		return EXIT_FAILURE;
	}

	/* get a local mapping to the object */
	*ptr = mmap(NULL, nbytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (*ptr == MAP_FAILED) {
		perror("mmap failed\n");
		return EXIT_FAILURE;
	}

	/* we no longer need the fd, so cleanup */
	close(fd);

	return 0;
}




int main(int argc, char* argv[])
{
//	printf("[FC] Hi I'm FC Server\n");
	int rcvid;
	char reply_msg[MAX_STRING_LEN];
	recv_buf_t msg;
	get_target_speed_resp_t resp_target;
	void *ptr;

	// register server name
	name_attach_t *attach;
	attach = name_attach(NULL, SERVER_NAME, 0);

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
				}
		}

		// got a message
		else {
//			printf("[FC] Received message type: %d\n", msg.type);

			switch (msg.type)
			{
			case GET_TARGET_SPEED_MSG_TYPE:
				// TODO target speed should come from the NAVIGATION PROCESS. Using HOVER for now debug purposes.
				resp_target.target = HOVER;
				MsgReply(rcvid, EOK, &resp_target, sizeof(resp_target));
				break;
			}
		}

		// reset buffers (only when using string msgs, currently unused).
		memset(reply_msg, 0, sizeof(reply_msg));
		memset(msg.rmsg, 0, sizeof(msg.rmsg));
	}

	// clean up
	name_detach(attach, 0);

    return EXIT_SUCCESS;
}
