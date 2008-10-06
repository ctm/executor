struct foo
{
  int elem;
};

typedef struct foo **Handle;

extern Handle NewHandle (int);
#define noErr 0
extern int MemError(void);
typedef int Size;
extern Size GetHandleSize (Handle);
extern void DisposHandle (Handle);
