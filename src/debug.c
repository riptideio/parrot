/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#include <stdarg.h>

#include "debug.h"

static FILE *debug_stream = NULL;

void enable_debug(FILE *stream) {
#ifdef DEBUG_STDERR_ALWAYS
    debug_printf("enable_debug(fd=%d)\n", (stream == NULL) ? -1 : fileno(stream));
#endif
    if (stream != NULL)
        debug_stream = stream;
    else
        debug_stream = stderr;
}

void disable_debug(void) {
#ifdef DEBUG_STDERR_ALWAYS
    debug_printf("disable_debug()\n");
#endif
    if (debug_stream != NULL && debug_stream != stderr)
        fclose(debug_stream);
    debug_stream = NULL;
}

void debug_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

#ifdef DEBUG_STDERR_ALWAYS
    va_list ap_ap;
    va_copy(ap_ap, ap);
#endif

    if (debug_stream != NULL) {
        vfprintf(debug_stream, format, ap);
        fflush(debug_stream);
    }
    va_end(ap);

#ifdef DEBUG_STDERR_ALWAYS
    va_start(ap_ap, format);
    vfprintf(stderr, format, ap_ap);
    va_end(ap_ap);
    fflush(stderr);
#endif
    return;
}
