#if !defined (_RSYS_TYPES_H_)
#define _RSYS_TYPES_H_

#if !defined (INT_TYPES_TYPEDEFED)
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;

#define INT_TYPES_TYPEDEFED
#endif /* !INT_TYPES_TYPEDEFED */

#if !defined (BOOLEAN_T_TYPEDEFED)
# undef FALSE
# undef TRUE
typedef enum { FALSE, TRUE } boolean_t;
#else /* BOOLEAN_T_TYPEDEFED */
# if !defined (FALSE)
#  define FALSE 0
# endif
# if !defined (TRUE)
#  define TRUE 1
# endif
#endif /* BOOLEAN_T_TYPEDEFED */

#if !defined (INT16_MAX)
#define INT16_MAX	((int16) 0x7FFF)
#endif

#if !defined (INT16_MIN)
#define INT16_MIN	((int16) (-INT16_MAX - 1))
#endif

#if !defined (UINT16_MAX)
#define UINT16_MAX	((uint16) 0xFFFF)
#endif

#if !defined (INT32_MAX)
#define INT32_MAX	((int32) 0x7FFFFFFF)
#endif

#if !defined (INT32_MIN)
#define INT32_MIN	((int32) (-INT32_MAX - 1))
#endif

#if !defined (UINT32_MAX)
#define UINT32_MAX	((uint32) 0xFFFFFFFF)
#endif

#endif /* !_RSYS_TYPES_H_ */
