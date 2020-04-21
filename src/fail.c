/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "fail.h"

void fail(const char *format, ...) {
    va_list ap;

    fprintf(stderr, "ERROR: ");
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

void fail_with_errno(const char *format, ...) {
    va_list ap;

    fprintf(stderr, "ERROR: ");
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, ", %s.\n", strerror(errno));
    exit(1);
}
