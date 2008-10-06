#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "disasm.h"
#include "label.h"
#include "hash.h"


static const struct
{
  const char *orig_name;
  const char *canon_name;
} branches[] =
{
  { "bhi", "bhi" },
  { "bls", "bls" },
  { "bcc", "bcc" },
  { "bcs", "bcs" },
  { "bne", "bne" },
  { "beq", "beq" },
  { "bvc", "bvc" },
  { "bvs", "bvs" },
  { "bpl", "bpl" },
  { "bmi", "bmi" },
  { "bge", "bge" },
  { "blt", "blt" },
  { "bgt", "bgt" },
  { "ble", "ble" },
  { "bsr", "bsr" },
  { "bra", "bra" },
  { "jmp", "jmp" },
  { "jsr", "jsr" },

  { "bhis", "bhi" },
  { "blss", "bls" },
  { "bccs", "bcc" },
  { "bcss", "bcs" },
  { "bnes", "bne" },
  { "beqs", "beq" },
  { "bvcs", "bvc" },
  { "bvss", "bvs" },
  { "bpls", "bpl" },
  { "bmis", "bmi" },
  { "bges", "bge" },
  { "blts", "blt" },
  { "bgts", "bgt" },
  { "bles", "ble" },
  { "bsrs", "bsr" },
  { "bras", "bra" },
  { "jmps", "jmp" },
  { "jsrs", "jsr" },

  { "bhil", "bhi" },
  { "blsl", "bls" },
  { "bccl", "bcc" },
  { "bcsl", "bcs" },
  { "bnel", "bne" },
  { "beql", "beq" },
  { "bvcl", "bvc" },
  { "bvsl", "bvs" },
  { "bpll", "bpl" },
  { "bmil", "bmi" },
  { "bgel", "bge" },
  { "bltl", "blt" },
  { "bgtl", "bgt" },
  { "blel", "ble" },
  { "bsrl", "bsr" },
  { "bral", "bra" },
  { "jmpl", "jmp" },
  { "jsrl", "jsr" },

  { "dbhi", "dbhi" },
  { "dbls", "dbls" },
  { "dbcc", "dbcc" },
  { "dbcs", "dbcs" },
  { "dbne", "dbne" },
  { "dbeq", "dbeq" },
  { "dbvc", "dbvc" },
  { "dbvs", "dbvs" },
  { "dbpl", "dbpl" },
  { "dbmi", "dbmi" },
  { "dbge", "dbge" },
  { "dblt", "dblt" },
  { "dbgt", "dbgt" },
  { "dble", "dble" },
  { "dbra", "dbra" },
  { "dbf",  "dbra" },
  { "dbt",  "dbt"  },
};


/* Moves on to the start of the next line. */
static char *
next_line (char *s)
{
  while (*s && *s != '\n')
    s++;
  if (*s == '\n')
    s++;
  return s;
}


static char *
extract_branch_target (const char *opcode, char *operand)
{
  char *start;
  int i;

  /* Skip leading @# that gdb throws in for PC relative jsr's. */
  if (!strcmp (opcode, "jsr")
      && operand[0] == '@'
      && operand[1] == '#')
    operand += 2;

  if (operand[0] == '0' && operand[1] == 'x')
    start = operand + 2;
  else if (operand[0] == 'd' && operand[1] >= '0' && operand[1] <= '7'
	   && operand[2] == ',' && operand[3] == '0' && operand[4] == 'x')
    start = operand + 5;
  else
    return NULL;

  for (i = 0; start[i] != '\0'; i++)
    if (!isxdigit (start[i]))
      return NULL;
  return start - 2;
}


static int
call_p (const char *opcode)
{
  return (!strncmp (opcode, "bsr", 3)
	  || !strncmp (opcode, "jsr", 3));
}
       

