/*
 * ASCII                                An implementation of hashbase commands.
 *
 * Version:                                    @(#)ascii.c    0.0.1    09/07/14
 * Authors:             Maciej A. Czyzewski, <maciejanthonyczyzewski@gmail.com>
 *
 */

#include <hb_core.h>

extern map_t database;

pipe_t ascii_inf(pipe_t *tokens)
{
	pipe_t buffer = pipe_empty();

    buffer = pipe_new("hashbase ");
    buffer = pipe_cat(buffer, HB_VERSION);
    buffer = pipe_cat(buffer, " (c) 2014 Maciej A. Czyzewski");
    
    return buffer;
}

pipe_t ascii_set(pipe_t *tokens)
{
	pipe_t buffer = pipe_empty();

    map_put(&database, tokens[1], tokens[2]);
    buffer = pipe_fromlonglong(HB_OK);

    return buffer;
}

pipe_t ascii_get(pipe_t *tokens)
{
	pipe_t buffer = pipe_empty();

    if ((map_get(&database, tokens[1], (void**)(&buffer))) != HB_ERR) {
        buffer = pipe_new(buffer);
    }else{
    	buffer = pipe_fromlonglong(HB_ERR);
    }

    return buffer;
}

pipe_t ascii_del(pipe_t *tokens)
{
	pipe_t buffer = pipe_empty();

    map_remove(&database, tokens[1]);
    buffer = pipe_fromlonglong(HB_OK);

    return buffer;
}

pipe_t ascii_len(pipe_t *tokens)
{
	pipe_t buffer = pipe_empty();

	buffer = pipe_fromlonglong(map_length(&database));

    return buffer;
}

pipe_t ascii_clr(pipe_t *tokens)
{
	pipe_t buffer = pipe_empty();

    map_free(&database);
    buffer = pipe_fromlonglong(HB_OK);

    return buffer;
}