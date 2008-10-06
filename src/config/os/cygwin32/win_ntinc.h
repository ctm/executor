#define IN

#define CTL_CODE(device_type, function, method, access)	\
(((device_type) << 16) |				\
 ((function) << 2) |					\
 ((method) << 0) |					\
 ((access) << 14))

#define METHOD_BUFFERED 0
#define FILE_ANY_ACCESS 0
