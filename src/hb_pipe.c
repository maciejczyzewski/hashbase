/*
 * PIPE                                               A simple thread-safe FIFO
 *
 * Version:                                     @(#)pipe.c    0.0.1    09/07/14
 * Authors:                               Clark Gaebel, <cg.wowus.cg@gmail.com>
 *                                     Salvatore Sanfilippo <antirez@gmail.com>
 *                      Maciej A. Czyzewski, <maciejanthonyczyzewski@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <hb_core.h>

/* Create a new pipe_t string with the content specified by the 'init' pointer
 * and 'initlen'.
 * If NULL is used for 'init' the string is initialized with zero bytes.
 *
 * The string is always null-termined (all the pipe_t strings are, always) so
 * even if you create an pipe_t string with:
 *
 * mystring = pipe_newlen("abc",3");
 *
 * You can print the string with printf() as there is an implicit \0 at the
 * end of the string. However the string is binary safe and can contain
 * \0 characters in the middle, as the length is stored in the pipe_t header. */
pipe_t pipe_newlen(const void *init, size_t initlen)
{
    struct pipe_hdr_t *sh;

    if (init) {
        sh = malloc(sizeof *sh+initlen+1);
    } else {
        sh = calloc(sizeof *sh+initlen+1,1);
    }
    if (sh == NULL) return NULL;
    sh->len = initlen;
    sh->free = 0;
    if (initlen && init)
        memcpy(sh->buf, init, initlen);
    sh->buf[initlen] = '\0';
    return (char*)sh->buf;
}

/* Create an empty (zero length) pipe_t string. Even in this case the string
 * always has an implicit null term. */
pipe_t pipe_empty(void)
{
    return pipe_newlen("",0);
}

/* Create a new pipe_t string starting from a null termined C string. */
pipe_t pipe_new(const char *init)
{
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return pipe_newlen(init, initlen);
}

size_t pipe_len(const pipe_t s)
{
    struct pipe_hdr_t *sh = (void*)(s-sizeof *sh);
    return sh->len;
}

/* Duplicate an pipe_t string. */
pipe_t pipe_dup(const pipe_t s)
{
    return pipe_newlen(s, pipe_len(s));
}

/* Free an pipe_t string. No operation is performed if 's' is NULL. */
void pipe_free(pipe_t s)
{
    if (s == NULL) return;
    free(s-sizeof(struct pipe_hdr_t));
}

size_t pipe_avail(const pipe_t s)
{
    struct pipe_hdr_t *sh = (void*)(s-sizeof *sh);
    return sh->free;
}

/* Set the pipe_t string length to the length as obtained with strlen(), so
 * considering as content only up to the first null term character.
 *
 * This function is useful when the pipe_t string is hacked manually in some
 * way, like in the following example:
 *
 * s = pipe_new("foobar");
 * s[2] = '\0';
 * pipe_updatelen(s);
 * printf("%d\n", pipe_len(s));
 *
 * The output will be "2", but if we comment out the call to pipe_updatelen()
 * the output will be "6" as the string was modified but the logical length
 * remains 6 bytes. */
void pipe_updatelen(pipe_t s)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);;
    int reallen = strlen(s);
    sh->free += (sh->len-reallen);
    sh->len = reallen;
}

/* Modify an pipe_t string on-place to make it empty (zero length).
 * However all the existing buffer is not discarded but set as free space
 * so that next append operations will not require allocations up to the
 * number of bytes previously available. */
void pipe_clear(pipe_t s)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);;
    sh->free += sh->len;
    sh->len = 0;
    sh->buf[0] = '\0';
}

/* Enlarge the free space at the end of the pipe_t string so that the caller
 * is sure that after calling this function can overwrite up to addlen
 * bytes after the end of the string, plus one more byte for nul term.
 *
 * Note: this does not change the *length* of the pipe_t string as returned
 * by pipelen(), but only the free buffer space we have. */
