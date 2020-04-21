/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#ifndef ECHO_CHAMBER_CLIENT_H
#define ECHO_CHAMBER_CLIENT_H

#include "config.h"
#include "comm.h"

void client_loop(configuration *config, interface_struct *interface);

#endif //ECHO_CHAMBER_CLIENT_H
