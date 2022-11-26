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

// forward declarations for functions
int get_target_speed_from_server(int coid, int index);
void update_shmem(void* ptr, int speed);
void adjust_speed_to_target(int speed, int target, void* ptr);

void *update_propeller(void *);
void *wind(void *);

// mutexes so wind simulation and propellers don't collide with data access (prevent race conditions)
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *mutexes[] = { &mutex1, &mutex2, &mutex3, &mutex4 };

int main(void) {
	// arrays for each thread's function
	thread_args_t thread_args[NUM_PROPS];
	pthread_t tids[NUM_PROPS];

	// array of pointers to each propellers speed data, to be used by wind thread so it can force the new speeds on the propeller threads
	int **ptrs = (int **)malloc(4*sizeof(int));

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
		ptrs[i] = ptr;

		// clean up don't need fd anymore
		close(fd);

		thread_args[i].ptr = ptr;
		thread_args[i].coid = coid;
		thread_args[i].mutex = mutexes[i];
		thread_args[i].prop_index = (propeller_index_t)(i);

		// create attr for ROUND ROBIN scheduling and create thread
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(&tids[i], &attr, update_propeller, (void *)(&thread_args[i]));
	}

	// thread to simulate wind
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_create(NULL, &attr, &wind, (void *)ptrs);

	for(i = 0; i < NUM_PROPS; ++i) {
		pthread_join(tids[i], NULL);
		munmap(ptrs[i], sizeof(int));
	}

	free(ptrs);
	return EXIT_SUCCESS;
}


// get the target speed, what the propeller should be spinning at, from the server
int get_target_speed_from_server(int coid, int index) {
	get_speed_msg_t msg_target;
	get_speed_resp_t resp_target;
	msg_target.type = GET_SPEED_MSG_TYPE;
	msg_target.prop_index = index;
	MsgSend(coid, &msg_target, sizeof(msg_target), &resp_target, sizeof(resp_target));
	return resp_target.target;
}


// updates the speed data in shared memory object for respective sensor to read
void update_shmem(void* ptr, int speed) {
	*(int *)ptr = speed;
}


// adjust the speed by RATE such that it matches target speed given by Flight Controller. Reacts in real-time, decreases if too high and increases if too low
void adjust_speed_to_target(int speed, int target, void* ptr) {
	while (speed != target) {
		if (speed < target) {
			speed += RATE;
		}
		else {
			speed -= RATE;
		}
		update_shmem(ptr, speed);
		usleep(ADJUST_SPEED_INTERVAL_MICROSEC);
	}
}

void *update_propeller(void *args) {
	thread_args_t *th_args = args;
	int speed, target;

	while (1)
	{
		pthread_mutex_lock(th_args->mutex);

		speed = *(int *)th_args->ptr;
		target = get_target_speed_from_server(th_args->coid, th_args->prop_index);
		adjust_speed_to_target(speed, target, th_args->ptr);

		pthread_mutex_unlock(th_args->mutex);
		sleep(PROP_WAIT);
	}
}

// random wind simulation to show system reacting and adjusting in real-time, using mutex to prevent collisions (race conditions)
void* wind(void* args) {
	int **curr_speeds = (int **) args;
	int rand1, rand2, rand3, rand4, new_speed1, new_speed2, new_speed3, new_speed4, i;
	time_t t;
	srand((unsigned) time(&t));

	while (1){
		// gust of wind hits all propellers every (sleep) seconds
		sleep(WIND_INTERVAL_SEC);

		// generate random speeds between 0 and MAX_RPM
		rand1 = (rand() % MAX_RPM+1);
		rand2 = (rand() % MAX_RPM+1);
		rand3 = (rand() % MAX_RPM+1);
		rand4 = (rand() % MAX_RPM+1);

		// ensure new speed fits evenly with the RATE intervals so that new speed will exactly match target eventually
		new_speed1 = rand1 - (rand1 % RATE);
		new_speed2 = rand2 - (rand2 % RATE);
		new_speed3 = rand3 - (rand3 % RATE);
		new_speed4 = rand4 - (rand4 % RATE);
		int new_speeds[] = {new_speed1, new_speed2, new_speed3, new_speed4};

		// acquire all mutex locks, force the new speed change, and release locks
		for (i = 0; i < NUM_PROPS; i++) {
			pthread_mutex_lock(mutexes[i]);
		}

		for (i = 0; i < NUM_PROPS; i++) {
			*(curr_speeds[i]) = new_speeds[i];
		}

		for (int i = 0; i < NUM_PROPS; i++) {
			pthread_mutex_unlock(mutexes[i]);
		}
	}
}


