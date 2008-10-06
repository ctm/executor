
/* view.c */
void changeview (short view, Rect * r);
void iconview (void);
void icsview (void);
void listingview (void);
void updatewin (...);
void sortwin (int (*f) (const void *, const void *), short order);
int namecmp (const void *x, const void *y);
void namesort (void);
int datecmp (const void *x, const void *y);
void moddatesort (void);
int sizecmp (const void *x, const void *y);
void sizesort (void);
void defaultsort (void);
