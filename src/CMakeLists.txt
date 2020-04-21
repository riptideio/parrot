#  Copyright (c) 2020 Riptide IO, Inc.
#  All rights reserved.

cmake_minimum_required(VERSION 3.5)
project(parrot C)

set(CMAKE_C_STANDARD 99)

add_definitions(
        -DINI_ALLOW_BOM=0
        -DINI_STOP_ON_FIRST_ERROR=1
        -DINI_HANDLER_LINENO=1
        -DINI_MAX_LINE=1024
)

add_executable(parrot main.c client.c comm.c config.c debug.c fail.c server.c inih-r48/ini.c)
target_compile_options(parrot PUBLIC "-DINI_INLINE_COMMENT_PREFIXES=\"\#\"")
target_compile_options(parrot PUBLIC "-DINI_START_COMMENT_PREFIXES=\"\#\"")
install(TARGETS parrot DESTINATION bin)