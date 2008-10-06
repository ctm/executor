/*
 * These are macros that may be useful elsewhere.  I don't really intend
 * for them to live in a file named "for_sam.h", but I'm not the SDL
 * architect, so it's not my place to determine where they should live.
 */

#if defined (__GNUC__)

/* advance_n_bytes advances the pointer pointed to by ptrp by n_bytes,
   regardless of what ptrp points to. */

#define advance_n_bytes(ptrp, n_bytes)				\
({								\
  typeof (ptrp) _ptrp;						\
								\
  _ptrp = (ptrp);						\
  *(_ptrp) = (typeof (*_ptrp))((char *)*(_ptrp) + n_bytes);	\
})

#define advance_n_bytes_voidp(ptrp, n_bytes) advance_n_bytes (ptrp, n_bytes)

#else

/* This macro will cause ptrp to be evaluated twice and will also cause
   compiler warnings.  Should this unsafe macro even be defined here?*/

#define advance_n_bytes(ptrp, n_bytes) \
  (*(ptrp) = ((char *)*(ptrp) + n_bytes))

/* advance_n_bytes_voidp has no such bad side-effects, but it's void *
   specific */

static void
advance_n_bytes_voidp (void **ptrp, size_t n_bytes)
{
  *(ptrp) = (char *)*(ptrp) + n_bytes;
}

#endif /* !defined (__GNUC__) */

#if defined (__GNUC__)

#define MAX(a, b)		  \
({				  \
  typeof (a) _a;		  \
  typeof (b) _b;		  \
				  \
  _a = (a);			  \
  _b = (b);			  \
  _a >= _b ? _a : _b;		  \
})

#define MAX_size_t(a, b) MAX (a, b)

#else

/* This macro will cause a to be evaluated twice and may also cause
   b to be evaluated twice.  Should this unsafe macro even be defined
   here? */

#define MAX(a, b) ((a) >= (b) ? (a) : (b))

/* MAX_size_t has no such bad side-effects, but it's size_t specific */

static size_t
MAX_size_t (size_t a, size_t b)
{
  size_t retval;

  retval = a >= b ? a : b;
  return retval;
}

#endif /* !defined (__GNUC__) */
