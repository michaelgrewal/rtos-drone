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
void *sens1(void *);
void *sens2(void *);
void *sens3(void *);
void *sens4(void *);

struct thread_args {
	void *ptr;
	int coid;
};



int main(void) {
	// arrays for each thread's function
	const void *funcs[] = {&sens1, &sens2, &sens3, &sens4};

	int fd, i, coid;
	void *ptr;
//	printf("[S] Hi I'm sensor.\n");

	coid = name_open(SERVER_NAME, 0);

	// open shared memory for each sensor to read speed data written by propeller
	for (i = 0; i < NUM_SENS; i++) {

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
		struct thread_args *args = malloc(sizeof(struct thread_args));
		args->ptr = ptr;
		args->coid = coid;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(NULL, NULL, funcs[i], (void *)args);
	}

	sleep(5);
	return 0;
}

// send pulse to server with updated speed value
void send_speed_to_server(int coid, int speed, int code) {
	MsgSendPulse(coid, -1, code, speed);
}

// read the propeller's current speed
int read_prop_speed(void *ptr) {
	return *(int *)ptr;
}

// thread functions
// each sensor shares the same connection id to the server and send the respective propeller speed value in a pulse
void* sens1(void* args) {
	struct thread_args *th_args = args;
	int speed1;

	while (1)
	{
		speed1 = read_prop_speed(th_args->ptr);
		send_speed_to_server(th_args->coid, speed1, _PULSE_CODE_UPDATE_SPEED1);
	}
}

void* sens2(void* args) {
	struct thread_args *th_args = args;
	int speed2;

	while (1)
	{
		speed2 = read_prop_speed(th_args->ptr);
		send_speed_to_server(th_args->coid, speed2, _PULSE_CODE_UPDATE_SPEED2);
	}
}

void* sens3(void* args) {
	struct thread_args *th_args = args;
	int speed3;

	while (1)
	{
		speed3 = read_prop_speed(th_args->ptr);
		send_speed_to_server(th_args->coid, speed3, _PULSE_CODE_UPDATE_SPEED3);
	}
}

void* sens4(void* args) {
	struct thread_args *th_args = args;
	int speed4;

	while (1)
	{
		speed4 = read_prop_speed(th_args->ptr);
		send_speed_to_server(th_args->coid, speed4, _PULSE_CODE_UPDATE_SPEED4);
	}
}


