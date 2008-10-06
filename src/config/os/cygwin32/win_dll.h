#if !defined (_win_dll_h_)
#define _win_dll_h_

enum { DLL_TYPE  = 0x646c6c20 };  /* 'dll ' */
enum { DLL_MAGIC = 0x9cf237cd }; /* random */

typedef struct
{
  uint32 type; /* see above */
  uint32 magic; /* see above */
  uint32 size; /* total length of parameter block */
  const char *dll_name;
  const char *function_name;
  void *arg_to_function;
}
dll_param_block;

enum
{
  DLL_NO_ERROR = 0,
  DLL_BAD_MAGIC_ERROR = -1,
  DLL_BAD_LENGTH_ERROR = -2,
  DLL_NO_LIBRARY_ERROR = -3,
  DLL_NO_FUNCTION_ERROR = -4,
};

#endif
