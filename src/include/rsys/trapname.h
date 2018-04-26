#if !defined(_RSYS_TRAPNAME_H_)
#define _RSYS_TRAPNAME_H_

#if !defined(NDEBUG)
extern const char *trap_name(int trapno);
extern const char *trap_name_array[0x1000];
#endif

#endif /* !_RSYS_TRAPNAME_H_ */
