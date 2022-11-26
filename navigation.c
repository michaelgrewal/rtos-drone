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
    
    set_target_speed_msg_t msg;
    msg.type = SET_TARGET_SPEED_MSG_TYPE;
    msg.nav_data.direction = 0;
    msg.nav_data.duration = 0;

    coid = name_open(SERVER_NAME, 0);

    MsgSend(coid, &msg, sizeof(msg), NULL, 0);

    name_close(coid);

    return EXIT_SUCCESS;
}
