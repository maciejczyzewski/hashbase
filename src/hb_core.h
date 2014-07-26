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

#ifndef _HB_CORE_H_
#define _HB_CORE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

/*-----------------------------------------------------------------------------
 * HASHBASE config/macros
 *-------------------------------------------------------------------------- */

#define HB_VERSION          "0.0.1"

#define HB_OK               0
#define HB_ERR              -1

#define HB_LOG_OK           "\033[1;32m   ok >>\033[0m"
#define HB_LOG_ERR          "\033[1;31merror >>\033[0m"
#define HB_LOG_WRN          "\033[1;33m warn >>\033[0m"
#define HB_LOG_INF          " info >>"

#define HB_MAP_SIZE         512
#define HB_MAP_LENGTH       256

#define HB_NET_PORT         5555
#define HB_NET_BUFFER       512
#define HB_NET_BACKLOG      256

#define HB_CORE_LOCK        "/tmp/hashbase.pid"
#define HB_CORE_MAX_OPTIONS 32
#define HB_CORE_MAX_ARGS    32

#define HB_PIPE_PREALLOC    (1024*1024)

/*-----------------------------------------------------------------------------
 * HASHBASE server/client
 *-------------------------------------------------------------------------- */

struct server {
    /* options with no argument */

    int                     status;           /* memory  : last status code */

    int                     buffer;           /* network : packet lenght */
    int                     backlog;          /* network : tcp backlog */
    int                     port;             /* network : tcp listening port */
    int                     socket;           /* network : tcp socket */
    struct sockaddr_in      addr;             /* network : tcp addr */

    pid_t                   pid;              /* process : pid */
    char *                  lock;             /* process : lock */
    bool                    daemonize:1;      /* process : daemon */
    bool                    keepRunning:1;    /* process : status */
};

struct client {
    /* options with no argument */

    int                     socket;           /* network : tcp socket */
    struct sockaddr_in      addr;             /* network : tcp addr */
    int                     size;             /* network : tcp size */
};

/*-----------------------------------------------------------------------------
 * HASHBASE modules
 *-------------------------------------------------------------------------- */

#include <hb_args.h>
#include <hb_ascii.h>
#include <hb_map.h>
#include <hb_net.h>
#include <hb_pipe.h>
#include <hb_util.h>

/*-----------------------------------------------------------------------------
 * HASHBASE functions
 *-------------------------------------------------------------------------- */

void core_init(int, char * []);
void core_close(int);

#endif