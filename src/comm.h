/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#ifndef ECHO_CHAMBER_COMM_H
#define ECHO_CHAMBER_COMM_H

#include <stdbool.h>

#include <termios.h>
#include <unistd.h>

typedef struct {
    const char *device;
    bool is_open;
    int  fd;
    unsigned int bps;
    struct termios saved_termios;
} interface_struct;

interface_struct new_interface(const char *interface_path);
void open_interface(interface_struct *interface);
void close_interface(interface_struct *interface);
void send_packet(interface_struct *interface, const unsigned char *data, size_t len);
bool recv_packet(interface_struct *interface, unsigned char *buffer, size_t buffer_size);

#endif //ECHO_CHAMBER_COMM_H
