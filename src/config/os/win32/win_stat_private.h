#if !defined(__STAT_FUN_PRIVATE_H__)
#define __STAT_FUN_PRIVATE_H__

#if 0 /* this stuff is no longer used */

typedef struct stat_hash_str
{
  char *stat_name;
  uint32_t stat_ino;
  struct stat_hash_str *stat_next;
}
stat_hash_t;

enum { N_STAT_ENTRIES = 101 };

#endif

#endif
