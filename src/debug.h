/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#ifndef ECHO_CHAMBER_LIB_H
#define ECHO_CHAMBER_LIB_H
/*
 * Uncomment `#define DEBUG_STDERR_ALWAYS' to debug debug.
 */
// #define DEBUG_STDERR_ALWAYS
/*
 * Uncomment `#define DEBUG_CONFIG' to debug configuration.
 */
// #define DEBUG_CONFIG

#include <stdio.h>

void enable_debug(FILE *stream);
void disable_debug(void);
void debug_printf(const char *format, ...);

#endif //ECHO_CHAMBER_LIB_H
