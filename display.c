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

inline void update();
inline void goto_x_y(unsigned x, unsigned y);

int main(int argc, char* argv[]) {
	int fd, running;
	void *ptr;
	char output[MAX_STRING_LEN];
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
	// TODO should show the current command too
	running = 1;
	while(running) {
		// for DEBUG, otherwise the print formatting below will overwrite any debug
//		 printf("\r[D] Reads Speed1: %d, Speed2: %d, Speed3: %d, Speed4: %d, Altitude: %d\n", ((int*)ptr)[0], ((int*)ptr)[1], ((int*)ptr)[2], ((int*)ptr)[3], ((int*)ptr)[4]);
//		 fflush(stdout);
//		 sleep(1);

		// cleaner output but will overwrite any debug print statements
	    update();
	    sprintf(output,
	    		"Propeller 1 (CCW): %5d RPM\t | \tPropeller 2 (CW) : %5d RPM\n"
	    		            "\t\t\t--------------------\n"
	    		"Propeller 3 (CW) : %5d RPM\t | \tPropeller 4 (CCW): %5d RPM\n"
	    		"\n"
	    		"Altitude: %d meters\n"
	    		"Current Command: <?>",
				((int*)ptr)[0], ((int*)ptr)[1], ((int*)ptr)[2], ((int*)ptr)[3], ((int*)ptr)[4]);

	    puts(output);
	    sleep(2);
	    goto_x_y(0, 0);
	}

	munmap(ptr, PAGE_SIZE);
    return EXIT_SUCCESS;
}

void update() {
	printf("\033[H\033[J");
}

void goto_x_y(unsigned x, unsigned y) {
	printf("\033[%d;%dH", x, y);
}
