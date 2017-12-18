#if !defined(__safe_alloca__)
#define __safe_alloca__

#if !defined(NDEBUG)

#define FIREWALL_START 0x940504FE
#define FIREWALL_STOP_0 0xEE
#define FIREWALL_STOP_1 0x41
#define FIREWALL_STOP_2 0x51
#define FIREWALL_STOP_3 0x59

/*
 * I don't like this implementation, but it works.
 */

#define SAFE_DECL()           \
    unsigned long ___newsize; \
    unsigned char *___tempptr

#define SAFE_alloca(size)                                      \
    (                                                          \
        ___newsize = (size) + 3 * sizeof(long),                \
        ___tempptr = (decltype(___tempptr))alloca(___newsize), \
        ((uint32_t *)___tempptr)[0] = ___newsize,                \
        ((uint32_t *)___tempptr)[1] = FIREWALL_START,            \
        ___tempptr[___newsize - 4] = FIREWALL_STOP_0,          \
        ___tempptr[___newsize - 3] = FIREWALL_STOP_1,          \
        ___tempptr[___newsize - 2] = FIREWALL_STOP_2,          \
        ___tempptr[___newsize - 1] = FIREWALL_STOP_3,          \
        (void *)(___tempptr + 2 * sizeof(long)))

#define ASSERT_SAFE(var)                                           \
    do                                                             \
    {                                                              \
        ___tempptr = (unsigned char *)(var);                       \
        ___tempptr -= 2 * sizeof(long);                            \
        gui_assert(((uint32_t *)___tempptr)[1] == FIREWALL_START);   \
        ___newsize = ((uint32_t *)___tempptr)[0];                    \
        gui_assert(___tempptr[___newsize - 4] == FIREWALL_STOP_0); \
        gui_assert(___tempptr[___newsize - 3] == FIREWALL_STOP_1); \
        gui_assert(___tempptr[___newsize - 2] == FIREWALL_STOP_2); \
        gui_assert(___tempptr[___newsize - 1] == FIREWALL_STOP_3); \
    } while(0)

#else /* defined(NDEBUG) */

#define SAFE_DECL()
#define SAFE_alloca(size) alloca(size)
#define ASSERT_SAFE(var) \
    do                   \
    {                    \
    } while(0)

#endif /* defined(NDEBUG) */

#endif /* !defined(__safe_alloca__) */
