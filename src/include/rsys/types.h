#if !defined(_RSYS_TYPES_H_)
#define _RSYS_TYPES_H_

#if !defined(INT_TYPES_TYPEDEFED)
#include <stdint.h>
#include <sys/types.h>
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16_t;
typedef uint16_t uint16_t;
typedef int32_t int32_t;
typedef uint32_t uint32_t;
typedef int64_t int64_t;
typedef uint64_t uint64_t;

#define INT_TYPES_TYPEDEFED
#endif /* !INT_TYPES_TYPEDEFED */

#if !defined(INT16_MAX)
#define INT16_MAX ((int16_t)0x7FFF)
#endif

#if !defined(INT16_MIN)
#define INT16_MIN ((int16_t)(-INT16_MAX - 1))
#endif

#if !defined(UINT16_MAX)
#define UINT16_MAX ((uint16_t)0xFFFF)
#endif

#if !defined(INT32_MAX)
#define INT32_MAX ((int32_t)0x7FFFFFFF)
#endif

#if !defined(INT32_MIN)
#define INT32_MIN ((int32_t)(-INT32_MAX - 1))
#endif

#if !defined(UINT32_MAX)
#define UINT32_MAX ((uint32_t)0xFFFFFFFF)
#endif

namespace Executor
{
struct Point
{
    int16_t v;
    int16_t h;
};
}

#endif /* !_RSYS_TYPES_H_ */
