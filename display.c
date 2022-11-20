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
	printf("[D] Hi I'm display HUD client.\n");

	fd = shm_open(SHM_NAME, O_RDONLY, 0);
	if (fd == -1) {
		perror("shm_open() failed");
		return EXIT_FAILURE;
	}

	ptr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);

	sleep(2);
	printf("[D] Reads %s\n", (char *)ptr);


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
