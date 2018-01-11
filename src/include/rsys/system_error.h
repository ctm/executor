#if !defined(_SYSTEM_ERROR_H_)
#define _SYSTEM_ERROR_H_

namespace Executor
{
enum
{
    GENERIC_COMPLAINT_ID = -3000
};

typedef void (*system_error_callback_t)(void);

extern int system_error(const char *_message, int _default_button,
                        const char *button0,
                        const char *button1,
                        const char *button2,
                        system_error_callback_t func0,
                        system_error_callback_t func1,
                        system_error_callback_t func2);
}

#endif /* !_SYSTEM_ERROR_H_ */
