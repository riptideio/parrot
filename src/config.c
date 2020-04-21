/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <argp.h>

#include "inih-r48/ini.h"
#include "debug.h"
#include "config.h"
#include "fail.h"

#define EC_VERSION "%VERSION%"

const char *argp_program_version = EC_VERSION;
const char *argp_program_bug_address = "<support@riptideio.com>";
static char doc[] = "parrot -- Used to test / \"burn-in\" serial ports.";
static struct argp_option options[] =  {
    {"baudrate",   'b', "BAUDRATE", 0,
     "Set baudrate (default 38400)"},
    {"bps",         0, "BPS", OPTION_ALIAS},
    {"config",     'c', "FILE", 0,
     "Read configuration from FILE"},
    {"debug",      'd', "FILE",  OPTION_ARG_OPTIONAL,
     "Enable debugging messages to FILE (or stderr if FILE not specified.)"},
    {"debug-file", 'D', "FILE",  0,
     "Enable debugging messages to FILE (or stderr if FILE == \"\".)"},
    {"interface",  'i', "INTERFACE", 0,
     "Serial INTERFACE to use (REQUIRED)"},
    {"role",       'r', "ROLE", 0,
     "Set ROLE to client or server (REQUIRED)"},
    {0}
};

#define CONFIG_FORMAT "\
 .ini_file = %s\n\
 .role = %s\n\
 .set_role = %d\n\
 .baudrate = %d\n\
 .set_baudrate = %d\n\
 .interface = %s\n\
 .set_interface = %d\n\
 .debug = %s,\n\
 .set_debug = %d\n"

void debug_print_config(const configuration *c) {
    debug_printf(
            CONFIG_FORMAT,
            c->ini_file,
            (c->role == ROLE_SERVER) ? "server" : (c->role == ROLE_CLIENT) ? "client" : "NONE",
            c->set_role,
            c->baudrate,
            c->set_baudrate,
            c->interface,
            c->set_interface,
            c->debug,
            c->set_debug
    );
}

static int ini_file_handler(void* user, const char* section, const char* name,
                     const char* value, int lineno)
{
    configuration* pconfig = (configuration*)user;
#ifdef DEBUG_STDERR_ALWAYS
    debug_printf(
        "ini_file_handler(user='%s', section='%s', name='%s', value='%s', lineno=%d)\n",
        user, section, name, value, lineno
    );
#endif
    if (strcmp(name, "baudrate") == 0 || strcmp(name, "bps") == 0) {
        pconfig->set_baudrate = true;
        pconfig->baudrate = strtol(value, NULL, 10);
        if (pconfig->baudrate == 0) {
            fail("invalid baudrate '%s' on line %d.", value, lineno);
        }
    } else if (strcmp(name, "debug") == 0 || strcmp(name, "debug-file") == 0 || strcmp(name, "mstpdbgfile") == 0) {
        pconfig->set_debug = true;
        pconfig->debug = strdup(value);
    } else if (strcmp(name, "interface") == 0) {
        pconfig->set_interface = true;
        pconfig->interface = strdup(value);
    } else if (strcmp(name, "role") == 0) {
        pconfig->set_role = true;
        if (strcmp(value, "client") == 0) {
            pconfig->role = ROLE_CLIENT;
        } else if (strcmp(value, "server")) {
            pconfig->role = ROLE_SERVER;
        } else {
            fail("invalid role '%s' on line %d, must be 'client' or 'server'.", value, lineno);
        }
    }
    return 1;
}

static error_t option_handler(int key, char *arg, struct argp_state *state) {
    configuration *overrides = state->input;

    switch (key) {
        case 'b':
            overrides->set_baudrate = true;
            overrides->baudrate = strtol(arg, NULL, 10);
            break;
        case 'c':
            overrides->ini_file = strdup(arg);
            break;
        case 'd':
            /* hack to support "-d FILE" and -d=FILE. Optional args sort of suck. */
            while (arg && strlen(arg) && (arg[0] == ' ' || arg[0] == '=')) arg++ ;
        case 'D':
            overrides->set_debug = true;
            overrides->debug = (arg == NULL) ? "" : strdup(arg) ;
            break;
        case 'i':
            overrides->set_interface = true;
            overrides->interface = arg;
            break;
        case 'r':
            overrides->set_role = true;
            if (strcmp(arg, "client") == 0) {
                overrides->role = ROLE_CLIENT;
            } else if (strcmp(arg, "server") == 0) {
                overrides->role = ROLE_SERVER;
            } else {
                fail("invalid role '%s', must be 'client' or 'server'.", arg);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static void override_config(configuration *config, const configuration *overrides) {
    if (overrides->set_baudrate) {
        config->set_baudrate = overrides->set_baudrate;
        config->baudrate = overrides->baudrate;
    }
    if (overrides->ini_file) {
        config->ini_file = overrides->ini_file;
    }
    if (overrides->set_debug) {
        config->set_debug = overrides->set_debug;
        config->debug = overrides->debug;
    }
    if (overrides->set_interface) {
        config->set_interface = overrides->set_interface;
        config->interface = overrides->interface;
    }
    if (overrides->set_role) {
        config->set_role = overrides->set_role;
        config->role = overrides->role;
    }
}

static void validate_config(const configuration *config) {
    if (!config->set_role) {
        fail("role must be set.");
    }
    if (!config->set_interface) {
        fail("interface must be set.");
    }
}


static void config_debug(configuration *cfg) {
#ifdef DEBUG_STDERR_ALWAYS
    debug_printf("config_debug: cfg->set_debug=%d, cfg->debug=%s\n", cfg->set_debug, cfg->debug);
#endif
    if (cfg->set_debug) {
        if (cfg->debug == NULL || strlen(cfg->debug) == 0)
            enable_debug(NULL);
        else {
            FILE *stream = fopen(cfg->debug, "w");
            if (stream == NULL) {
                fail_with_errno("Could not open debug log: %s", cfg->debug);
            }
#ifdef DEBUG_STDERR_ALWAYS
            debug_printf("config_debug: FILE *stream = fopen(\"%s\", \"w\");\n", cfg->debug);
#endif
            enable_debug(stream);
        }
    } else {
        disable_debug();
    }
}

static struct argp argp = { options, option_handler, 0, doc };

configuration get_configuration(int argc, char* argv[]) {
    configuration config = CONFIGURATION_DEFAULTS;
    configuration overrides = CONFIGURATION_DEFAULTS;
    /* Arguments have the highest precedence. */
    argp_parse(&argp, argc, argv, 0, 0, &overrides);
    config_debug(&overrides);
    debug_printf("overrides:\n");
    debug_print_config(&overrides);
    if (overrides.ini_file != NULL) {
        /* If there is a config file, load it. */
        if (ini_parse(overrides.ini_file, ini_file_handler, &config) < 0) {
            fail("Can't load '%s' as a configuration file\n", overrides.ini_file);
        }
        debug_printf("%s:\n", overrides.ini_file);
        debug_print_config(&config);
    }
    /* Update the configuration from the arguments (aka overrides.) */
    override_config(&config, &overrides);
    config_debug(&config);
    debug_printf("merged config:\n");
    debug_print_config(&config);
    validate_config(&config);
    return config;
}