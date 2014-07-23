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

#ifndef _HB_PIPE_H_
#define _HB_PIPE_H_

#include <sys/types.h>
#include <stdarg.h>

typedef char *pipe_t;

struct pipe_hdr_t {
    int len;
    int free;
    char buf[];
};

static inline size_t pipe_len(const pipe_t s)
{
    struct pipe_hdr_t *sh = (void*)(s-sizeof *sh);
    return sh->len;
}

static inline size_t pipe_avail(const pipe_t s)
{
    struct pipe_hdr_t *sh = (void*)(s-sizeof *sh);
    return sh->free;
}

pipe_t  pipe_newlen(const void *init, size_t initlen);
pipe_t  pipe_new(const char *init);
pipe_t  pipe_empty(void);
size_t  pipe_len(const pipe_t s);
pipe_t  pipe_dup(const pipe_t s);
void    pipe_free(pipe_t s);
size_t  pipe_avail(const pipe_t s);
pipe_t  pipe_growzero(pipe_t s, size_t len);
pipe_t  pipe_catlen(pipe_t s, const void *t, size_t len);
pipe_t  pipe_cat(pipe_t s, const char *t);
pipe_t  pipe_catpipe(pipe_t s, const pipe_t t);
pipe_t  pipe_cpylen(pipe_t s, const char *t, size_t len);
pipe_t  pipe_cpy(pipe_t s, const char *t);

pipe_t  pipe_catvprintf(pipe_t s, const char *fmt, va_list ap);
#ifdef __GNUC__
pipe_t  pipe_catprintf(pipe_t s, const char *fmt, ...)
__attribute__((format(printf, 2, 3)));
#else
pipe_t  pipe_catprintf(pipe_t s, const char *fmt, ...);
#endif

void    pipe_trim(pipe_t s, const char *cset);
void    pipe_range(pipe_t s, int start, int end);
void    pipe_updatelen(pipe_t s);
void    pipe_clear(pipe_t s);
int     pipe_cmp(const pipe_t s1, const pipe_t s2);
pipe_t *pipe_splitlen(const char *s, int len, const char *sep, int seplen, int *count);
void    pipe_freesplitres(pipe_t *tokens, int count);
void    pipe_tolower(pipe_t s);
void    pipe_toupper(pipe_t s);
pipe_t  pipe_fromlonglong(long long value);
pipe_t  pipe_catrepr(pipe_t s, const char *p, size_t len);
pipe_t *pipe_splitargs(const char *line, int *argc);
pipe_t  pipe_mapchars(pipe_t s, const char *from, const char *to, size_t setlen);
pipe_t  pipe_join(char **argv, int argc, char *sep, size_t seplen);
pipe_t  pipe_joinpipe(pipe_t *argv, int argc, const char *sep, size_t seplen);

/* Low level functions exposed to the user API */
pipe_t  pipe_MakeRoomFor(pipe_t s, size_t addlen);
void    pipe_IncrLen(pipe_t s, int incr);
pipe_t  pipe_RemoveFreeSpace(pipe_t s);
size_t  pipe_AllocSize(pipe_t s);

#endif