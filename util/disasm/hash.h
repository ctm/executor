#ifndef _HASH_H_
#define _HASH_H_

#define LOG2_NUM_HASH_BUCKETS 11
#define NUM_HASH_BUCKETS (1UL << LOG2_NUM_HASH_BUCKETS)

typedef struct _hash_elt_t
{
  struct _hash_elt_t *next;
  char *key;
  char *value;
} hash_elt_t;

typedef struct
{
  hash_elt_t *bucket[NUM_HASH_BUCKETS];
} hash_table_t;

extern hash_table_t *hash_new (void);
extern void hash_free (hash_table_t *h);
extern const char *hash_lookup (hash_table_t *h, const char *key);
extern void hash_insert (hash_table_t *h, const char *key, const char *value);
extern void hash_remove (hash_table_t *h, const char *key);

#endif  /* !_HASH_H_ */
