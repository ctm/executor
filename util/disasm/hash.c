#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static inline unsigned
hash (const char *s)
{
  unsigned n;

  for (n = 0; *s; s++)
    n = (n * 33) + *s;

  return n % NUM_HASH_BUCKETS;
}


hash_table_t *
hash_new ()
{
  hash_table_t *h;
  h = (hash_table_t *)malloc (sizeof *h);
  memset (h, 0, sizeof *h);
  return h;
}


void
hash_free (hash_table_t *h)
{
  hash_elt_t *e, *next;
  int i;

  for (i = 0; i < NUM_HASH_BUCKETS; i++)
    {
      for (e = h->bucket[i]; e != NULL; e = next)
	{
	  next = e->next;
	  free (e->key);
	  free (e->value);
	  free (e);
	}
    }

  free (h);
}


const char *
hash_lookup (hash_table_t *h, const char *key)
{
  hash_elt_t *e;
  for (e = h->bucket[hash (key)]; e != NULL; e = e->next)
    {
      if (!strcmp (e->key, key))
	return e->value;
    }

  return NULL;
}


void
hash_insert (hash_table_t *h, const char *key, const char *value)
{
  if (!hash_lookup (h, key))
    {
      hash_elt_t *e, **bucket;
      bucket = &h->bucket[hash (key)];
      e = (hash_elt_t *)malloc (sizeof *e);
      e->key   = strcpy (malloc (strlen (key  ) + 1), key);
      e->value = strcpy (malloc (strlen (value) + 1), value);
      e->next = *bucket;
      *bucket = e;
    }
}


void
hash_remove (hash_table_t *h, const char *key)
{
  hash_elt_t **e;
  for (e = &h->bucket[hash (key)]; (*e) != NULL; e = &(*e)->next)
    {
      if (!strcmp ((*e)->key, key))
	{
	  hash_elt_t *old;
	  free ((*e)->key);
	  free ((*e)->value);
	  old = *e;
	  *e = (*e)->next;
	  free (old);
	  return;
	}
    }

  fprintf (stderr, "Failed to remove elt from hash table!\n");
}
