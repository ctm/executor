/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "opfind.h"
#include "asmsamples.h"

typedef struct
{
  unsigned short start;
  unsigned char size;
  unsigned char done;
  const char *src_reg;  /* e.g. "%ecx", or NULL when movl $1,4(%edi), etc. */
  unsigned long value;  /* This field is only valid if src_reg == NULL.    */
} move_t;


typedef struct
{
  unsigned long signature;   /* Magic value in asm_code[asm_index] op slots. */
  const char *reg_name;      /* Name of register, e.g. "%ecx".               */
} op_signature_t;


/* Index of the asm we want to process. */
int asm_index;


/* This creates a list of move_t's to output all of the fixed bytes for
 * a given set of assembly code.
 */
static int
construct_fixed_moves (const unsigned char *vary_mask, int code_size,
		       move_t *move)
{
  int num_moves;
  int i;

  for (i = 0, num_moves = 0; i < code_size; )
    {
      if (vary_mask[i] == 0)
	{
	  move_t *m;
	  int j, s;

	  m = &move[num_moves++];
	  m->start = i;
	  m->done = FALSE;

	  /* Find the best size of move. */
	  for (s = 3; s >= 0; s--)
	    if (i + s < code_size && vary_mask[i + s] == 0)
	      break;
	  ++s;
	  m->size = (s != 3) ? s : 4;

	  /* Compute the fixed value.  Since all asm templates share
	   * the same fixed bytes (by definition) we just grab the
	   * byte values from the first one.
	   */
	  m->value = 0;
	  for (j = 0; j < s; j++)
	    m->value |= asm_code[asm_index].start[i + j] << (8 * j);

	  i += s;
	}
      else
	i++;
    }

  return num_moves;
}


/* This gives a rough guess as to how well two moves will pair back-to-back
 * on the Pentium.  I'm not sure how misalignment affects pairing, so this
 * just ignores that issue.
 */
static int
pairing_goodness (const move_t *m1, const move_t *m2)
{
  unsigned mask1, mask2, mask;
  int i, score, pairable1, pairable2;

  /* Default to a score of 1000 (worst possible). */
  score = 1000;

  /* Try not to put two word moves back to back, since they need to go
   * in the U-pipe.
   */
  if (m1->size != 2 || m2->size != 2)
    score += 100;

  /* See if each instruction can be paired with anyone.
   * On the Pentium, instructions with an immediate value and
   * a constant displacement are not pairable!
   */
  pairable1 = (m1->start == 0 || m1->src_reg != NULL);
  pairable2 = (m2->start == 0 || m2->src_reg != NULL);

  /* If we are wasting a pairable instruction, this choice sucks. */
  if (pairable1 != pairable2)
    score -= 500;

  /* Try to "use up" the word moves by placing them next to long or
   * byte moves, so we don't end up with several word moves at the end.
   */
  if (m1->size != 2 && m2->size != 2)
    score--;

  /* Compute bit masks for the cache bytes touched by each move.  We
   * do this because adjacent moves which touch the same 4-byte cache
   * bank are slower than those which do not.
   */
  for (i = m1->start, mask1 = 0; i < m1->start + m1->size; i++)
    mask1 |= 1 << (i & 31);
  for (i = m2->start, mask2 = 0; i < m2->start + m2->size; i++)
    mask2 |= 1 << (i & 31);

  /* Since we might be misaligned, guess the odds of a cache bank
   * collision by considering all possible cache banks and finding out
   * how many collide with both moves.
   */
  for (i = 0, mask = 0xF; i < 32; i++)
    {
      /* If no collision with this possible bank, count that as being
       * slightly good.  Those moves which can't possibly collide will
       * end up with the best score.  Those which might collide
       * (i.e. adjacent longs which might be misaligned and can
       * therefore overlap the same cache bank) won't get quite as
       * good a score, but will still do better than moves that
       * necessarily collide.
       */
      if (!(mask & mask1) || !(mask & mask2))
	score++;

      /* Rotate the mask. */
      mask = (mask >> 31) | (mask << 1);
    }

  return score;
}


