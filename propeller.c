#include <stdio.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "propeller.h"
#include "flight_controller.h"

// forward declarations for thread functions
void *prop1(void *);
void *prop2(void *);
void *prop3(void *);
void *prop4(void *);




int main(void) {
	// arrays for each thread's function
	const void *funcs[] = {&prop1, &prop2, &prop3, &prop4};

	int fd, i;
	void *ptr;
//	printf("[P] Hi I'm propeller client.\n");

	// create shared memory for each propeller to store speed data into
	for (i = 0; i < NUM_PROPS; i++) {

		// create named shared memory
		fd = shm_open(shmem_props[i], O_RDWR|O_CREAT|O_TRUNC, 0600);
		if (fd == -1) {
			perror("shm_open() in propellers failed");
			return EXIT_FAILURE;
		}

		// allocate the memory for object
		if (ftruncate(fd, sizeof(int)) == -1) {
			perror("ftruncate failed\n");
			return EXIT_FAILURE;
		}

		// get a local mapping for object
		ptr = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		if (ptr == MAP_FAILED) {
			perror("mmap failed\n");
			return EXIT_FAILURE;
		}

		// clean up don't need fd anymore
		close(fd);

		// create attr for ROUND ROBIN scheduling and create thread
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(NULL, &attr, funcs[i], ptr);
	}

	sleep(5);
	return 0;
}

// updates the speed data in shared memory object for respective sensor to read
void update_shmem(void* ptr, int speed) {
	*(int *)ptr = speed;
}

// thread functions
// each propeller will update their current speed into shared memory object
void* prop1(void* ptr) {
	int speed1;

	while (1)
	{
		speed1 = 555;
		update_shmem(ptr, speed1);
	}
}

void* prop2(void* ptr) {
	int speed2;

	while (1)
	{
		speed2 = 666;
		update_shmem(ptr, speed2);
	}
}

void* prop3(void* ptr) {
	int speed3;

	while (1)
	{
		speed3 = 777;
		update_shmem(ptr, speed3);
	}
}

void* prop4(void* ptr) {
	int speed4;

	while (1)
	{
		speed4 = 888;
		update_shmem(ptr, speed4);
	}
}


