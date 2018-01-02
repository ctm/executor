#if !defined(_ERROR_H_)
#define _ERROR_H_

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include <stdint.h>
#include <stdbool.h>

#include "rsys/noreturn.h"

#ifdef __cplusplus
#include <string>
bool error_parse_option_string(std::string options);
extern "C" {
#endif
/*
 * Classes of warnings:
 *
 * warning_unimplemented: unimplemented code
 * warning_unexpected: unexpected parameter or return value
 * warning_trace_info: program flow trace information
 *
 * Classes of logs:
 
 * warning_trap_failure: trap returning some sort of failure
 * filesystem call & retval logging (will help Quark)
 *
 * Classes of tests:
 *
 * memory manager slamming
 * text edit slamming
 */

/* Can't use an enum here, since the preprocessor needs to be able to
 * constant fold expressions involving these values so we can
 * conditionally compile stuff.
 */
#define ERROR_UNIMPLEMENTED 0
#define ERROR_UNEXPECTED 1
#define ERROR_TRACE_INFO 2
#define ERROR_ERRNO 3
#define ERROR_TRAP_FAILURE 4
#define ERROR_FILESYSTEM_LOG 5
#define ERROR_MEMORY_MANAGER_SLAM 6
#define ERROR_TEXT_EDIT_SLAM 7
#define ERROR_SOUND_LOG 8
#define ERROR_FLOATING_POINT 9

/* This is a bit mask for those errors for which we should compile
 * in support.  This way we can conditionally compile in only
 * the code we want.  */
#if !defined(ERROR_SUPPORTED_MASK)
#define ERROR_SUPPORTED_MASK (~0) /* Compile in support for all errors. */
#endif

#define SUPPORT_LOG_ERR_TO_RAM /* simplification opportunity: is this feature useful? */

#define ERROR_BIT_MASK(err) (1 << (err))

/* Returns true iff that error is supported by this compile. */
#define ERROR_SUPPORTED_P(err) \
    ((ERROR_SUPPORTED_MASK & ERROR_BIT_MASK(err)) != 0)

/* Returns true iff that error is enabled at runtime. */
#define ERROR_ENABLED_P(err) \
    ((ROMlib_debug_level & ERROR_SUPPORTED_MASK & ERROR_BIT_MASK(err)) != 0)

#ifdef _MSC_VER
#define ATTR_FORMAT(...)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#else
#define ATTR_FORMAT(...) __attribute((format(__VA_ARGS__)))
#endif

extern uint32_t ROMlib_debug_level;

extern bool error_parse_option_string(const char *options);
extern bool error_set_enabled(int err, bool enabled_p);

#define gui_assert(expr) \
    ((void)((expr)       \
                ? 0      \
                : (gui_fatal("failed assertion `" #expr "'"), 0)))

/* ### eventually this should be nuked */
#define gui_abort() gui_fatal("abort")

#define gui_fatal(...) \
    _gui_fatal(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)

extern _NORET_1_ void _gui_fatal(const char *file, int line, const char *fn,
                                 const char *fmt, ...) _NORET_2_;

#define warning_helper(error, type, ...)      \
    _warning(error, type, __FILE__, __LINE__, \
             __PRETTY_FUNCTION__, " " __VA_ARGS__)

extern void _warning(int error, const char *type, const char *file, int line,
                     const char *fn, const char *fmt, ...)
    ATTR_FORMAT(printf, 6, 7);

#if ERROR_SUPPORTED_P(ERROR_UNIMPLEMENTED)
#define warning_unimplemented(...)          \
    warning_helper(ERROR_UNIMPLEMENTED,     \
                   "unimplemented feature", \
                   __VA_ARGS__)
#else
#define warning_unimplemented(fmt, args...)
#endif

#if ERROR_SUPPORTED_P(ERROR_UNEXPECTED)
#define warning_unexpected(...)        \
    warning_helper(ERROR_UNEXPECTED,   \
                   "unexpected event", \
                   __VA_ARGS__)
#else
#define warning_unexpected(fmt, args...)
#endif

#if ERROR_SUPPORTED_P(ERROR_FILESYSTEM_LOG)
#define warning_fs_log(...)              \
    warning_helper(ERROR_FILESYSTEM_LOG, \
                   "filesystem",         \
                   __VA_ARGS__)
#else
#define warning_fs_log(fmt, args...)
#endif

#if ERROR_SUPPORTED_P(ERROR_TRACE_INFO)
#define warning_trace_info(...)      \
    warning_helper(ERROR_TRACE_INFO, \
                   "trace info",     \
                   __VA_ARGS__)
#else
#define warning_trace_info(fmt, args...)
#endif

#if ERROR_SUPPORTED_P(ERROR_TRAP_FAILURE)
#define warning_trap_failure(...)      \
    warning_helper(ERROR_TRAP_FAILURE, \
                   "trap failure",     \
                   __VA_ARGS__)
#else
#define warning_trap_failure(fmt, args...)
#endif

#if ERROR_SUPPORTED_P(ERROR_SOUND_LOG)
extern void _sound_warning(const char *file, int line, const char *fn,
                           const char *fmt, ...);
#define warning_sound_log(...) \
    _sound_warning(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#else
#define warning_sound_log(fmt, args...)
#endif

#if ERROR_SUPPORTED_P(ERROR_FLOATING_POINT)
#define warning_floating_point(...)      \
    warning_helper(ERROR_FLOATING_POINT, \
                   "FP",                 \
                   __VA_ARGS__)
#else
#define warning_floating_point(fmt, args...)
#endif

#define errno_fatal(...) \
    _errno_fatal(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
extern _NORET_1_ void _errno_fatal(const char *file, int line, const char *fn,
                                   const char *fmt, ...) _NORET_2_
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6)
    /* Older gcc's don't like two __attribute__ directives at once. */
    __attribute__((format(printf, 4, 5)))
#endif
    ;

#if ERROR_SUPPORTED_P(ERROR_ERRNO)
#define warning_errno(...) \
    _errno_warning(__FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
extern void _errno_warning(const char *file, int line, const char *fn,
                           const char *fmt, ...)
    ATTR_FORMAT(printf, 4, 5);
#else
#define warning_errno(fmt, args...)
#endif

#if defined(SUPPORT_LOG_ERR_TO_RAM)
extern bool log_err_to_ram_p;
extern void error_dump_ram_err_buf(const char *separator_message);
#endif

//extern const char NULL_STRING[];
#define NULL_STRING ""
#ifdef __cplusplus
}
#endif
#endif /* !_ERROR_H_ */