static const unsigned long size_mask[5] = { 0, 0xFF, 0xFFFF, 0, 0xFFFFFFFF };


static int
popular_constant (const move_t *move, int num_moves,
		  unsigned long *valp)
{
  int i, j, best_count;

  best_count = 0;
  *valp = 0;  /* default */

  for (i = 0; i < num_moves; i++)
    if (move[i].src_reg == NULL)
      {
	int count;
	unsigned long n = move[i].value;

	/* Compare against all others.  Don't start looping at i + 1 because
	 * a later long move value might help an earlier short move, but not
	 * vice versa.  We need to check all possibilities.
	 */
	for (j = 0, count = 0; j < num_moves; j++)
	  if (move[j].src_reg == NULL
	      && ((n & size_mask[move[j].size]) == move[j].value
		  || (move[j].size == 1
		      && ((n >> 8) & 0xFF) == move[j].value)))
	    count++;

	/* Did we break the record? */
	if (count > best_count)
	  {
	    best_count = count;
	    *valp = n;
	  }
      }

  return best_count;
}


/* Returns TRUE if a given register->memory move will be overwritten
 * by a constant move which has not yet been done.  This can happen in
 * this situation:
 *
 * 0xAA 0xBB 0xCC <opbyte0> <opbyte1> <opbyte2> <opbyte3>
 *
 * The Right Way to do this is to output a long to fill in the first
 * three bytes, then write out the operand to fill in the last four
 * bytes.  If we do it in the other order, opbyte0 will get stomped
 * by the last (garbage) byte of the constant long being written out.
 * This function identifies if such stompage would occur, so we know
 * to wait and output the operand bytes later.
 */
static int
operand_will_be_stomped (const move_t *op, const move_t *move_list,
			 int num_moves)
{
  int i;

  for (i = 0; i < num_moves; i++)
    {
      const move_t *m = &move_list[i];
      if (!m->done && m->src_reg == NULL
	  && m->start < op->start + op->size
	  && m->start + m->size > op->start)
	return TRUE;
    }
  return FALSE;
}


/* Attempts to do a decent job of Pentium instruction scheduling for
 * this list of move instructions.
 */
static void
schedule_move_list (move_t *move, int num_moves)
{
  int i, j, moves_done, popular_constant_count;
  move_t *new_move;
  unsigned long most_popular_constant;

  if (num_moves == 0)
    return;

  /* Allocate room for the new list. */
  new_move = (move_t *) alloca ((num_moves + 1) * sizeof new_move[0]);
  moves_done = 0;

#if 1
  /* See if there's some constant we use over and over again. */
  popular_constant_count = popular_constant (move, num_moves,
					     &most_popular_constant);
#else
  popular_constant_count = 0;
#endif

  /* Put the most popular constant in %ecx and use that. */
  if (popular_constant_count > 1)
    {
      printf ("\tmovl\t$0x%lX,%%ecx\n", most_popular_constant);
      for (i = 0; i < num_moves; i++)
	{
	  int size;

	  size = move[i].size;
	  if (move[i].src_reg == NULL)
	    {
	      if (move[i].value == (most_popular_constant & size_mask[size]))
		{
		  static const char *reg_name[5]
		    = { NULL, "%cl", "%cx", NULL, "%ecx" };
		  move[i].src_reg = reg_name[size];
		}
	      else if (size == 1
		       && move[i].value == ((most_popular_constant >> 8)
					    & 0xFF))
		move[i].src_reg = "%ch";
	    }
	}
    }

  new_move[moves_done] = move[0];
  move[0].done = TRUE;
  moves_done++;

  while (1)
    {
      int best_score, best_index;

      /* Find the move with the best Pentium pairing (greedy). */
      for (i = 1, best_score = -1, best_index = -1; i < num_moves; i++)
	if (!move[i].done
	    && (move[i].src_reg == NULL
		|| !operand_will_be_stomped (&move[i], move, num_moves)))
	  {
	    int score = pairing_goodness (&new_move[moves_done - 1], &move[i]);
	    if (score > best_score)
	      {
		best_score = score;
		best_index = i;
	      }
	  }

      /* If no match was found, we are done. */
      if (best_index == -1)
	break;

      /* Make that move next. */
      new_move[moves_done] = move[best_index];
      move[best_index].done = TRUE;
      moves_done++;
    }

  /* Replace the old list with the new list. */
  memcpy (move, new_move, num_moves * sizeof move[0]);
}


