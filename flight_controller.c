#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "flight_controller.h"


typedef union
{
	uint16_t type;
	struct _pulse pulse;
	char rmsg[MAX_STRING_LEN];
} recv_buf_t;

int create_shared_memory(int nbytes, void **ptr) {
	int fd;

	fd = shm_open(SHM_NAME, O_RDWR|O_CREAT|O_TRUNC, 0600);
	if (fd == -1) {
		perror("shm_open failed\n");
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
	printf("[FC] Hi I'm FC Server\n");
	int rcvid;
	char reply_msg[MAX_STRING_LEN];
	recv_buf_t msg;
	void *ptr;


	name_attach_t *attach;
	attach = name_attach(NULL, SERVER_NAME, 0);

	if (create_shared_memory(PAGE_SIZE, &ptr) != 0) {
		perror("create_shared_memory() failed");
		exit(EXIT_FAILURE);
	}
	strcpy((char *)ptr, "111-222-333-444");


	while(1)
	{
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		//pulse
		if (rcvid == 0) {
			switch (msg.pulse.code) {
			case _PULSE_CODE_DISCONNECT:
				printf("client is gone\n");
				ConnectDetach(msg.pulse.scoid);
				break;
			}
		}
		//message
		else {
			printf("[FC] Received message: %s\n", msg.rmsg);
			strcpy(reply_msg, "reply from server");
			MsgReply(rcvid, EOK, &reply_msg, sizeof(reply_msg));
		}

		memset(reply_msg, 0, sizeof(reply_msg));
		memset(msg.rmsg, 0, sizeof(msg.rmsg));
	}
	name_detach(attach, 0);

    return EXIT_SUCCESS;
}
