/*
 * hashbase - https://github.com/MaciejCzyzewski/hashbase
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Maciej A. Czyzewski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author: Maciej A. Czyzewski <maciejanthonyczyzewski@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <hb_core.h>

struct server server;
struct client client;

map_t database;

/* ================================ Globals ================================ */

int main(int argc , char *argv[])
{
    signal(SIGINT, core_close);

    server.pid        = getpid();
    server.lock       = HB_CORE_LOCK;

    server.port       = HB_NET_PORT;
    server.backlog    = HB_NET_BACKLOG;
    server.buffer     = HB_NET_BUFFER;

    server.daemonize  = false;

    client.size = sizeof(struct sockaddr_in);

    core_init(argc, argv);

    char *ascii_logo =
        "                                                           \n"
        "    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    \n"
        "    XXXXXXXXXXXXXXXXXXXXXXXXXX  _                     X    \n"
        "    XX|`|XXXXXXXXXXXXX|`|XXXXX | |__   __ _ ___  ___  X    \n"
        "    XX| '_ \\X/`_``/`__| '_`\\XX | '_ \\ / _` / __|/ _ \\ X\n"
        "    XX| |X| | (X| \\__ \\ |X| XX | |_) | (_| \\__ \\  __/ X\n"
        "    XX|_|X|_|\\__,_|___/_|X|_XX |_.__/ \\__,_|___/\\___| X \n"
        "    XXXXXXXXXXXXXXXXXXXXXXXXXX              ver %s X       \n"
        "    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    \n"
        "    port: %d, pid: %ld                                     \n\n";

    printf(ascii_logo, HB_VERSION, server.port, server.pid);

    server.status = net_init();
    if (server.status == HB_ERR) core_close(1);

    server.status = map_init();
    if (server.status == HB_ERR) core_close(1);

    fprintf(stdout, "hb: %s waiting for incoming connections...\n", HB_LOG_INF);

    struct ascii_t commands [] = {
        { "inf", ascii_inf },
        { "set", ascii_set },
        { "get", ascii_get },
        { "del", ascii_del },
        { "len", ascii_len },
        { "clr", ascii_clr },
    };

    server.commands = commands;

    net_loop();

    return 0;
}