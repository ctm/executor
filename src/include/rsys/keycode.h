#if !defined (NELEM)
#define NELEM(s) (sizeof (s) / sizeof (s)[0])
#endif
namespace Executor {
#define BASE_REVISION	1    /* revision expiration started with E 1.x */
#define MONTHS_IN_YEAR	12
#define BASE_MONTH	1    /* January is typically digitized as 1 */
#define BASE_YEAR	1995 /* expiration dates started in 1995 */

typedef unsigned char block_t[65];
typedef unsigned char ordering_t[65];

typedef struct
{
  long int serial_number;
  unsigned long n_cpu:16;
  unsigned int major_revision:3;
  unsigned int updates_p:1;
  unsigned int expires_p:1;
  unsigned int last_month:4;
  unsigned int last_year;
} decoded_info_t;

extern int serial_bits[32];
extern int n_cpu_bits[16];
extern int revision_number_bits[4];
extern int expiration_date_bits[8];
extern int unassigned_bits[5];
extern unsigned char key32[33];

extern int valid_key_format(const unsigned char *key);
extern void undes(block_t ciphertext, block_t keyin, block_t plaintext);
extern void texttoblock(const unsigned char *theirkey, block_t text);
extern void bitstonum(unsigned long *result, const int *locs, int numbits,
		      const block_t text);
extern int decode(const unsigned char *theirkey, decoded_info_t *infop);

extern void transpose(block_t data, ordering_t t, int n);
extern void f(int i, block_t key, block_t a, block_t x);
extern ordering_t InitialTr;
extern ordering_t KeyTr1;
extern ordering_t FinalTr;
extern ordering_t swap;
extern block_t key;
}
