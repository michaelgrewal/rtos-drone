#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "navigation.h"
#include "flight_controller.h"

int main(int argc, char **argv) {
    int coid;
	FILE *fp;
	char line[1024];
	set_speeds_msg_t msg;

	char direction[64];
	int duration;
	int command;

	coid = name_open(SERVER_NAME, 0);
	fp = fopen( "input.txt", "r" );

	if( fp != NULL ) {

		// get line by line file for reading
		while( fgets(line,1024,fp) ) {
			// split the data in the line into direction and duration
			command = sscanf(line, "%s %d", direction, &duration);
			if (command == 2) {
				msg.type = SET_SPEEDS_MSG_TYPE;
				msg.nav_data.direction = atoi(direction); // Hackjob for now to not have to recompile. Will add a file later
				msg.nav_data.duration = duration;

				MsgSend(coid, &msg, sizeof(msg), NULL, 0);
				sleep(duration);
			 }
			 else{
				 printf("Failed to scan all values\n");
			 }
		}
		fclose( fp );
	}

    name_close(coid);

    return EXIT_SUCCESS;
}
