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
#include <sys/types.h>
#include <signal.h>

#include <hb_core.h>

extern struct server server;
extern struct client client;

static void do_daemonize();
static void do_stop();

static void print_help(args_context_t);
static void print_version(args_context_t);

static void do_daemonize()
{
    server.daemonize = true;
    server.pid = fork();

    if (server.pid < 0) {
        fprintf(stdout, "hb: %s fork failed\n", HB_LOG_ERR);
        core_close(1);
    }

    if (server.pid > 0) {
        FILE *f = fopen(server.lock, "w+");

        if (f != NULL) {
            fprintf(f, "%d", server.pid);
            fclose(f);
        }

        fprintf(stdout, "hb: %s daemon process running [fd: %d]...\n", HB_LOG_OK, server.pid);
        exit(0);
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

static void do_stop()
{
    pid_t pid;

    FILE *f = fopen(server.lock, "r");

    if (f != NULL) {
        fscanf(f, "%d", &pid);
        fclose(f);

        kill(pid, SIGKILL);
        kill(pid, SIGTERM);

        remove(server.lock);
    }

    core_close(2);
}

static void print_help(args_context_t ctx)
{
    char buffer[2048];
    printf("\nUsage: hashbase [options]\n\n");
    printf("%s\n", args_create_help_string(&ctx, buffer, sizeof(buffer)));
    core_close(0);
}

static void print_version(args_context_t ctx)
{
    printf("%s\n", HB_VERSION);
    core_close(0);
}

static const args_option_t option_list[] = {
    { "daemonize", 'd', ARGS_OPTION_TYPE_NO_ARG,   0x0, 'd', "run hashbase as a daemon",                             0x0 },
    { "stop",      's', ARGS_OPTION_TYPE_NO_ARG,   0x0, 's', "close running daemon",                                 0x0 },
    { "port",      'p', ARGS_OPTION_TYPE_REQUIRED, 0x0, 'p', "set the tcp port to listen on",                   "NUMBER" },
    { "help",      'h', ARGS_OPTION_TYPE_NO_ARG,   0x0, 'h', "show hashbase version, usage, options, and exit",      0x0 },
    { "version",   'v', ARGS_OPTION_TYPE_NO_ARG,   0x0, 'v', "show version and exit",                                0x0 },
    ARGS_OPTIONS_END
};

void core_init(int argc, char *argv[])
{
    args_context_t ctx;
    args_create_context(&ctx, argc, argv, option_list);

    int opt;

    while ( ( opt = args_next( &ctx ) ) != -1 ) {
        switch ( opt ) {
        /* Warnings */
        case '+':
            fprintf(stdout, "hb: %s got argument without flag [%s]\n", HB_LOG_WRN, ctx.current_opt_arg);
            break;
        case '?':
            fprintf(stdout, "hb: %s unknown flag [%s]\n", HB_LOG_WRN, ctx.current_opt_arg);
            break;
        case '!':
            fprintf(stdout, "hb: %s invalid use of flag [%s]\n", HB_LOG_WRN, ctx.current_opt_arg);
            break;
        /* Options */
        case 'd':
            do_daemonize();
            break;
        case 's':
            do_stop();
            break;
        case 'p':
            server.port = atoi(ctx.current_opt_arg);
            break;
        /* Help & Version */
        case 'h':
            print_help(ctx);
            break;
        case 'v':
            print_version(ctx);
            break;
        default:
            break;
        }
    }
}

void core_close(int code)
{
    server.keepRunning = false;

    close(client.socket);
    close(server.socket);

    switch (code) {
    /* Signal close */
    case 2:
        fprintf(stdout, "hb: %s closing hashbase...\n", HB_LOG_INF);
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
