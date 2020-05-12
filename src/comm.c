/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "comm.h"
#include "debug.h"
#include "fail.h"

#define NEW_INTERFACE { "", false, -1, B38400 }

static unsigned char const crc8_table[] = {
        0xea, 0xd4, 0x96, 0xa8, 0x12, 0x2c, 0x6e, 0x50, 0x7f, 0x41, 0x03, 0x3d,
        0x87, 0xb9, 0xfb, 0xc5, 0xa5, 0x9b, 0xd9, 0xe7, 0x5d, 0x63, 0x21, 0x1f,
        0x30, 0x0e, 0x4c, 0x72, 0xc8, 0xf6, 0xb4, 0x8a, 0x74, 0x4a, 0x08, 0x36,
        0x8c, 0xb2, 0xf0, 0xce, 0xe1, 0xdf, 0x9d, 0xa3, 0x19, 0x27, 0x65, 0x5b,
        0x3b, 0x05, 0x47, 0x79, 0xc3, 0xfd, 0xbf, 0x81, 0xae, 0x90, 0xd2, 0xec,
        0x56, 0x68, 0x2a, 0x14, 0xb3, 0x8d, 0xcf, 0xf1, 0x4b, 0x75, 0x37, 0x09,
        0x26, 0x18, 0x5a, 0x64, 0xde, 0xe0, 0xa2, 0x9c, 0xfc, 0xc2, 0x80, 0xbe,
        0x04, 0x3a, 0x78, 0x46, 0x69, 0x57, 0x15, 0x2b, 0x91, 0xaf, 0xed, 0xd3,
        0x2d, 0x13, 0x51, 0x6f, 0xd5, 0xeb, 0xa9, 0x97, 0xb8, 0x86, 0xc4, 0xfa,
        0x40, 0x7e, 0x3c, 0x02, 0x62, 0x5c, 0x1e, 0x20, 0x9a, 0xa4, 0xe6, 0xd8,
        0xf7, 0xc9, 0x8b, 0xb5, 0x0f, 0x31, 0x73, 0x4d, 0x58, 0x66, 0x24, 0x1a,
        0xa0, 0x9e, 0xdc, 0xe2, 0xcd, 0xf3, 0xb1, 0x8f, 0x35, 0x0b, 0x49, 0x77,
        0x17, 0x29, 0x6b, 0x55, 0xef, 0xd1, 0x93, 0xad, 0x82, 0xbc, 0xfe, 0xc0,
        0x7a, 0x44, 0x06, 0x38, 0xc6, 0xf8, 0xba, 0x84, 0x3e, 0x00, 0x42, 0x7c,
        0x53, 0x6d, 0x2f, 0x11, 0xab, 0x95, 0xd7, 0xe9, 0x89, 0xb7, 0xf5, 0xcb,
        0x71, 0x4f, 0x0d, 0x33, 0x1c, 0x22, 0x60, 0x5e, 0xe4, 0xda, 0x98, 0xa6,
        0x01, 0x3f, 0x7d, 0x43, 0xf9, 0xc7, 0x85, 0xbb, 0x94, 0xaa, 0xe8, 0xd6,
        0x6c, 0x52, 0x10, 0x2e, 0x4e, 0x70, 0x32, 0x0c, 0xb6, 0x88, 0xca, 0xf4,
        0xdb, 0xe5, 0xa7, 0x99, 0x23, 0x1d, 0x5f, 0x61, 0x9f, 0xa1, 0xe3, 0xdd,
        0x67, 0x59, 0x1b, 0x25, 0x0a, 0x34, 0x76, 0x48, 0xf2, 0xcc, 0x8e, 0xb0,
        0xd0, 0xee, 0xac, 0x92, 0x28, 0x16, 0x54, 0x6a, 0x45, 0x7b, 0x39, 0x07,
        0xbd, 0x83, 0xc1, 0xff};
static unsigned char crc8(unsigned char crc, unsigned char const *data, size_t len)
{
    unsigned char const *end = data + len;
    while (data < end)
        crc = crc8_table[crc ^ *data++];
    return crc;
}

static interface_struct __atexit_interface_hack;
static void __atexit_hack(void) {
    close_interface(&__atexit_interface_hack);
}

static volatile sig_atomic_t timeout = false;

static void alarm_handler(int signal) {
    timeout = true;
    return;
}

