/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* fndecls.c
   processes C files and extract public function declarations */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void *
xmalloc(unsigned size)
{
    void *t;

    t = malloc(size);
    if(!t)
        fprintf(stderr, "fatal error: virtual memory exhausted\n"), exit(1);

    return t;
}

void *
xrealloc(void *t, unsigned new_size)
{
    t = realloc(t, new_size);
    if(!t)
        fprintf(stderr, "fatal error: virtual memory exhausted\n"), exit(1);

    return t;
}

static char *line_buf;
static int line_buf_len;
static int line_buf_max_len;

static char *current_file_name;
static FILE *current_file_fp;

static inline void
line_buf_init()
{
    *line_buf = '\0';
    line_buf_len = 0;
}

static inline void
line_buf_grow(char ch)
{
    if(line_buf_len == line_buf_max_len)
        line_buf = (char *)xrealloc(line_buf, line_buf_max_len *= 4);

    line_buf[line_buf_len++] = ch;
    line_buf[line_buf_len] = '\0';
}

static inline char
next_char()
{
    char ch = fgetc(current_file_fp);
    line_buf_grow(ch);

    return ch;
}

static inline void
fndecl()
{
    char ch, *t;
    char public1[7];

    ch = next_char();
    if(!(isdigit(ch)
         || ch == '_'))
        return;

    while(next_char() != '(')
        ;

    *public1 = next_char();
    public1[1] = next_char();
    public1[2] = next_char();
    public1[3] = next_char();
    public1[4] = next_char();
    public1[5] = next_char();
    public1[6] = '\0';

    if(strcmp(public1, "PUBLIC"))
        return;

    /* read until matching close paren */
    while(next_char() != ')')
        ;

    /* and read to end of line (hey, it's what cliff's stuff does) */
    while(next_char() != '\n')
        ;

    /* remove internal definitions */
    t = line_buf;

    while(*t)
    {
        if(*t == 'I'
           && !strncmp(t, "INTERNAL", 8))
            return;
        t++;
    }

    /* valid function decl, output it */
    fprintf(stdout, "%s", line_buf);
}

static inline void
process_file()
{
    int ch;

    while(1)
    {
        ch = fgetc(current_file_fp);

        if(ch == EOF)
        {
            if(!feof(current_file_fp))
                fprintf(stderr, "read error `%s' on input file `%s'",
                        "(errno description here)", current_file_name);
            return;
        }
        else if(ch == 'P')
        {
            line_buf_init();
            line_buf_grow(ch);
            fndecl();
        }
        else if(ch == '\n')
            continue;

        /* this line does not contain a function declaration, read 'till
	 the next line */
        while(1)
        {
            ch = fgetc(current_file_fp);
            if(ch == '\n'
               /* and don't get stuck looping at eof */
               || ch == EOF)
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    int i;

    if(argc == 1)
        fprintf(stderr, "useage: %s files...", *argv), exit(1);

    line_buf_max_len = 1024;
    line_buf = (char *)xmalloc(line_buf_max_len);

    for(i = 1; i < argc; i++)
    {
        current_file_name = argv[i];
        current_file_fp = fopen(current_file_name, "r");
        if(!current_file_fp)
        {
            fprintf(stderr, "warning: unable to open input file `%s'\n", argv[i]);
            continue;
        }
        process_file();
    }

    exit(EXIT_SUCCESS);
}
