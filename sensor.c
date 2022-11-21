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


void *sens1(void *);
void *sens2(void *);
void *sens3(void *);
void *sens4(void *);




int main(void) {
	const char *shmem_props[] = {"prop1", "prop2", "prop3", "prop4"};
	const void *funcs[] = {&sens1, &sens2, &sens3, &sens4};

	int fd, i;
	void *ptr;
//	printf("[S] Hi I'm sensor.\n");

	for (i = 0; i < NUM_SENS; i++) {
		fd = shm_open(shmem_props[i], O_RDONLY, 0);
		if (fd == -1) {
			perror("shm_open() in sensors failed");
			return EXIT_FAILURE;
		}
		ptr = mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, fd, 0);
		if (ptr == MAP_FAILED) {
			perror("mmap failed\n");
			return EXIT_FAILURE;
		}
		close(fd);

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_create(NULL, NULL, funcs[i], ptr);
	}
	sleep(5);
	return 0;
}

void send_speed_to_server(int coid, int speed, int code) {
	//send pulse to server
	MsgSendPulse(coid, -1, code, speed);
}

int read_prop_speed(void *ptr) {
	return *(int *)ptr;
}


void* sens1(void* ptr) {
	int coid, speed1;
	coid = name_open(SERVER_NAME, 0);

	while (1)
	{
		speed1 = read_prop_speed(ptr);
		send_speed_to_server(coid, speed1, _PULSE_CODE_UPDATE_SPEED1);
	}
}

void* sens2(void* ptr) {
	int coid, speed2;
	coid = name_open(SERVER_NAME, 0);

	while (1)
	{
		speed2 = read_prop_speed(ptr);
		send_speed_to_server(coid, speed2, _PULSE_CODE_UPDATE_SPEED2);
	}
}

void* sens3(void* ptr) {
	int coid, speed3;
	coid = name_open(SERVER_NAME, 0);

	while (1)
	{
		speed3 = read_prop_speed(ptr);
		send_speed_to_server(coid, speed3, _PULSE_CODE_UPDATE_SPEED3);
	}
}

void* sens4(void* ptr) {
	int coid, speed4;
	coid = name_open(SERVER_NAME, 0);

	while (1)
	{
		speed4 = read_prop_speed(ptr);
		send_speed_to_server(coid, speed4, _PULSE_CODE_UPDATE_SPEED4);
	}
}


