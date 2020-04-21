/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#include "client.h"
#include "server.h"

int main(int argc, char* argv[]) {
    configuration config = get_configuration(argc, argv);
    interface_struct interface = new_interface(config.interface);
    open_interface(&interface);
    switch (config.role) {
        case ROLE_CLIENT:
            client_loop(&config, &interface);
            break;
        case ROLE_SERVER:
            server_loop(&config, &interface);
            break;
    }
    return 0;
}
