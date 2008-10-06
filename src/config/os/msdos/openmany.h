#if !defined (_MSDOS_OPENMANY_H_)
#define _MSDOS_OPENMANY_H_

/* This is not a strict limit, but merely a hint.  It's hard to
 * do a proper limit because some files may already be open,
 * perhaps even in a way we can't check (e.g. by the DOS stub).
 */
#define MSDOS_DESIRED_MIN_FILES 30

typedef enum
{
  MSDOS_OM_SUCCESS,
  MSDOS_OM_FAILURE,
  MSDOS_OM_UNABLE_TO_TEST,
  MSDOS_OM_NOT_TESTED
} msdos_open_many_result_t;

extern msdos_open_many_result_t msdos_open_many_result;

extern boolean_t msdos_test_max_files (void);

#endif /* !_MSDOS_OPENMANY_H_ */