static void clear_timeout(void) {
    alarm(0);
    timeout = false;
}

static void set_timeout(int seconds) {
    alarm(seconds);
    timeout = false;
}

interface_struct new_interface(const char *interface_path) {
    interface_struct interface = NEW_INTERFACE;
    interface.device = strdup(interface_path);
    return interface;
}

void close_interface(interface_struct *interface) {
    clear_timeout();
    if (interface->is_open) {
        fprintf(stderr, "Closing down %s serial device...", __atexit_interface_hack.device);
        fflush(stderr);
        tcsetattr(interface->fd, TCSANOW, &interface->saved_termios);
        ioctl(interface->fd, TIOCSSERIAL, &interface->saved_serial);
        close(interface->fd);
        interface->is_open = false;
        interface->fd = -1;
        fprintf(stderr, "done.\n");
    }
}

typedef struct {
    char *name;
    unsigned int flag;
} flag_names;

flag_names known_flags [] = {
    {.name = "ASYNC_HUP_NOTIFY", .flag = ASYNC_HUP_NOTIFY},
    {.name = "ASYNC_FOURPORT ", .flag = ASYNC_FOURPORT},
    {.name = "ASYNC_SAK", .flag = ASYNC_SAK},
    {.name = "ASYNC_SPLIT_TERMIOS", .flag = ASYNC_SPLIT_TERMIOS},
    {.name = "ASYNC_SPD_MASK", .flag = ASYNC_SPD_MASK},
    {.name = "ASYNC_SPD_VHI", .flag = ASYNC_SPD_VHI},
    {.name = "ASYNC_SPD_CUST", .flag = ASYNC_SPD_CUST},
    {.name = "ASYNC_SKIP_TEST", .flag = ASYNC_SKIP_TEST},
    {.name = "ASYNC_AUTO_IRQ ", .flag = ASYNC_AUTO_IRQ},
    {.name = "ASYNC_SESSION_LOCKOUT", .flag = ASYNC_SESSION_LOCKOUT},
    {.name = "ASYNC_PGRP_LOCKOUT   ", .flag = ASYNC_PGRP_LOCKOUT},
    {.name = "ASYNC_CALLOUT_NOHUP  ", .flag = ASYNC_CALLOUT_NOHUP},
    {.name = "ASYNC_HARDPPS_CD", .flag = ASYNC_HARDPPS_CD},
    {.name = "ASYNC_SPD_SHI", .flag = ASYNC_SPD_SHI},
    {.name = "ASYNC_SPD_WARP", .flag = ASYNC_SPD_WARP},
    {.name = "ASYNC_LOW_LATENCY", .flag = ASYNC_LOW_LATENCY}
};

static void debug_serial_settings(struct serial_struct *serial_settings) {
    debug_printf("  serial_settings:\n");
    debug_printf("  type %d\n", serial_settings->type);
    debug_printf("  line %d\n", serial_settings->line);
    debug_printf("  port %u\n", serial_settings->port);
    debug_printf("  irq %d\n", serial_settings->irq);
    // debug_printf("  flags %d\n", serial_settings->flags);
    debug_printf("  flags ", serial_settings->flags);
    for (int i=0; i< sizeof(known_flags)/ sizeof(flag_names); ++i) {
        if (serial_settings->flags & known_flags[i].flag) {
            debug_printf("%s ", known_flags[i].name);
        }
    }
    debug_printf("\n");
    debug_printf("  xmit_fifo_size %d\n", serial_settings->xmit_fifo_size);
    debug_printf("  custom_divisor %d\n", serial_settings->custom_divisor);
    debug_printf("  baud_base %d\n", serial_settings->baud_base);
    debug_printf("  close_delay %hu\n", serial_settings->close_delay);
    debug_printf("  hub6 %d\n", serial_settings->hub6);
    debug_printf("  closing_wait %hu\n", serial_settings->closing_wait);
    debug_printf("  iomem_base %p\n", serial_settings->iomem_base);
    debug_printf("  iomem_reg_shift %hu\n", serial_settings->iomem_reg_shift);
    debug_printf("  port_high %u\n", serial_settings->port_high);
    debug_printf("  iomap_base %lu\n", serial_settings->iomap_base);
}

