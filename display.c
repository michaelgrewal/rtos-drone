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
//	printf("[D] Hi I'm display HUD.\n");

	// open named shared memory in read-only mode
	fd = shm_open(SHM_NAME, O_RDONLY, 0);
	if (fd == -1) {
		perror("shm_open() failed");
		return EXIT_FAILURE;
	}

	// map the shared memory
	ptr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);

	// close fd, not needed anymore
	close(fd);

	// keep updating the HUD in-line periodically
	while(1)
	{
		printf("\r[D] Reads Speed1: %d, Speed2: %d, Speed3: %d, Speed4: %d", ((int*)ptr)[0], ((int*)ptr)[1], ((int*)ptr)[2], ((int*)ptr)[3]);
		fflush(stdout);
		sleep(1);
	}

    return EXIT_SUCCESS;
}