char *
add_labels (char *code, unsigned long entry_point, int keep_addrs_p)
{
  static hash_table_t *branch_hash = NULL;
  char *s, *d, *new, *final, buf[32];
  hash_table_t *label_hash;
  unsigned long label_num;
  char addr[1024], opcode[1024], operand[1024], junk[1024];
  const char *label;

  if (code == NULL)
    return NULL;

  /* Set up the branch hash table (if we haven't already). */
  if (branch_hash == NULL)
    {
      int i;
      branch_hash = hash_new ();
      for (i = 0; i < sizeof branches / sizeof branches[0]; i++)
	hash_insert (branch_hash, branches[i].orig_name,
		     branches[i].canon_name);
    }


  /* Phase 1: identify all branch targets. */
  label_hash = hash_new ();
  sprintf (buf, "0x%lx", entry_point);
  hash_insert (label_hash, buf, "_main");
  for (s = code; *s != '\0'; s = next_line (s))
    {
      if (extract_field (s, 1, opcode)
	  && hash_lookup (branch_hash, opcode)
	  && extract_field (s, 2, operand)
	  && extract_branch_target (opcode, operand)
	  && !extract_field (s, 3, junk))
	{
	  hash_insert (label_hash, extract_branch_target (opcode, operand),
		       call_p (opcode) ? "F" : "L");
	}
    }

  /* Phase 2: assign sorted, contiguous label names. */
  for (s = code, label_num = 0; *s != '\0'; s = next_line (s))
    {
      if (extract_field (s, 0, addr))
	{
	  const char *s = hash_lookup (label_hash, addr);
	  if (s != NULL && s[1] == '\0')  /* Don't overwrite "_main". */
	    {
	      sprintf (buf, "%c%lu_", s[0], label_num++);
	      hash_remove (label_hash, addr);
	      hash_insert (label_hash, addr, buf);
	    }
	}
    }

  /* Phase 3: change all branch targets to reflect the new labels,
   *          replace all branch instructions with their canonical
   *          opcode name, and copy the input to the output.
   */
  new = (char *)malloc (100 + strlen (code) * 2);
  for (s = code, d = new; *s != '\0'; )
    {
      int changed_p;

      changed_p = FALSE;
      if (extract_field (s, 1, opcode)
	  && hash_lookup (branch_hash, opcode)
	  && extract_field (s, 2, operand)
	  && extract_branch_target (opcode, operand)
	  && !extract_field (s, 3, junk))
	{
	  label = hash_lookup (label_hash,
			       extract_branch_target (opcode, operand));
	  if (label != NULL && label[0] != '\0' && label[1] != '\0')
	    {
	      if (!extract_field (s, 0, addr))
		abort ();
	      sprintf (d, "%s %s ", addr, opcode);
	      d += strlen (d);

	      /* If it's a dbra, etc. then copy the data register specifier. */
	      if (operand[0] == 'd')
		{
		  strncpy (d, operand, 3);
		  d += 3;
		}

	      /* Replace the raw address with the target label. */
	      strcpy (d, label);
	      d += strlen (d);
	      *d++ = '\n';

	      s = next_line (s);
	      changed_p = TRUE;
	    }
	}

      /* If we didn't rewrite the branch target above, just copy the line. */
      if (!changed_p)
	{
	  while (*s != '\n' && *s != '\0')
	    *d++ = *s++;
	  if (*s == '\n')
	    *d++ = *s++;
	}
    }
  *d = '\0';

  /* Phase 4: print out every line with leading labels and canonicalize
   *          all opcodes.
   */
  free (code);
  final = (char *)malloc (100 + strlen (new) * 2);
  for (s = new, d = final; *s != '\0'; s = next_line (s))
    {
      int f;

      if (!extract_field (s, 0, addr))
	abort ();

      /* Separate call targets with blank lines. */
      label = hash_lookup (label_hash, addr);
      if (label != NULL && label[0] == 'F')
	*d++ = '\n';

      if (keep_addrs_p)
	{
	  sprintf (d, "%s\t", addr);
	  d += strlen (d);
	}

      /* Print out any label for this line. */
      if (label != NULL)
	{
	  sprintf (d, "%s:", label);
	  d += strlen (d);
	}

      /* Print out the opcode. */
      if (extract_field (s, 1, opcode))
	{
	  const char *canon_opcode;
	  canon_opcode = hash_lookup (branch_hash, opcode);
	  sprintf (d, "\t%s", canon_opcode ? canon_opcode : opcode);
	  d += strlen (d);
	}

      /* Dump out the rest of the operands, separated by spaces. */
      for (f = 2; extract_field (s, f, operand); f++)
	{
	  sprintf (d, "%s%s", (f == 2) ? "\t" : " ", operand);
	  d += strlen (d);
	}

      *d++ = '\n';
    }
  *d = '\0';

  free (new);
  hash_free (label_hash);
  return (char *)realloc (final, strlen (final) + 1);
}
