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

int speed1, speed2, speed3, speed4;
volatile int state = 1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *prop1(void *);
void *prop2(void *);
void *prop3(void *);
void *prop4(void *);




int main(void) {

	int fd;
	void *ptr;
	printf("[P] Hi I'm propeller client.\n");

	fd = shm_open(SHM_NAME, O_RDWR, 0);
	if (fd == -1) {
		perror("shm_open() failed");
		return EXIT_FAILURE;
	}
	ptr = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	sleep(2);

	pthread_create(NULL, NULL, &prop1, ptr);
	pthread_create(NULL, NULL, &prop2, ptr);
	pthread_create(NULL, NULL, &prop3, ptr);
	pthread_create(NULL, NULL, &prop4, ptr);

	sleep(5);

	return 0;
}

void update_shmem(void* ptr, int speed, int n) {
	int i = n-1;
	((int *)ptr)[i] = speed;
}


void* prop1(void* ptr) {
	while (1)
	{
		pthread_mutex_lock(&mutex);
		while(state != 1)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		speed1 = 555;
		update_shmem(ptr, speed1, state);
		state = 2;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}
}

void* prop2(void* ptr) {
	while (1)
	{
		pthread_mutex_lock(&mutex);
		while(state != 2)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		speed2 = 666;
		update_shmem(ptr, speed2, state);
		state = 3;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}
}

void* prop3(void* ptr) {
	while (1)
	{
		pthread_mutex_lock(&mutex);
		while(state != 3)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		speed3 = 777;
		update_shmem(ptr, speed3, state);
		state = 4;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}
}

void* prop4(void* ptr) {
	while (1)
	{
		pthread_mutex_lock(&mutex);
		while(state != 4)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		speed4 = 888;
		update_shmem(ptr, speed4, state);
		state = 1;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}
}


