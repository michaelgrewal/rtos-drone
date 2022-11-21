#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "display.h"
#include "flight_controller.h"



int main(int argc, char* argv[])
{
	int fd;
	void *ptr;
	printf("[D] Hi I'm display HUD.\n");

	fd = shm_open(SHM_NAME, O_RDONLY, 0);
	if (fd == -1) {
		perror("shm_open() failed");
		return EXIT_FAILURE;
	}

	ptr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	printf("[D] Reads Speed1: %d, Speed2: %d, Speed3: %d, Speed4: %d\n", ((int*)ptr)[0], ((int*)ptr)[1], ((int*)ptr)[2], ((int*)ptr)[3]);

	sleep(3);

	printf("[D] Reads Speed1: %d, Speed2: %d, Speed3: %d, Speed4: %d\n", ((int*)ptr)[0], ((int*)ptr)[1], ((int*)ptr)[2], ((int*)ptr)[3]);

	//set the message value to 'test'
//	strcpy(smsg, "test");

	//establish a connection to the server's channel
//	coid = name_open(SERVER_NAME, 0);

	//send the message to the server and get a reply
//	MsgSend(coid, &smsg, sizeof(smsg), &rmsg, sizeof(rmsg));

    //print received checksum
//	printf("[D] Received message: %s\n", rmsg);

    //Close a server connection that was opened by name_open()
//	name_close(coid);

    return EXIT_SUCCESS;
}
