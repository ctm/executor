#if !defined (_RSYS_VERSION_H_)
#define _RSYS_VERSION_H_

/* $Id: version.h 94 2005-05-25 15:53:40Z ctm $ */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined (EXECUTOR_VERSION)
#define EXECUTOR_VERSION "2.1pr16" /* don't forget EXECUTOR_VERSION_NUMERIC */
#endif

#define SYSTEM_VERSION_ENCODING(m,n,x,b) ((m) * 1000000 \
					  + (n) * 10000 \
					  + ((x) ? ((x) - 'a' + 1) * 100 : 0) \
					  + (b))

#if !defined (EXECUTOR_VERSION_NUMERIC)
#define EXECUTOR_VERSION_NUMERIC SYSTEM_VERSION_ENCODING(2, 1, 0, 16)
#endif

#define MINIMUM_SYSTEM_FILE_NEEDED SYSTEM_VERSION_ENCODING(2, 0, 0, 3)

#define MAJOR_REVISION 2
#define MINOR_REVISION 1

extern const char ROMlib_executor_version[];
extern const char *ROMlib_executor_full_name;

#define PLATFORM_MOD 10 /* last decimal digit of serial number is platform */
#define COMPOSITE_MOD 100 /* 2nd to last digit is price class */

#if defined(NEXTSTEP)
#define VERSION_SIG PLATFORM_NEXTSTEP
#elif defined(MSDOS)
#define VERSION_SIG PLATFORM_DOS
#elif defined(CYGWIN32)
#define VERSION_SIG PLATFORM_CYGWIN32
#elif defined(LINUX)
#define VERSION_SIG PLATFORM_LINUX
#elif defined(MACOSX_)
#define VERSION_SIG PLATFORM_MACOSX
#else
#error "Need VERSION_SIG #define"
#endif

enum { CLASS_COMMERCIAL = 0, CLASS_EDUCATIONAL = 10, CLASS_STUDENT = 20 };
enum { PLATFORM_NEXTSTEP = 0, PLATFORM_CYGWIN32 = 1, PLATFORM_DOS = 2,
       PLATFORM_LINUX = 3, PLATFORM_ANY = 4, PLATFORM_MACOSX = 5 };

enum
{
  STUDENT_DOS = CLASS_STUDENT + PLATFORM_DOS,
  STUDENT_LINUX = CLASS_STUDENT + PLATFORM_LINUX,
};

#define EXECUTOR_NAME "executor"

extern void ROMlib_set_system_version (uint32 version);

#ifdef __cplusplus
}
#endif

#endif /* !_RSYS_VERSION_H_ */
