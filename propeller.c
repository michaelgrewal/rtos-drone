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

// forward declarations for thread functions
void *prop1(void *);
void *prop2(void *);
void *prop3(void *);
void *prop4(void *);
void *wind(void *);

// mutexes so wind simulation and propellers don't collide with data access (prevent race conditions)
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *mutexes[] = {&mutex1, &mutex2, &mutex3, &mutex4};



int main(void) {
	// arrays for each thread's function
	const void *funcs[] = {&prop1, &prop2, &prop3, &prop4};

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

		struct thread_args *args = malloc(sizeof(struct thread_args));
		args->ptr = ptr;
		args->coid = coid;

		// create attr for ROUND ROBIN scheduling and create thread
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(NULL, &attr, funcs[i], (void *)args);
	}

	// thread to simulate wind
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_create(NULL, &attr, &wind, (void *)ptrs);

	sleep(STAY_ALIVE_TIME);	// TODO (maybe pthread join to wait for them instead)
	return 0;
}


// get the target speed, what the propeller should be spinning at, from the server
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
		usleep(250000);
	}
}


// thread functions
// each propeller will update their current speed into shared memory object
void* prop1(void* args) {
	struct thread_args *th_args = args;
	int speed, target;

	while (1)
	{
		pthread_mutex_lock(&mutex1);

		speed = *(int *)th_args->ptr;
		target = get_target_speed_from_server(th_args->coid);
		adjust_speed_to_target(speed, target, th_args->ptr);

		pthread_mutex_unlock(&mutex1);
		sleep(1);

	}
}

void* prop2(void* args) {
	struct thread_args *th_args = args;
	int speed, target;

	while (1)
	{
		pthread_mutex_lock(&mutex2);

		speed = *(int *)th_args->ptr;
		target = get_target_speed_from_server(th_args->coid);
		adjust_speed_to_target(speed, target, th_args->ptr);

		pthread_mutex_unlock(&mutex2);
		sleep(1);

	}
}

void* prop3(void* args) {
	struct thread_args *th_args = args;
	int speed, target;

	while (1)
	{
		pthread_mutex_lock(&mutex3);

		speed = *(int *)th_args->ptr;
		target = get_target_speed_from_server(th_args->coid);
		adjust_speed_to_target(speed, target, th_args->ptr);

		pthread_mutex_unlock(&mutex3);
		sleep(1);

	}
}

void* prop4(void* args) {
	struct thread_args *th_args = args;
	int speed, target;

	while (1)
	{
		pthread_mutex_lock(&mutex4);

		speed = *(int *)th_args->ptr;
		target = get_target_speed_from_server(th_args->coid);
		adjust_speed_to_target(speed, target, th_args->ptr);

		pthread_mutex_unlock(&mutex4);
		sleep(1);
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
		sleep(5);

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


