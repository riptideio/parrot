/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#include <string.h>

#include "debug.h"
#include "client.h"

static unsigned char buffer[1024];

void client_loop(configuration *config, interface_struct *interface) {
    debug_printf("client_loop\n");
    while (1) {
        if (recv_packet(interface, buffer, sizeof(buffer)))
            send_packet(interface, buffer, strlen(buffer)+1);
    }
}
