/*
 * Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/*
 * Yahoo -- yet another configuration file thingy.  Who needs YACC when
 * you expect your end user to be familiar with DOS .ini files?
 */

#include "rsys/common.h"
#include <ctype.h>
#include <string>

#include "rsys/ini.h"

using namespace Executor;

typedef struct heading_link_str
{
    struct heading_link_str *next;
    heading_t heading;
    pair_link_t *pairs;
} heading_link_t;

static heading_link_t *headings;

heading_t
Executor::new_heading(unsigned char *start, int len)
{
    heading_t retval;
    heading_link_t *linkp;

    linkp = new heading_link_t;
    if(!linkp)
        retval = "";
    else
    {
        char *p = (char *)alloca(len + 1);
        if(!p)
        {
            retval = "";
            delete linkp;
        }
        else
        {
            strncpy(p, (char *)start, len);
            p[len] = 0;
            linkp->heading = p;
            linkp->next = headings;
            linkp->pairs = 0;
            headings = linkp;
            retval = linkp->heading;
        }
    }

    return retval;
}

static heading_link_t *
find_heading(heading_t heading)
{
    static heading_link_t *cachep;
    heading_link_t *retval;

    if(cachep && cachep->heading == heading)
        retval = cachep;
    else
    {
        for(retval = headings;
            retval && retval->heading != heading;
            retval = retval->next)
            ;
        if(retval)
            cachep = retval;
    }
    return retval;
}

void
Executor::new_key_value_pair(heading_t heading, unsigned char *keystart, int keylen,
                             unsigned char *valuestart, int valuelen)
{
    pair_link_t *pairp = new pair_link_t;
    pairp->key = std::string(keystart, keystart + keylen);
    pairp->value = std::string(valuestart, valuestart + valuelen);

    heading_link_t *headingp = find_heading(heading);
    pairp->next = 0;
    if(!headingp)
    {
        warning_unexpected("couldn't find %s", heading.c_str());
        delete pairp;
    }
    else
    {
        pair_link_t **pairpp;

        for(pairpp = &headingp->pairs;
            *pairpp;
            pairpp = &(*pairpp)->next)
            ;
        *pairpp = pairp;
    }
}

#if 0
/* This routine is currently unsafe to use because it might result in
   dangling pointers */
void
discard_all_inis (void)
{
  heading_link_t *headerp, *nextheaderp;

  for (headerp = headings; headerp; headerp = nextheaderp)
    {
      nextheaderp = headerp->next;
      {
	pair_link_t *pairp, *nextpairp;

	for (pairp = headerp->pairs; pairp; pairp = nextpairp)
	  {
	    nextpairp = pairp->next;
	    free (pairp->key);
	    free (pairp->value);
	    free (pairp);
	  }
      }
      free (headerp);
    }
}
#endif

bool
Executor::read_ini_file(const char *filename)
{
    using namespace std;
    FILE *fp;
    bool retval;

    fp = fopen(filename, "r");
    if(fp)
    {
        string heading = "";
        int line = 0;

        while(!feof(fp))
        {
            unsigned char buf[1024];
            unsigned char *p;

            ++line;
            if(!fgets((char *)buf, sizeof buf, fp))
                buf[0] = 0;

            for(p = buf; *p && isspace(*p); ++p)
                ;
            switch(*p)
            {
                case '[':
                {
                    unsigned char *ep;

                    ++p;
                    ep = (unsigned char *)strrchr((char *)p, ']');
                    if(*ep)
                        heading = new_heading(p, ep - p);
                    else
                        warning_unexpected("missing ] on line %d", line);
                }
                break;
                case '#':
                case 0:
                    break;
                default:
                {
                    unsigned char *eq;

                    eq = (unsigned char *)strchr((char *)p, '=');
                    if(!eq)
                        warning_unexpected("missing = on line %d", line);
                    else
                    {
                        unsigned char *ep;

                        ep = p + strlen((char *)p);
                        while(isspace(*(ep - 1)))
                            --ep;
                        new_key_value_pair(heading, p, eq - p, eq + 1, ep - eq - 1);
                    }
                }
                break;
            }
        }
    }
    retval = !!fp;
    return retval;
}

pair_link_t *
Executor::get_pair_link_n(heading_t heading, int n)
{
    pair_link_t *retval;
    heading_link_t *headingp;

    retval = NULL;
    headingp = find_heading(heading);
    if(headingp)
    {
        retval = headingp->pairs;
        while(retval && n > 0)
        {
            retval = retval->next;
            --n;
        }
    }

    return retval;
}

FILE *
Executor::open_ini_file_for_writing(const char *filename)
{
    FILE *retval;

    retval = fopen(filename, "w");
    return retval;
}

bool
Executor::add_heading_to_file(FILE *fp, heading_t heading)
{
    bool retval;

    retval = fprintf(fp, "[%s]\n", heading.c_str()) > 0;
    return retval;
}

bool
Executor::add_key_value_to_file(FILE *fp, ini_key_t key, value_t value)
{
    bool retval;

    retval = fprintf(fp, "%s=%s\n", key.c_str(), value.c_str()) > 0;
    return retval;
}

bool
Executor::close_ini_file(FILE *fp)
{
    bool retval;

    retval = fclose(fp) == 0;
    return retval;
}

value_t
Executor::find_key(heading_t heading, ini_key_t key)
{
    value_t retval;
    heading_link_t *headingp;

    retval = "";
    if(heading != "" && key != "")
    {
        headingp = find_heading(heading);
        if(headingp)
        {
            pair_link_t *pairp;

            for(pairp = headingp->pairs;
                pairp && pairp->key != key;
                pairp = pairp->next)
                ;
            if(pairp)
                retval = pairp->value;
        }
    }
    return retval;
}
