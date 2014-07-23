/*
 * CORE                                                                     ---
 *
 * Version:                                     @(#)core.c    0.0.1    09/07/14
 * Authors:             Maciej A. Czyzewski, <maciejanthonyczyzewski@gmail.com>
 *
 * Fixes:
 *      M. A. Czyzewski :   Some small cleanups, optimizations, and fixed a
 *                  core_close() bug.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <hb_core.h>

extern struct server server;
extern struct client client;

/* Output command version. */
static void version(core_t *self)
{
    printf("%s\n", self->version);
    core_free(self);
    core_close(0);
}

/* Output command help. */
static void help(core_t *self)
{
    printf("\n");
    printf("  Usage: %s %s\n", self->name, self->usage);
    printf("\n");
    printf("  Options:\n");
    printf("\n");

    int i;

    for (i = 0; i < self->option_count; ++i) {
        core_option_t *option = &self->options[i];
        printf("    %s, %-25s %s\n"
               , option->small
               , option->large_with_arg
               , option->description);
    }

    printf("\n");
    core_free(self);
    core_close(0);
}

static void daemonize(core_t *self)
{
    server.daemonize = true;
    server.pid = fork();

    if (server.pid < 0) {
        fprintf(stdout, "hb: %s fork failed\n", HB_LOG_ERR);
        core_close(1);
    }

    if (server.pid > 0) {
        fprintf(stdout, "hb: %s daemon process running [fd: %d]...\n", HB_LOG_OK, server.pid);
        core_close(0);
    }

    umask(0);

    if ((server.pid = setsid()) < 0) {
        core_close(1);
    }

    server.status = chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

static void port(core_t *self)
{
    server.port = atoi(self->arg);
}

void core_init(int argc, char *argv[])
{
    core_t cmd;

    cmd.arg = NULL;
    cmd.name = argv[0];
    cmd.version = HB_VERSION;
    cmd.option_count = cmd.argc = 0;
    cmd.usage = "[options]";
    cmd.nargv = NULL;

    core_option(&cmd, "-V", "--version", "show version, usage, options, and exit", version);
    core_option(&cmd, "-h", "--help", "show version and exit", help);

    core_option(&cmd, "-d", "--daemonize", "run hashbase as a daemon", daemonize);
    core_option(&cmd, "-p", "--port <arg>", "set the tcp port to listen on (default: 5555)", port);

    core_parse(&cmd, argc, argv);

    core_free(&cmd);
}

void core_close(int code)
{
    server.keepRunning = false;

    close(client.socket);
    close(server.socket);

    if (code == 2) fprintf(stdout, "\n");
    fprintf(stdout, "hb: %s closing hashbase...\n", HB_LOG_INF);

    switch (code) {
        /* Signal close */
        case 2:
            exit(0);
            break;
        /* Error */
        case 1:
            exit(1);
            break;
        /* Default */
        default:
            exit(0);
            break;
    }
}

/* Free up commander after use. */
void core_free(core_t *self)
{
    int i;

    for (i = 0; i < self->option_count; ++i) {
        core_option_t *option = &self->options[i];
        free(option->argname);
        free(option->large);
    }

    if (self->nargv) {
        for (i = 0; self->nargv[i]; ++i) {
            free(self->nargv[i]);
        }
        free(self->nargv);
    }
}

/* Parse argname from `str`. For example
 * Take "--required <arg>" and populate `flag`
 * with "--required" and `arg` with "<arg>". */
static void parse_argname(const char *str, char *flag, char *arg)
{
    int buffer = 0;
    size_t flagpos = 0;
    size_t argpos = 0;
    size_t len = strlen(str);
    size_t i;

    for (i = 0; i < len; ++i) {
        if (buffer || '[' == str[i] || '<' == str[i]) {
            buffer = 1;
            arg[argpos++] = str[i];
        } else {
            if (' ' == str[i]) continue;
            flag[flagpos++] = str[i];
        }
    }

    arg[argpos] = '\0';
    flag[flagpos] = '\0';
}

/* Normalize the argument vector by exploding
 * multiple options (if any). For example
 * "foo -abc --scm git" -> "foo -a -b -c --scm git" */
static char ** normalize_args(int *argc, char **argv)
{
    int size = 0;
    int alloc = *argc + 1;
    char **nargv = malloc(alloc * sizeof(char *));
    int i;
    size_t j;

    for (i = 0; argv[i]; ++i) {
        const char *arg = argv[i];
        size_t len = strlen(arg);

        /* Short flag. */
        if (len > 2 && '-' == arg[0] && !strchr(arg + 1, '-')) {
            alloc += len - 2;
            nargv = realloc(nargv, alloc * sizeof(char *));
            for (j = 1; j < len; ++j) {
                nargv[size] = malloc(3);
                sprintf(nargv[size], "-%c", arg[j]);
                size++;
            }
            continue;
        }

        /* Regular arg. */
        nargv[size] = malloc(len + 1);
        strcpy(nargv[size], arg);
        size++;
    }

    nargv[size] = NULL;
    *argc = size;
    return nargv;
}

/* Define an option. */
void core_option(core_t *self, const char *small, const char *large, const char *desc, core_callback_t cb)
{
    if (self->option_count == HB_CORE_MAX_OPTIONS) {
        core_free(self);
    }

    int n = self->option_count++;
    core_option_t *option = &self->options[n];
    option->cb = cb;
    option->small = small;
    option->description = desc;
    option->required_arg = option->optional_arg = 0;
    option->large_with_arg = large;
    option->argname = malloc(strlen(large) + 1);
    assert(option->argname);
    option->large = malloc(strlen(large) + 1);
    assert(option->large);
    parse_argname(large, option->large, option->argname);

    if ('[' == option->argname[0]) option->optional_arg = 1;
    if ('<' == option->argname[0]) option->required_arg = 1;
}

/* Parse `argv` (internal).
 * Input arguments should be normalized first
 * see `normalize_args`. */
static void core_parse_args(core_t *self, int argc, char **argv)
{
    int literal = 0;
    int i, j;

    for (i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        for (j = 0; j < self->option_count; ++j) {
            core_option_t *option = &self->options[j];

            /* Match flag. */
            if (!strcmp(arg, option->small) || !strcmp(arg, option->large)) {
                self->arg = NULL;

                /* Required. */
                if (option->required_arg) {
                    arg = argv[++i];
                    if (!arg || '-' == arg[0]) {
                        fprintf(stderr, "%s %s argument required\n", option->large, option->argname);
                        core_free(self);
                        core_close(1);
                    }
                    self->arg = arg;
                }

                /* Optional. */
                if (option->optional_arg) {
                    if (argv[i + 1] && '-' != argv[i + 1][0]) {
                        self->arg = argv[++i];
                    }
                }

                /* Invoke callback. */
                option->cb(self);
                goto match;
            }
        }

        /* Search '--'. */
        if ('-' == arg[0] && '-' == arg[1] && 0 == arg[2]) {
            literal = 1;
            goto match;
        }

        /* Unrecognized. */
        if ('-' == arg[0] && !literal) {
            fprintf(stderr, "unrecognized flag %s\n", arg);
            core_free(self);
            core_close(1);
        }

        int n = self->argc++;
        if (n == HB_CORE_MAX_ARGS) {
            core_free(self);
        }
        self->argv[n] = (char *) arg;
match:
        ;
    }
}

/* Parse `argv` (public). */
void core_parse(core_t *self, int argc, char **argv)
{
    self->nargv = normalize_args(&argc, argv);
    core_parse_args(self, argc, self->nargv);
    self->argv[self->argc] = NULL;
}