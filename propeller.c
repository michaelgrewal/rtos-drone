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

	int fd, i, coid;
	void *ptr;
//	printf("[P] Hi I'm propeller client.\n");

	coid = name_open(SERVER_NAME, 0);

	/*
	 * allocate memory for each propeller to store speed data into, and share with sensors that have read access into.
	 * this is separate from the flight controller shared memory.
	 * propellers individually store their speed locally, then sensors read it, and send it to flight controller where it gets stored into the "central" memory
	 */
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

		struct thread_args *args = malloc(sizeof(struct thread_args));
		args->ptr = ptr;
		args->coid = coid;

		// create attr for ROUND ROBIN scheduling and create thread
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(NULL, &attr, funcs[i], (void *)args);
	}

	sleep(10);	// TODO (maybe pthread join to wait for them instead)
	return 0;
}

int get_target_speed_from_server(int coid) {
	get_target_speed_msg_t msg_target;
	get_target_speed_resp_t resp_target;
	msg_target.type = GET_TARGET_SPEED_MSG_TYPE;
	MsgSend(coid, &msg_target, sizeof(msg_target), &resp_target, sizeof(resp_target));
	return resp_target.target;
}

// updates the speed data in shared memory object for respective sensor to read
void update_shmem(void* ptr, int speed) {
	*(int *)ptr = speed;
}

// thread functions
// each propeller will update their current speed into shared memory object
void* prop1(void* args) {
	struct thread_args *th_args = args;
	int speed1;

	while (1)
	{
		speed1 = get_target_speed_from_server(th_args->coid);
		update_shmem(th_args->ptr, speed1);
	}
}

void* prop2(void* args) {
	struct thread_args *th_args = args;
	int speed2;

	while (1)
	{
		speed2 = get_target_speed_from_server(th_args->coid);
		update_shmem(th_args->ptr, speed2);
	}
}

void* prop3(void* args) {
	struct thread_args *th_args = args;
	int speed3;

	while (1)
	{
		speed3 = get_target_speed_from_server(th_args->coid);
		update_shmem(th_args->ptr, speed3);
	}
}

void* prop4(void* args) {
	struct thread_args *th_args = args;
	int speed4;

	while (1)
	{
		speed4 = get_target_speed_from_server(th_args->coid);
		update_shmem(th_args->ptr, speed4);
	}
}


