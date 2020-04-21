/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#ifndef ECHO_CHAMBER_SERVER_H
#define ECHO_CHAMBER_SERVER_H

#include "config.h"
#include "comm.h"

void server_loop(configuration *config, interface_struct *interface);

#endif //ECHO_CHAMBER_SERVER_H