pipe_t pipe_MakeRoomFor(pipe_t s, size_t addlen)
{
    struct pipe_hdr_t *sh, *newsh;
    size_t free = pipe_avail(s);
    size_t len, newlen;

    if (free >= addlen) return s;
    len = pipe_len(s);
    sh = (void*) (s-sizeof *sh);;
    newlen = (len+addlen);
    if (newlen < HB_PIPE_PREALLOC)
        newlen *= 2;
    else
        newlen += HB_PIPE_PREALLOC;
    newsh = realloc(sh, sizeof *newsh+newlen+1);
    if (newsh == NULL) return NULL;

    newsh->free = newlen - len;
    return newsh->buf;
}

/* Reallocate the pipe_t string so that it has no free space at the end. The
 * contained string remains not altered, but next concatenation operations
 * will require a reallocation.
 *
 * After the call, the passed pipe_t string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
pipe_t pipe_RemoveFreeSpace(pipe_t s)
{
    struct pipe_hdr_t *sh;

    sh = (void*) (s-sizeof *sh);;
    sh = realloc(sh, sizeof *sh+sh->len+1);
    sh->free = 0;
    return sh->buf;
}

/* Return the total size of the allocation of the specifed pipe_t string,
 * including:
 * 1) The pipe_t header before the pointer.
 * 2) The string.
 * 3) The free buffer at the end if any.
 * 4) The implicit null term.
 */
size_t pipe_AllocSize(pipe_t s)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);;

    return sizeof(*sh)+sh->len+sh->free+1;
}

/* Increment the pipe_t length and decrements the left free space at the
 * end of the string according to 'incr'. Also set the null term
 * in the new end of the string.
 *
 * This function is used in order to fix the string length after the
 * user calls pipe_MakeRoomFor(), writes something after the end of
 * the current string, and finally needs to set the new length.
 *
 * Note: it is possible to use a negative increment in order to
 * right-trim the string.
 *
 * Usage example:
 *
 * Using pipe_IncrLen() and pipe_MakeRoomFor() it is possible to mount the
 * following schema, to cat bytes coming from the kernel to the end of an
 * pipe_t string without copying into an intermediate buffer:
 *
 * oldlen = pipe_len(s);
 * s = pipe_MakeRoomFor(s, BUFFER_SIZE);
 * nread = read(fd, s+oldlen, BUFFER_SIZE);
 * ... check for nread <= 0 and handle it ...
 * pipe_IncrLen(s, nread);
 */
void pipe_IncrLen(pipe_t s, int incr)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);;

    assert(sh->free >= incr);
    sh->len += incr;
    sh->free -= incr;
    assert(sh->free >= 0);
    s[sh->len] = '\0';
}

/* Grow the pipe_t to have the specified length. Bytes that were not part of
 * the original length of the pipe_t will be set to zero.
 *
 * if the specified length is smaller than the current length, no operation
 * is performed. */
pipe_t pipe_growzero(pipe_t s, size_t len)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);
    size_t totlen, curlen = sh->len;

    if (len <= curlen) return s;
    s = pipe_MakeRoomFor(s,len-curlen);
    if (s == NULL) return NULL;

    /* Make sure added region doesn't contain garbage */
    sh = (void*)(s-sizeof *sh);
    memset(s+curlen,0,(len-curlen+1)); /* also set trailing \0 byte */
    totlen = sh->len+sh->free;
    sh->len = len;
    sh->free = totlen-sh->len;
    return s;
}

/* Append the specified binary-safe string pointed by 't' of 'len' bytes to the
 * end of the specified pipe_t string 's'.
 *
 * After the call, the passed pipe_t string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
pipe_t pipe_catlen(pipe_t s, const void *t, size_t len)
{
    struct pipe_hdr_t *sh;
    size_t curlen = pipe_len(s);

    s = pipe_MakeRoomFor(s,len);
    if (s == NULL) return NULL;
    sh = (void*) (s-sizeof *sh);;
    memcpy(s+curlen, t, len);
    sh->len = curlen+len;
    sh->free = sh->free-len;
    s[curlen+len] = '\0';
    return s;
}

/* Append the specified null termianted C string to the pipe_t string 's'.
 *
 * After the call, the passed pipe_t string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
pipe_t pipe_cat(pipe_t s, const char *t)
{
    return pipe_catlen(s, t, strlen(t));
}

/* Append the specified pipe_t 't' to the existing pipe_t 's'.
 *
 * After the call, the modified pipe_t string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
pipe_t pipe_catpipe(pipe_t s, const pipe_t t)
{
    return pipe_catlen(s, t, pipe_len(t));
}

/* Destructively modify the pipe_t string 's' to hold the specified binary
 * safe string pointed by 't' of length 'len' bytes. */
