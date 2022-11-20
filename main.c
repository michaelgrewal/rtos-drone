#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "main.h"
#define NUM_PROCS 2

int main(int argc, char **argv)
{
	int i, status;
	pid_t pid;
	char *procs[] = {"flight_controller", "display"};

	for(i = 0; i < NUM_PROCS; i++) {
		char	*args[] = { procs[i], NULL };

		if ((pid = spawn(procs[i], 0, NULL, NULL, args, NULL)) == -1)
			perror("spawn() failed");

	}

	while (1) {
		if ((pid = wait(&status)) == -1) {
			perror("wait() failed (no more child processes?)");
			exit(EXIT_FAILURE);
		}
	}


	return 0;
}