/* Prints out assembly for a list of move instructions. */
static void
print_move_list (const move_t *move, int num_moves)
{
  int i;

  for (i = 0; i < num_moves; i++)
    {
      char offset[32], src_operand[32];

      /* Compute a string for the offset.  We use the empty string instead
       * of "0" because gas generates bigger code if you specify an
       * explicit 0 offset instead of just leaving it out altogether.
       */
      if (move[i].start == 0)
	offset[0] = '\0';
      else
	sprintf (offset, "%d", move[i].start);

      /* Compute a string for the source operand. */
      if (move[i].src_reg != NULL)
	strcpy (src_operand, move[i].src_reg);
      else
	sprintf (src_operand, "$0x%lX", move[i].value);

      /* Print out the asm code. */
      printf ("\tmov%c\t%s,%s(%%edi)\n",
	      "?bw?l"[move[i].size], src_operand, offset);
    }
}


/* Prints out the offsets for the longs that vary between asm_code entries. */
static int
construct_operand_moves (const unsigned char *mask, int code_size,
			 const op_signature_t *sig, int num_sigs,
			 move_t *move)
{
  int i, num_moves;

  for (i = 0, num_moves = 0; i < code_size; )
    {
      if (mask[i] != 0)
	{
	  unsigned long val;
	  int s;

	  if (i + 4 > code_size
	      || mask[i    ] != 0xFF || mask[i + 1] != 0xFF
	      || mask[i + 2] != 0xFF || mask[i + 3] != 0xFF)
	    {
	      fprintf (stderr, "Found varying bytes, but they weren't "
		       "a solid block of four!\n");
	      exit (EXIT_FAILURE);
	    }
	  
	  /* Fetch the operand value from the first code template.
	   * This is the "signature" value and can tell us which operand
	   * we have.
	   */
	  val = (asm_code[asm_index].start[i]
		 | (asm_code[asm_index].start[i + 1] << 8)
		 | (asm_code[asm_index].start[i + 2] << 16)
		 | (asm_code[asm_index].start[i + 3] << 24));

	  /* Try to find a match for this signature. */
	  for (s = 0; s < num_sigs; s++)
	    if (sig[s].signature == val)
	      {
		move_t *m = &move[num_moves++];
		m->start   = i;
		m->size    = 4;
		m->done    = FALSE;
		m->src_reg = sig[s].reg_name;
		m->value   = 0;
		break;
	      }

	  i += 4;
	}
      else
	i++;
    }

  return num_moves;
}
 

static void
generate_asm (const unsigned char *vary_mask, int code_size,
	      const op_signature_t *sig, int num_sigs,
	      int gen_fixed)
{
  move_t *move;
  int num_moves;

  /* Allocate an empty array. */
  move = (move_t *) alloca (code_size * sizeof move[0]);
  memset (move, 0, code_size * sizeof move[0]);

  /* Construct the moves for the fixed (constant) values. */
  if (gen_fixed)
    num_moves = construct_fixed_moves (vary_mask, code_size, move);
  else
    num_moves = 0;

  /* Construct the moves for any operands we know about. */
  num_moves += construct_operand_moves (vary_mask, code_size, sig, num_sigs,
					move + num_moves);

  /* Instruction schedule the moves for the Pentium. */
  schedule_move_list (move, num_moves);

  /* Dump out the asm code. */
  print_move_list (move, num_moves);
}


/* This computes a bit mask of every bit that varies between any two
 * asm_code entries.
 */