pipe_t pipe_cpylen(pipe_t s, const char *t, size_t len)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);;
    size_t totlen = sh->free+sh->len;

    if (totlen < len) {
        s = pipe_MakeRoomFor(s,len-sh->len);
        if (s == NULL) return NULL;
        sh = (void*) (s-sizeof *sh);;
        totlen = sh->free+sh->len;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    sh->len = len;
    sh->free = totlen-len;
    return s;
}

/* Like pipe_cpylen() but 't' must be a null-termined string so that the length
 * of the string is obtained with strlen(). */
pipe_t pipe_cpy(pipe_t s, const char *t)
{
    return pipe_cpylen(s, t, strlen(t));
}

/* Like pipe_catpritf() but gets va_list instead of being variadic. */
pipe_t pipe_catvprintf(pipe_t s, const char *fmt, va_list ap)
{
    va_list cpy;
    char *buf, *t;
    size_t buflen = 16;

    while(1) {
        buf = malloc(buflen);
        if (buf == NULL) return NULL;
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        vsnprintf(buf, buflen, fmt, cpy);
        if (buf[buflen-2] != '\0') {
            free(buf);
            buflen *= 2;
            continue;
        }
        break;
    }
    t = pipe_cat(s, buf);
    free(buf);
    return t;
}

/* Append to the pipe_t string 's' a string obtained using printf-alike format
 * specifier.
 *
 * After the call, the modified pipe_t string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = pipe_empty("Sum is: ");
 * s = pipe_catprintf(s,"%d+%d = %d",a,b,a+b).
 *
 * Often you need to create a string from scratch with the printf-alike
 * format. When this is the need, just use pipe_empty() as the target string:
 *
 * s = pipe_catprintf(pipe_empty(), "... your format ...", args);
 */
pipe_t pipe_catprintf(pipe_t s, const char *fmt, ...)
{
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = pipe_catvprintf(s,fmt,ap);
    va_end(ap);
    return t;
}

/* Remove the part of the string from left and from right composed just of
 * contiguous characters found in 'cset', that is a null terminted C string.
 *
 * After the call, the modified pipe_t string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = pipe_new("AA...AA.a.aa.aHelloWorld     :::");
 * s = pipe_trim(s,"A. :");
 * printf("%s\n", s);
 *
 * Output will be just "Hello World".
 */
void pipe_trim(pipe_t s, const char *cset)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);;
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+pipe_len(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (sh->buf != sp) memmove(sh->buf, sp, len);
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
}

/* Turn the string into a smaller (or equal) string containing only the
 * substring specified by the 'start' and 'end' indexes.
 *
 * start and end can be negative, where -1 means the last character of the
 * string, -2 the penultimate character, and so forth.
 *
 * The interval is inclusive, so the start and end characters will be part
 * of the resulting string.
 *
 * The string is modified in-place.
 *
 * Example:
 *
 * s = pipe_new("Hello World");
 * pipe_range(s,1,-1); => "ello World"
 */
void pipe_range(pipe_t s, int start, int end)
{
    struct pipe_hdr_t *sh = (void*) (s-sizeof *sh);;
    size_t newlen, len = pipe_len(s);

    if (len == 0) return;
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(sh->buf, sh->buf+start, newlen);
    sh->buf[newlen] = 0;
    sh->free = sh->free+(sh->len-newlen);
    sh->len = newlen;
}

/* Apply tolower() to every character of the pipe_t string 's'. */
void pipe_tolower(pipe_t s)
{
    int len = pipe_len(s), j;

    for (j = 0; j < len; j++) s[j] = tolower(s[j]);
}

/* Apply toupper() to every character of the pipe_t string 's'. */
void pipe_toupper(pipe_t s)
{
    int len = pipe_len(s), j;

    for (j = 0; j < len; j++) s[j] = toupper(s[j]);
}