void open_interface(interface_struct *interface) {
    struct termios termios_settings;
    struct serial_struct serial_settings;
    int fd;
    if (interface->is_open) {
        close_interface(interface);
    }
    fd = open(interface->device, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        fail_with_errno("Could not open '%s'", interface->device);
    }
    interface->fd = fd;
    fcntl(interface->fd, F_SETFL, 0);
    memset(&termios_settings, 0, sizeof(termios_settings));
    memset(&serial_settings, 0, sizeof(serial_settings));
    if (ioctl(fd, TIOCGSERIAL, &serial_settings) < 0) {
        fail_with_errno("Cannot get serial info");
    }
    memcpy(&interface->saved_serial, &serial_settings, sizeof(serial_settings));
    debug_serial_settings(&serial_settings);
    termios_settings.c_cc[VMIN] = 1;
    termios_settings.c_cc[VTIME] = 100;
    termios_settings.c_cflag = interface->bps | CS8 | CLOCAL | CREAD;
    termios_settings.c_iflag = 0;
    termios_settings.c_lflag = 0;
    termios_settings.c_oflag = 0;
    tcgetattr(interface->fd, &interface->saved_termios);
    tcsetattr(interface->fd, TCSAFLUSH, &termios_settings);
    interface->is_open = true;
    __atexit_interface_hack = *interface;
    atexit(__atexit_hack);
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    return;
}

static void send_byte(int fd, const unsigned char b) {
    if (write(fd, &b, 1) < 0) {
        fail_with_errno("Send byte failed");
    }
}

static bool recv_byte(int fd, unsigned char *p_byte, const char *message) {
    int n = read(fd, p_byte, 1);
    if (timeout || (n < 0 && errno == EINTR)) {
        if (timeout || errno == EINTR) {
            debug_printf("read timed out\n");
            return false;
        }
        fail_with_errno(message);
    }
    return n ? true : false;
}

void send_packet(interface_struct *interface, const unsigned char *data, size_t len) {
    unsigned char _len;
    unsigned char _crc;
    if (len > 255)
        fail("%d is too long for max packet size.");
    debug_printf("send_packet: sending %d byte packet.\n", len);
    _len = (unsigned char)(len & 0xFF);
    set_timeout(10);
    send_byte(interface->fd, '\xFF');
    send_byte(interface->fd, '\x55');
    send_byte(interface->fd, _len);
    _crc = crc8(0, data, len);
    while (_len--)
        send_byte(interface->fd, *data++);
    send_byte(interface->fd, _crc);
    if (tcdrain(interface->fd) < 0) {
        fail_with_errno("tcdrain failed");
    }
    clear_timeout();
    debug_printf("Sent %d byte packet.\n", len);
}

bool recv_packet(interface_struct *interface, unsigned char *buffer, size_t buffer_size) {
    unsigned char a_byte, n_bytes;
    int n_read;
    unsigned char crc = 0;

    if (buffer_size < 256) {
        fail("buffer must be at least 256 bytes long.");
    }
    memset(buffer, 0, buffer_size);

    set_timeout(5);
    /* Cheesy scan for beginning of packet ... */
    // debug_printf("scan for 0xFF\n");
    for (a_byte=0; a_byte!=0xFF; )
        if (!recv_byte(interface->fd, &a_byte, "Unexpected error scanning for 0xFF"))
            return false;

    // debug_printf("scan for 0x55\n");
    for (a_byte=0; a_byte!=0x55; )
        if (!recv_byte(interface->fd, &a_byte, "Unexpected error scanning for 0x55"))
            return false;

    set_timeout(1);
    // debug_printf("read length.\n");
    if (!recv_byte(interface->fd, &n_bytes, "Unexpected error scanning for packet length"))
        return false;

    n_read = 0;
    while (n_bytes--) {
        set_timeout(1);
        if (!recv_byte(interface->fd, &a_byte, "Unexpected error reading packet bytes"))
            return false;
        buffer[n_read++] = a_byte;
        crc = crc8(crc, &a_byte, 1);
    }

    set_timeout(1);
    if (!recv_byte(interface->fd, &a_byte, "Unexpected error reading for packet crc"))
        return false;
    clear_timeout();

    if (crc != a_byte) {
        debug_printf("CRC mismatch reading packet. Expected %02X, read %02X.\n", crc, a_byte);
        return false;
    }
    debug_printf("Read valid %d byte packet.\n", n_read);
    return true;
}