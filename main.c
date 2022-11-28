#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "main.h"

const char *get_proc_name(char * const *procs, pid_t *pids, size_t len, pid_t pid);

// Parent main process to initiate the Drone
int main(int argc, char **argv) {
	int i, status;

	// array of all procs that need to start
	char * const procs[] = {"flight_controller", "navigation", "display", "propeller", "sensor"};
	const char *proc;
	size_t len = sizeof(procs) / sizeof(procs[0]);

	pid_t pids[len];
	pid_t pid;

	// spawn each subprocess
	for(i = 0; i < len; i++) {
		char * const args[] = { procs[i], NULL };

		if ((pid = spawn(procs[i], 0, NULL, NULL, args, NULL)) == -1)
			perror("spawn() failed");

		pids[i] = pid;
	}

	// wait for child processes to return
	for (i = 0; i < len; ++i) {
		if ((pid = wait(&status)) == -1) {
			perror("wait() failed (no more child processes?)");
			exit(EXIT_FAILURE);
		}

		if ((proc = get_proc_name(procs, pids, len, pid))) {
			printf("Process %s (pid: %d) finished.\n", proc, pid);
		} else {
			fprintf(stderr, "Error, unknown process with pid %d returned\n", pid);
			return EXIT_FAILURE;
		}
	}

	printf("All processes finsihed. Exiting\n");
	return EXIT_SUCCESS;
}

const char *get_proc_name(char * const *procs, pid_t *pids, size_t len, pid_t pid) {
	size_t i;
	for(i = 0; i < len; ++i) {
		if (pids[i] == pid) {
			return procs[i];
		}
	}

	return NULL;
}