/* Compare two pipe_t strings s1 and s2 with memcmp().
 *
 * Return value:
 *
 *     1 if s1 > s2.
 *    -1 if s1 < s2.
 *     0 if s1 and s2 are exactly the same binary string.
 *
 * If two strings share exactly the same prefix, but one of the two has
 * additional characters, the longer string is considered to be greater than
 * the smaller one. */
int pipe_cmp(const pipe_t s1, const pipe_t s2)
{
    size_t l1, l2, minlen;
    int cmp;

    l1 = pipe_len(s1);
    l2 = pipe_len(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1,s2,minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}

/* Split 's' with separator in 'sep'. An array
 * of pipe_t strings is returned. *count will be set
 * by reference to the number of tokens returned.
 *
 * On out of memory, zero length string, zero length
 * separator, NULL is returned.
 *
 * Note that 'sep' is able to split a string using
 * a multi-character separator. For example
 * pipe_split("foo_-_bar","_-_"); will return two
 * elements "foo" and "bar".
 *
 * This version of the function is binary-safe but
 * requires length arguments. pipe_split() is just the
 * same function but for zero-terminated strings.
 */
pipe_t *pipe_splitlen(const char *s, int len, const char *sep, int seplen, int *count)
{
    int elements = 0, slots = 5, start = 0, j;
    pipe_t *tokens;

    if (seplen < 1 || len < 0) return NULL;

    tokens = malloc(sizeof(pipe)*slots);
    if (tokens == NULL) return NULL;

    if (len == 0) {
        *count = 0;
        return tokens;
    }
    for (j = 0; j < (len-(seplen-1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {
            pipe_t *newtokens;

            slots *= 2;
            newtokens = realloc(tokens,sizeof(pipe)*slots);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = pipe_newlen(s+start,j-start);
            if (tokens[elements] == NULL) goto cleanup;
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = pipe_newlen(s+start,len-start);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;

cleanup: {
        int i;
        for (i = 0; i < elements; i++) pipe_free(tokens[i]);
        free(tokens);
        *count = 0;
        return NULL;
    }
}

/* Free the result returned by pipe_splitlen(), or do nothing if 'tokens' is NULL. */
void pipe_freesplitres(pipe_t *tokens, int count)
{
    if (!tokens) return;
    while(count--)
        pipe_free(tokens[count]);
    free(tokens);
}

/* Create an pipe_t string from a long long value. It is much faster than:
 *
 * pipe_catprintf(pipeempty(),"%lld\n", value);
 */
pipe_t pipe_fromlonglong(long long value)
{
    char buf[32], *p;
    unsigned long long v;

    v = (value < 0) ? -value : value;
    p = buf+31; /* point to the last character */
    do {
        *p-- = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p-- = '-';
    p++;
    return pipe_newlen(p,32-(p-buf));
}

/* Append to the pipe_t string "s" an escaped string representation where
 * all the non-printable characters (tested with isprint()) are turned into
 * escapes in the form "\n\r\a...." or "\x<hex-number>".
 *
 * After the call, the modified pipe_t string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
pipe_t pipe_catrepr(pipe_t s, const char *p, size_t len)
{
    s = pipe_catlen(s,"\"",1);
    while(len--) {
        switch(*p) {
        case '\\':
        case '"':
            s = pipe_catprintf(s,"\\%c",*p);
            break;
        case '\n':
            s = pipe_catlen(s,"\\n",2);
            break;
        case '\r':
            s = pipe_catlen(s,"\\r",2);
            break;
        case '\t':
            s = pipe_catlen(s,"\\t",2);
            break;
        case '\a':
            s = pipe_catlen(s,"\\a",2);
            break;
        case '\b':
            s = pipe_catlen(s,"\\b",2);
            break;
        default:
            if (isprint(*p))
                s = pipe_catprintf(s,"%c",*p);
            else
                s = pipe_catprintf(s,"\\x%02x",(unsigned char)*p);
            break;
        }
        p++;
    }
    return pipe_catlen(s,"\"",1);
}

/* Helper function for pipesplitargs() that returns non zero if 'c'
 * is a valid hex digit. */
static int is_hex_digit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Helper function for pipesplitargs() that converts a hex digit into an
 * integer from 0 to 15 */
static int hex_digit_to_int(char c)
{
    switch(c) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    default:
        return 0;
    }
}

/* Split a line into arguments, where every argument can be in the
 * following programming-language REPL-alike form:
 *
 * foo bar "newline are supported\n" and "\xff\x00otherstuff"
 *
 * The number of arguments is stored into *argc, and an array
 * of pipe_t is returned.
 *
 * The caller should free the resulting array of pipe_t strings with
 * pipe_freesplitres().
 *
 * Note that pipe_catrepr() is able to convert back a string into
 * a quoted string in the same format pipe_splitargs() is able to parse.
 *
 * The function returns the allocated tokens on success, even when the
 * input string is empty, or NULL if the input contains unbalanced
 * quotes or closed quotes followed by non space characters
 * as in: "foo"bar or "foo'
 */
pipe_t *pipe_splitargs(const char *line, int *argc)
{
    const char *p = line;
    char *current = NULL;
    char **vector = NULL;

    *argc = 0;
    while(1) {
        /* skip blanks */
        while(*p && isspace(*p)) p++;
        if (*p) {
            /* get a token */
            int inq=0;  /* set to 1 if we are in "quotes" */
            int insq=0; /* set to 1 if we are in 'single quotes' */
            int done=0;

            if (current == NULL) current = pipe_empty();
            while(!done) {
                if (inq) {
                    if (*p == '\\' && *(p+1) == 'x' &&
                        is_hex_digit(*(p+2)) &&
                        is_hex_digit(*(p+3))) {
                        unsigned char byte;

                        byte = (hex_digit_to_int(*(p+2))*16)+
                               hex_digit_to_int(*(p+3));
                        current = pipe_catlen(current,(char*)&byte,1);
                        p += 3;
                    } else if (*p == '\\' && *(p+1)) {
                        char c;

                        p++;
                        switch(*p) {
                        case 'n':
                            c = '\n';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        case 'b':
                            c = '\b';
                            break;
                        case 'a':
                            c = '\a';
                            break;
                        default:
                            c = *p;
                            break;
                        }
                        current = pipe_catlen(current,&c,1);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = pipe_catlen(current,p,1);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p+1) == '\'') {
                        p++;
                        current = pipe_catlen(current,"'",1);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = pipe_catlen(current,p,1);
                    }
                } else {
                    switch(*p) {
                    case ' ':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\0':
                        done=1;
                        break;
                    case '"':
                        inq=1;
                        break;
                    case '\'':
                        insq=1;
                        break;
                    default:
                        current = pipe_catlen(current,p,1);
                        break;
                    }
                }
                if (*p) p++;
            }
            /* add the token to the vector */
            vector = realloc(vector,((*argc)+1)*sizeof(char*));
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        } else {
            /* Even on empty input string return something not NULL. */
            if (vector == NULL) vector = malloc(sizeof(void*));
            return vector;
        }
    }

err:
    while((*argc)--)
        pipe_free(vector[*argc]);
    free(vector);
    if (current) pipe_free(current);
    *argc = 0;
    return NULL;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: pipe_mapchars(mystring, "ho", "01", 2)
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the pipe_t string pointer, that is always the same
 * as the input pointer since no resize is needed. */
pipe_t pipe_mapchars(pipe_t s, const char *from, const char *to, size_t setlen)
{
    size_t j, i, l = pipe_len(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < setlen; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}

/* Join an array of C strings using the specified separator (also a C string).
 * Returns the result as an pipe_t string. */
pipe_t pipe_join(char **argv, int argc, char *sep, size_t seplen)
{
    pipe_t join = pipe_empty();
    int j;

    for (j = 0; j < argc; j++) {
        join = pipe_cat(join, argv[j]);
        if (j != argc-1) join = pipe_catlen(join,sep,seplen);
    }
    return join;
}

/* Like pipejoin, but joins an array of pipe strings. */
pipe_t pipe_joinpipe(pipe_t *argv, int argc, const char *sep, size_t seplen)
{
    pipe_t join = pipe_empty();
    int j;

    for (j = 0; j < argc; j++) {
        join = pipe_catpipe(join, argv[j]);
        if (j != argc-1) join = pipe_catlen(join,sep,seplen);
    }
    return join;
}