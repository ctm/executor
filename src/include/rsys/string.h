#if !defined(_RSYS_STRING_H_)
#define _RSYS_STRING_H_

namespace Executor
{

#define TEMP_C_STRING_FROM_STR255(str)                       \
    ({                                                       \
        char *retval;                                        \
        int str_len;                                         \
                                                             \
        str_len = *(unsigned char *)(str);                   \
        retval = (char *)alloca(str_len + 1);                \
        memcpy(retval, (unsigned char *)(str) + 1, str_len); \
        retval[str_len] = 0;                                 \
        retval;                                              \
    })

extern void str255_from_c_string(Str255 str255, const char *c_stringp);
extern char *pstr_index_after(StringPtr p, char c, int i);
extern void str63assign(Str63 new1, const StringPtr old);
extern void str31assign(Str63 new1, const StringPtr old);
extern StringHandle stringhandle_from_c_string(const char *c_stringp);
}

#endif /* !_RSYS_STRING_H_ */
