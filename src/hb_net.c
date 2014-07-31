/*
 * NET                An implementation of the TCP/UDP network access protocol.
 *
 * Version:                                      @(#)net.c    0.0.1    09/07/14
 * Authors:             Maciej A. Czyzewski, <maciejanthonyczyzewski@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <hb_core.h>

extern struct server server;
extern struct client client;

int net_init(void)
{
    if ((server.socket = socket(AF_INET, SOCK_STREAM, 0)) == HB_ERR) {
        fprintf(stdout, "hb: %s could not create socket\n", HB_LOG_ERR);
        return HB_ERR;
    }

    server.addr.sin_family = AF_INET;
    server.addr.sin_addr.s_addr = INADDR_ANY;
    server.addr.sin_port = htons(server.port);

    if (bind(server.socket, (struct sockaddr *)&server.addr, sizeof(server.addr)) < HB_OK) {
        fprintf(stdout, "hb: %s port is already in use\n", HB_LOG_ERR);
        return HB_ERR;
    }

    listen(server.socket, server.backlog);

    return HB_OK;
}

int net_loop(void)
{
    pthread_t thread_id;

    while ((client.socket = accept(server.socket, (struct sockaddr *)&client.addr, (socklen_t*)&client.size)) || server.keepRunning) {
        if (client.socket < HB_OK) {
            fprintf(stdout, "hb: %s connection failed\n", HB_LOG_ERR);
            return HB_ERR;
        }

        fprintf(stdout, "hb: %s connection accepted [fd: %d]\n", HB_LOG_OK, client.socket);

        if (pthread_create(&thread_id, NULL, net_handler, (void*) &client.socket) < HB_OK) {
            fprintf(stdout, "hb: %s could not create thread\n", HB_LOG_ERR);
            return HB_ERR;
        }
    }

    return HB_OK;
}

void *net_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int read_size = 0;

    char assocc[HB_NET_BUFFER];
    pipe_t buffer = pipe_empty();

    while ((read_size = recv(sock, assocc, HB_NET_BUFFER, 0)) > 0) {
        pipe_t packet = pipe_newlen(assocc, read_size);

        buffer = pipe_catpipe(buffer, packet);
        packet = pipe_empty();

        if (pipe_len(buffer) > 1) {
            if (buffer[(pipe_len(buffer)-1)] == '\n' && buffer[(pipe_len(buffer)-2)] == '\r') {
                pipe_trim(buffer, "\r\n");
                packet = net_command(buffer);
                packet = pipe_cat(packet, "\r\n");
                server.status = write(sock, packet, (int) pipe_len(packet));

                buffer = pipe_empty();
                packet = pipe_empty();
            }
        }

        pipe_free(packet);
    }

    switch (read_size) {
        case HB_OK:
            fprintf(stdout, "hb: %s client disconnected [fd: %d]\n", HB_LOG_OK, sock);
            fflush(stdout);
            break;
        case HB_ERR:
            fprintf(stdout, "hb: %s client receive failed [fd: %d]\n", HB_LOG_ERR, sock);
            break;
    }

    return HB_OK;
}

void *net_command(void *buffer)
{
    pipe_t *tokens;
    int count, i;

    tokens = pipe_splitargs(buffer, &count);

    for (i = 0 ;; i++) {
        if ( server.commands[i].name == NULL ) {
            break;
        } else if (!strcmp(server.commands[i].name, tokens[0]) && server.commands[i].func) {
            return buffer = server.commands[i].func(tokens);
        }
    }

    return buffer = pipe_fromlonglong(HB_ERR);
}
