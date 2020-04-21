/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#ifndef ECHO_CHAMBER_CONFIG_H
#define ECHO_CHAMBER_CONFIG_H

#include <stdbool.h>

#define ROLE_NONE -1
#define ROLE_CLIENT 0
#define ROLE_SERVER 1

/* Configuration compatible with misty configuration. */
typedef struct
{
    const char *ini_file;    /* --config CONFIG_FILE; -c  CONFIG_FILE */
    int role;                /* role: ROLE; --role ROLE; -r ROLE */
    bool set_role;
    long baudrate;           /* baudrate: BAUDRATE; --baudrate BAUDRATE; -b BAUDRATE */
    bool set_baudrate;
    const char* interface;   /* interface: INTERFACE; --interface INTERFACE; -i INTERFACE */
    bool set_interface;
    const char* debug;       /* mstpdbgfile: FILE; --debug [FILE]; -d [FILE] */
    bool set_debug;
} configuration;

#define CONFIGURATION_DEFAULTS {\
        .ini_file = NULL, \
        .role = ROLE_NONE, \
        .set_role = false, \
        .baudrate = 38400, \
        .set_baudrate = false, \
        .interface = NULL, \
        .set_interface = false, \
        .debug = NULL, \
        .set_debug = false \
}

void debug_print_config(const configuration *c);
configuration get_configuration(int argc, char* argv[]);

#endif //ECHO_CHAMBER_CONFIG_H
