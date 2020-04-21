/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#ifndef ECHO_CHAMBER_ERROR_H
#define ECHO_CHAMBER_ERROR_H

void fail(const char *format, ...) __attribute__((noreturn));
void fail_with_errno(const char *format, ...) __attribute__((__noreturn__));

#endif //ECHO_CHAMBER_ERROR_H
