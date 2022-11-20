#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>

#include "flight_controller.h"


typedef union
{
	struct _pulse pulse;
	char rmsg [MAX_STRING_LEN +1];
} myMessage_t;


int main(int argc, char* argv[])
{
	printf("[FC] Hi I'm FC Server\n");
	int rcvid;
	char reply_msg[MAX_STRING_LEN];
	myMessage_t msg;

	name_attach_t *attach;
	attach = name_attach(NULL, SERVER_NAME, 0);

	while(1)
	{
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		//pulse
		if (rcvid == 0) {
			switch (msg.pulse.code) {
			case _PULSE_CODE_DISCONNECT:
				printf("client is gone\n");
				ConnectDetach(msg.pulse.scoid);
				break;
			}
		}
		//message
		else {
			printf("[FC] Received message: %s\n", msg.rmsg);
			strcpy(reply_msg, "from server");
			MsgReply(rcvid, EOK, &reply_msg, sizeof(reply_msg));
		}

		memset(reply_msg, 0, sizeof(reply_msg));
		memset(msg.rmsg, 0, sizeof(msg.rmsg));
	}
	name_detach(attach, 0);

    return EXIT_SUCCESS;
}
