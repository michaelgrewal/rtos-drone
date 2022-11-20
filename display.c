#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "display.h"


int main(int argc, char* argv[])
{
	char smsg[MAX_STRING_LEN];
	char rmsg[MAX_STRING_LEN];
	int coid;
	printf("[D] Hi I'm display HUD client.\n");

	//set the message value to 'test'
	strcpy(smsg, "test");

	//establish a connection to the server's channel
	coid = name_open(SERVER_NAME, 0);

	//send the message to the server and get a reply
	MsgSend(coid, &smsg, sizeof(smsg), &rmsg, sizeof(rmsg));

    //print received checksum
    
	printf("[D] Received message: %s\n", rmsg);


    //Close a server connection that was opened by name_open()
	name_close(coid);

    return EXIT_SUCCESS;
}