static void
compute_vary_mask (unsigned char *mask, int code_size)
{
  const asm_code_t *a, *b;
  int i;

  memset (mask, 0, code_size);
  for (a = &asm_code[asm_index]; a->start != NULL; a++)
    for (b = a + 1; b->start != NULL; b++)
      {
	for (i = 0; i < code_size; i++)
	  mask[i] |= a->start[i] ^ b->start[i];
      }
}


/* This computes the size of the asm code and returns it.  It also checks
 * for any inconsistencies in the supplied templates.
 */
static int
compute_code_size (void)
{
  int i, size;

  for (size = 0, i = asm_index; asm_code[i].start != NULL; i++)
    {
      int new_size = asm_code[i].end - asm_code[i].start;

      if (i == asm_index)
	size = new_size;
      else if (size != new_size)
	{
	  fprintf (stderr, "asm sizes differ based on operands!\n"
		   "This takes %d bytes:\n", size);
	  fputs (asm_code[asm_index].code, stderr);
	  fprintf (stderr, "\nWhile this takes %d bytes:\n", new_size);
	  fputs (asm_code[i].code, stderr);
	  exit (EXIT_FAILURE);
	}
    }

  return size;
}


static int
compute_operand_signatures (op_signature_t *sig, int argc, char *argv[])
{
  int i, j, num_sigs;

  for (i = 0, num_sigs = 0; i < argc; i += 2)
    {
      unsigned long l;

      /* Make sure the register name starts with a `%'. */
      if (argv[i + 1][0] != '%')
	{
	  fprintf (stderr, "Illegal register name \"%s\".  Register names "
		   "must start with `%%'.\n", argv[i + 1]);
	  exit (EXIT_FAILURE);
	}

      /* Read in the signature value. */
      l = 0;
      sscanf (argv[i] + 2, "%lx", &l);
      if (l == 0)
	{
	  fprintf (stderr,
		   "Illegal signature number \"%s\"; must be a nonzero "
		   "hexadecimal integer.\n", argv[i]);
	  exit (EXIT_FAILURE);
	}

      /* Check for duplicate signatures. */
      for (j = 0; j < num_sigs; j++)
	if (sig[j].signature == l)
	  {
	    fprintf (stderr, "Fatal error: duplicate operand signatures.\n");
	    exit (EXIT_FAILURE);
	  }

      /* Save the info away. */
      sig[num_sigs].signature = l;
      sig[num_sigs].reg_name  = argv[i + 1];
      num_sigs++;
    }

  return num_sigs;
}


int
main (int argc, char *argv[])
{
  unsigned char *vary_mask;
  int i, code_size, num_sigs;
  op_signature_t *sig;
  
  /* Check args. */
  if (argc < 3
      || !(argc % 2)
      || (strcmp (argv[2], "-genasm")
	  && strcmp (argv[2], "-genasm-skip-fixed")
	  && strcmp (argv[2], "-print-size")))
    {
      fprintf (stderr,
	       "Usage: %s asm_index <-genasm|-genasm-skip-fixed|-print-size>\n"
	       "\t\t[operand-signature1 reg1] [operand-signature2 reg2]...\n",
	       argv[0]);
      exit (EXIT_FAILURE);
    }

  asm_index = atoi (argv[1]);

  /* Compute operand signatures. */
  sig = (op_signature_t *) alloca (argc * sizeof sig[0]);
  memset (sig, 0, argc * sizeof sig[0]);
  num_sigs = compute_operand_signatures (sig, argc - 3, argv + 3);

  /* Compute the byte size of this asm code. */
  code_size = compute_code_size ();

  /* Figure out which bytes vary. */
  vary_mask = alloca (code_size);
  compute_vary_mask (vary_mask, code_size);

  if (!strcmp (argv[2], "-genasm"))
    generate_asm (vary_mask, code_size, sig, num_sigs, TRUE);
  else if (!strcmp (argv[2], "-genasm-skip-fixed"))
    generate_asm (vary_mask, code_size, sig, num_sigs, FALSE);
  else if (!strcmp (argv[2], "-print-size"))
    printf ("%d\n", code_size);

  return EXIT_SUCCESS;
}
