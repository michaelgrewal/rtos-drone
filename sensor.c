#include <stdio.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "propeller.h"
#include "flight_controller.h"
#include "sensor.h"

// forward declarations for thread functions
void *sensor(void *);

int main(void) {
	// arrays for each thread's pulse code
	const uint8_t codes[NUM_PROPS] = {_PULSE_CODE_UPDATE_SPEED1, _PULSE_CODE_UPDATE_SPEED2,
									  _PULSE_CODE_UPDATE_SPEED3, _PULSE_CODE_UPDATE_SPEED4};

	int fd, i, coid;
	void *ptr;
	thread_args_t thread_args[NUM_PROPS];
	pthread_t tids[NUM_PROPS];

	coid = name_open(SERVER_NAME, 0);

	// open shared memory for each sensor to read speed data written by propeller
	for (i = 0; i < NUM_PROPS; i++) {

		// open named shared memory in read-only
		fd = shm_open(shmem_props[i], O_RDONLY, 0);
		if (fd == -1) {
			perror("shm_open() in sensors failed");
			return EXIT_FAILURE;
		}

		// create a local mapping
		ptr = mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, fd, 0);
		if (ptr == MAP_FAILED) {
			perror("mmap failed\n");
			return EXIT_FAILURE;
		}

		// close fd no longer need it
		close(fd);

		// create attr for ROUND ROBIN scheduling and create thread
		thread_args[i].ptr = ptr;
		thread_args[i].coid = coid;
		thread_args[i].prop_code = codes[i];

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(&tids[i], &attr, sensor, (void *)(&thread_args[i]));
	}

	for (i = 0; i < NUM_PROPS; ++i) {
		pthread_join(tids[i], NULL);
		munmap(thread_args[i].ptr, sizeof(int));
	}
	
	name_close(coid);

	return EXIT_SUCCESS;
}



// read updated speed value and send pulse to server
void send_speed_to_server(int coid, void *ptr, int code) {
	int speed;
	speed = *(int *)ptr;
	if (-1 == MsgSendPulse(coid, -1, code, speed)) {
		if (EBADF == errno) {
			pthread_exit(NULL);
		} else {
			perror("send_speed_to_server(): ");
			pthread_exit(NULL);
		}
	}
	sleep(1);
}


// repeatedly send Propeller speed readings to the Flight Controller
void *sensor(void *args) {
	thread_args_t *th_args = args;

	while (1) {
		send_speed_to_server(th_args->coid, th_args->ptr, th_args->prop_code);
	}
}
