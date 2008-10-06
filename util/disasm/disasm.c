#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "disasm.h"


#define DISASM_CODE_START 0x10000

#define NOP_SIZE 2
#define NOP 0x4E71
#define NOPS_BETWEEN_SEGMENTS  4

#define NO_ADDRESS 0xFFFFFFFF


static char *patch_intersegment_calls (char *code, const codeseg_t *code0,
				       const codeseg_t *seg);
static codeseg_t *sort_segments (codeseg_t *seg);
static unsigned long jump_table_entry_to_address (long a5_offset,
						  const codeseg_t *code0,
						  const codeseg_t *seg);
static int create_asm (const char *filename, const unsigned char *mem, 
		       unsigned long num_bytes,
		       unsigned long starting_address);
static int create_gdb_commands (const char *filename, unsigned long num_bytes);
static char *cleanup_assembly (char *code);


#define GDB_TERMINATING_STRING "End of assembler dump.\n"
#define GDB_TERMINATING_STRING_LENGTH \
     (sizeof (GDB_TERMINATING_STRING) - 1)


/* Disassembles the specified block of memory and returns the disassembled
 * code.
 */
char *
disasm (const unsigned char *mem, unsigned long num_bytes,
	unsigned long starting_address)
{
  FILE *gdb_fp;
  char *buf;
  unsigned long size, max_size;
  int c;

  /* Create the assembly file containing the raw bytes. */
  if (create_asm ("/tmp/bytes.s", mem, num_bytes, starting_address))
    return NULL;

  /* Compile that into a bogus executable. */
  if (system ("gcc -b i486-linuxaout -g -c /tmp/bytes.s -o /tmp/bytes.o"))
    return NULL;

  /* Create a temporary set of commands for gdb to execute. */
  if (create_gdb_commands ("/tmp/gdbcmds", num_bytes))
    return NULL;

  /* Run gdb as the disassembler. */
  gdb_fp = popen ("gdb -nx -batch -cd /tmp -x gdbcmds bytes.o", "r");
  if (gdb_fp == NULL)
    {
      fprintf (stderr, "Problems executing gdb.\n");
      return NULL;
    }

  max_size = 1024;
  buf = (char *)malloc (max_size);

  /* Skip first informational line from gdb. */
  while ((c = getc (gdb_fp)) != EOF && c != '\n')
    ;

  /* Read in everything from gdb. */
  for (size = 0; (c = getc (gdb_fp)) != EOF; size++)
    {
      if (size >= max_size - 1)
	{
	  if (max_size >= 4 * 1024 * 1024)
	    max_size += 4 * 1024 * 1024;
	  else
	    max_size *= 2;
	  buf = (char *)realloc (buf, max_size);
	  if (buf == NULL)
	    {
	      fprintf (stderr, "Virtual memory exhausted!\n");
	      exit (-100);
	    }
	}
      
      buf[size] = c;
    }

  /* Close the gdb pipe. */
  pclose (gdb_fp);

  /* Strip off the useless message gdb prints out at the end. */
  if (size >= GDB_TERMINATING_STRING_LENGTH
      && !strncmp (buf + size - GDB_TERMINATING_STRING_LENGTH,
		   GDB_TERMINATING_STRING,
		   GDB_TERMINATING_STRING_LENGTH))
    size -= GDB_TERMINATING_STRING_LENGTH;

  /* Terminate the array and minimize the required memory. */
  buf[size] = '\0';
  buf = (char *)realloc (buf, size + 1);
  if (buf == NULL)
    {
      fprintf (stderr, "realloc problem!\n");
      exit (-102);
    }

  remove ("/tmp/disassembly");
  remove ("/tmp/gdbcmds");
  remove ("/tmp/bytes.s");
  remove ("/tmp/bytes");

  /* Verify that the code went where the wanted it to go. */
  if (size > 2)
    {
      unsigned long first_addr;
      if (buf[0] != '0' || buf[1] != 'x'
	  || sscanf (buf + 2, "%lx", &first_addr) != 1)
	{
	  int i;

	  fprintf (stderr, "Unable to verify first address!  First line:\n");
	  for (i = 0; buf[i] != '\n' && buf[i] != '\0'; i++)
	    putc (buf[i], stderr);
	  putc ('\n', stderr);

	  free (buf);
	  return NULL;
	}

      if (first_addr != starting_address)
	{
	  fprintf (stderr, "Internal error: wanted to start code at 0x%lX, "
		   "but started it at 0x%lX.\n", starting_address,
		   first_addr);
	  free (buf);
	  return NULL;
	}
    }

  return cleanup_assembly (buf);
}


/* Disassembles the specified linked list of code segments and returns
 * the disassembled code.
 */
char *
disasm_code_segments (codeseg_t *seg, unsigned long *entry_point)
{
  codeseg_t *code0, *c, *prev;
  unsigned char *all_code;
  unsigned long all_code_size, max_all_code_size;
  char *disassembled_code;

  /* Sort the code segments by segment number. */
  seg = sort_segments (seg);

  /* Default value for the entry point. */
  *entry_point = NO_ADDRESS;

  prev = NULL;
  for (code0 = seg; code0 != NULL && code0->segment != 0; code0 = code0->next)
    prev = code0;
  if (code0 == NULL)
    {
      fprintf (stderr, "Unable to find code resource 0!\n");
      return NULL;
    }

  /* Take code0 out of the chain. */
  if (prev == NULL)
    seg = code0->next;
  else
    prev->next = code0->next;

  /* Allocate enough space to hold all of the code. */
  for (c = seg, max_all_code_size = 0; c != NULL; c = c->next)
    max_all_code_size += c->num_code_bytes + NOPS_BETWEEN_SEGMENTS * NOP_SIZE;
  all_code = (unsigned char *)malloc (max_all_code_size);
  if (all_code == NULL)
    {
      fprintf (stderr, "malloc failed in disasm_code_segments, "
	       "requested %lu bytes.\n", max_all_code_size);
      exit (-105);
    }

  all_code_size = 0;
  for (c = seg; c != NULL; c = c->next)
    {
      int i;

      /* Don't disassemble code segment 0! */
      if (c->segment == 0)
	{
	  fprintf (stderr, "Multiple code 0 segments!\n");
	  continue;
	}
      
      /* Append this code to the big block of code. */
      c->disasm_start = DISASM_CODE_START + all_code_size;
      memcpy (all_code + all_code_size, c->code, c->num_code_bytes);
      all_code_size += c->num_code_bytes;

      /* Write out some NOPs between segments in case things get
       * out of sync because of data mistakenly disassembled as code.
       */
      for (i = 0; i < NOPS_BETWEEN_SEGMENTS; i++)
	{
	  all_code[all_code_size    ] = NOP >> 8;
	  all_code[all_code_size + 1] = NOP & 0xFF;
	  all_code_size += NOP_SIZE;
	}
    }

  if (all_code_size == 0)
    return NULL;

  /* Disassemble the code. */
  disassembled_code = disasm (all_code,
			      all_code_size - NOPS_BETWEEN_SEGMENTS * NOP_SIZE,
			      DISASM_CODE_START);

  if (disassembled_code == NULL)
    {
      fprintf (stderr, "Fatal error, disasm returned NULL, exiting.\n");
      exit (-103);
    }

  /* Go through and patch up intersegment jsr's so they point to the
   * actual target.
   */
  disassembled_code = patch_intersegment_calls (disassembled_code, code0, seg);

  /* Compute the address of the program's entry point. */
  *entry_point = jump_table_entry_to_address (read_long (&code0->code[12]) + 2,
					      code0, seg);

  return disassembled_code;
}


/* Eliminates extra cruft and canonicalizes the assembly. */
static char *
cleanup_assembly (char *code)
{
  char *new, *s, *d;
  char last = '\0';
  int in_angle_brackets_p;
  unsigned long len;

  len = strlen (code) + 1;
  new = (char *) malloc (len + 1000);
  if (new == NULL)
    {
      fprintf (stderr, "malloc failed in cleanup_assembly, "
	       "requested %lu bytes.\n", len);
      exit (-104);
    }

  in_angle_brackets_p = FALSE;
  for (s = code, d = new; *s != '\0'; s++)
    {
      int c = *s;

      if (c != '\n' && isspace (last) && isspace (c))
	continue;
      if (in_angle_brackets_p)
	{
	  if (c == '>')
	    in_angle_brackets_p = FALSE;
	}
      else if (c == '<')
	in_angle_brackets_p = TRUE;
      else if (c != ':' || s == code || s[-1] != '>')
	{
	  if (c == '\n' || c == ',')
	    while (d > new && isspace (d[-1]))
	      --d;
	  *d++ = last = c;
	}
    }
  *d = '\0';

  free (code);
  return (char *)realloc (new, strlen (new) + 1);
}


/* Creates an assembly file with the specified code bytes.  MEM and NUM_BYTES
 * specify the code bytes, STARTING_ADDRESS specifies the address at which
 * the code in the disassembly should begin.
 */
static int
create_asm (const char *filename, const unsigned char *mem,
	    unsigned long num_bytes, unsigned long starting_address)
{
  FILE *asm_fp;
  unsigned long n;

  /* Open the output file. */
  asm_fp = fopen (filename, "w");
  if (asm_fp == NULL)
    {
      perror (filename);
      return -1;
    }

#if 0
  fprintf (asm_fp,
	   ".text\n"
	   ".space 0x%lX\n", starting_address - 0x1078);
#warning "FIXME - this is a gross hack...must be a good way to set the PC"
#endif

  fprintf (asm_fp,
	   ".text\n"
	   ".set ., 0x%lX\n"
	   ".globl main\n"
	   "main:\n"	   /* For ELF */
	   ".globl _main\n"
	   "_main:\n",	   /* for a.out */
	   starting_address);

  for (n = 0; n < num_bytes; n += 8)
    {
      unsigned long i;

      fputs ("\t.byte ", asm_fp);
      for (i = 0; i < 8 && n + i < num_bytes; i++)
	fprintf (asm_fp, "%s0x%02X", (i != 0) ? "," : "",
		 (unsigned)mem[n + i]);
      putc ('\n', asm_fp);
    }

  /* Toss out a few NOPs so disassembly can't run off the end and die. */
  fprintf (asm_fp, "\t.byte 0x4E,0x71,0x4E,0x71,0x4E,0x71,0x4E,0x71\n");

  /* Dump out low globals.  Sadly gdb only uses these for branch targets. */
  fputs (".set _nilhandle,0x00\n"
	 ".globl _nilhandle\n"
	 ".set _trapvectors,0x80\n"
	 ".globl _trapvectors\n"
	 ".set _dodusesit,0xE4\n"
	 ".globl _dodusesit\n"
	 ".set _monkeylives,0x100\n"
	 ".globl _monkeylives\n"
	 ".set _ScrVRes,0x102\n"
	 ".globl _ScrVRes\n"
	 ".set _ScrHRes,0x104\n"
	 ".globl _ScrHRes\n"
	 ".set _ScreenRow,0x106\n"
	 ".globl _ScreenRow\n"
	 ".set _MemTop,0x108\n"
	 ".globl _MemTop\n"
	 ".set _BufPtr,0x10C\n"
	 ".globl _BufPtr\n"
	 ".set _HeapEnd,0x114\n"
	 ".globl _HeapEnd\n"
	 ".set _TheZone,0x118\n"
	 ".globl _TheZone\n"
	 ".set _UTableBase,0x11C\n"
	 ".globl _UTableBase\n"
	 ".set _loadtrap,0x12D\n"
	 ".globl _loadtrap\n"
	 ".set _CPUFlag,0x12F\n"
	 ".globl _CPUFlag\n"
	 ".set _ApplLimit,0x130\n"
	 ".globl _ApplLimit\n"
	 ".set _SysEvtMask,0x144\n"
	 ".globl _SysEvtMask\n"
	 ".set _EventQueue,0x14A\n"
	 ".globl _EventQueue\n"
	 ".set _RndSeed_L,0x156\n"
	 ".globl _RndSeed_L\n"
	 ".set _SysVersion,0x15A\n"
	 ".globl _SysVersion\n"
	 ".set _SEvtEnb,0x15C\n"
	 ".globl _SEvtEnb\n"
	 ".set _VBLQueue,0x160\n"
	 ".globl _VBLQueue\n"
	 ".set _Ticks,0x16A\n"
	 ".globl _Ticks\n"
	 ".set _KeyMap,0x174\n"
	 ".globl _KeyMap\n"
	 ".set _KeyThresh,0x18E\n"
	 ".globl _KeyThresh\n"
	 ".set _KeyRepThresh,0x190\n"
	 ".globl _KeyRepThresh\n"
	 ".set _Lvl1DT,0x192\n"
	 ".globl _Lvl1DT\n"
	 ".set _hyperlong,0x1AA\n"
	 ".globl _hyperlong\n"
	 ".set _Lvl2DT,0x1B2\n"
	 ".globl _Lvl2DT\n"
	 ".set _UnitNtryCnt,0x1D2\n"
	 ".globl _UnitNtryCnt\n"
	 ".set _VIA,0x1D4\n"
	 ".globl _VIA\n"
	 ".set _SCCRd,0x1D8\n"
	 ".globl _SCCRd\n"
	 ".set _SCCWr,0x1DC\n"
	 ".globl _SCCWr\n"
	 ".set _IWM,0x1E0\n"
	 ".globl _IWM\n"
	 ".set _Scratch20,0x1E4\n"
	 ".globl _Scratch20\n"
	 ".set _SPValid,0x1F8\n"
	 ".globl _SPValid\n"
	 ".set _SPATalkA,0x1F9\n"
	 ".globl _SPATalkA\n"
	 ".set _SPATalkB,0x1FA\n"
	 ".globl _SPATalkB\n"
	 ".set _SPConfig,0x1FB\n"
	 ".globl _SPConfig\n"
	 ".set _SPPortA,0x1FC\n"
	 ".globl _SPPortA\n"
	 ".set _SPPortB,0x1FE\n"
	 ".globl _SPPortB\n"
	 ".set _SPAlarm,0x200\n"
	 ".globl _SPAlarm\n"
	 ".set _SPFont,0x204\n"
	 ".globl _SPFont\n"
	 ".set _SPKbd,0x206\n"
	 ".globl _SPKbd\n"
	 ".set _SPPrint,0x207\n"
	 ".globl _SPPrint\n"
	 ".set _SPVolCtl,0x208\n"
	 ".globl _SPVolCtl\n"
	 ".set _SPClikCaret,0x209\n"
	 ".globl _SPClikCaret\n"
	 ".set _SPMisc2,0x20B\n"
	 ".globl _SPMisc2\n"
	 ".set _Time,0x20C\n"
	 ".globl _Time\n"
	 ".set _BootDrive,0x210\n"
	 ".globl _BootDrive\n"
	 ".set _SFSaveDisk,0x214\n"
	 ".globl _SFSaveDisk\n"
	 ".set _KbdLast,0x218\n"
	 ".globl _KbdLast\n"
	 ".set _KbdType,0x21E\n"
	 ".globl _KbdType\n"
	 ".set _MemErr,0x220\n"
	 ".globl _MemErr\n"
	 ".set _SdVolume,0x260\n"
	 ".globl _SdVolume\n"
	 ".set _SoundPtr,0x262\n"
	 ".globl _SoundPtr\n"
	 ".set _SoundBase,0x266\n"
	 ".globl _SoundBase\n"
	 ".set _SoundActive,0x27E\n"
	 ".globl _SoundActive\n"
	 ".set _SoundLevel,0x27F\n"
	 ".globl _SoundLevel\n"
	 ".set _CurPitch,0x280\n"
	 ".globl _CurPitch\n"
	 ".set _mathones,0x282\n"
	 ".globl _mathones\n"
	 ".set _ROM85,0x28E\n"
	 ".globl _ROM85\n"
	 ".set _PortBUse,0x291\n"
	 ".globl _PortBUse\n"
	 ".set _ScreenVars,0x292\n"
	 ".globl _ScreenVars\n"
	 ".set _Key1Trans,0x29E\n"
	 ".globl _Key1Trans\n"
	 ".set _Key2Trans,0x2A2\n"
	 ".globl _Key2Trans\n"
	 ".set _SysZone,0x2A6\n"
	 ".globl _SysZone\n"
	 ".set _ApplZone,0x2AA\n"
	 ".globl _ApplZone\n"
	 ".set _ROMBase,0x2AE\n"
	 ".globl _ROMBase\n"
	 ".set _RAMBase,0x2B2\n"
	 ".globl _RAMBase\n"
	 ".set _DSAlertTab,0x2BA\n"
	 ".globl _DSAlertTab\n"
	 ".set _ExtStsDT,0x2BE\n"
	 ".globl _ExtStsDT\n"
	 ".set _ABusVars,0x2D8\n"
	 ".globl _ABusVars\n"
	 ".set _FinderName,0x2E0\n"
	 ".globl _FinderName\n"
	 ".set _DoubleTime,0x2F0\n"
	 ".globl _DoubleTime\n"
	 ".set _CaretTime,0x2F4\n"
	 ".globl _CaretTime\n"
	 ".set _ScrDmpEnb,0x2F8\n"
	 ".globl _ScrDmpEnb\n"
	 ".set _BufTgFNum,0x2FC\n"
	 ".globl _BufTgFNum\n"
	 ".set _BufTgFFlg,0x300\n"
	 ".globl _BufTgFFlg\n"
	 ".set _BufTgFBkNum,0x302\n"
	 ".globl _BufTgFBkNum\n"
	 ".set _BufTgDate,0x304\n"
	 ".globl _BufTgDate\n"
	 ".set _DrvQHdr,0x308\n"
	 ".globl _DrvQHdr\n"
	 ".set _heapcheck,0x316\n"
	 ".globl _heapcheck\n"
	 ".set _Lo3Bytes,0x31A\n"
	 ".globl _Lo3Bytes\n"
	 ".set _MinStack,0x31E\n"
	 ".globl _MinStack\n"
	 ".set _DefltStack,0x322\n"
	 ".globl _DefltStack\n"
	 ".set _GZRootHnd,0x328\n"
	 ".globl _GZRootHnd\n"
	 ".set _EjectNotify,0x338\n"
	 ".globl _EjectNotify\n"
	 ".set _IAZNotify,0x33C\n"
	 ".globl _IAZNotify\n"
	 ".set _FCBSPtr,0x34E\n"
	 ".globl _FCBSPtr\n"
	 ".set _DefVCBPtr,0x352\n"
	 ".globl _DefVCBPtr\n"
	 ".set _VCBQHdr,0x356\n"
	 ".globl _VCBQHdr\n"
	 ".set _FSQHdr,0x360\n"
	 ".globl _FSQHdr\n"
	 ".set _WDCBsPtr,0x372\n"
	 ".globl _WDCBsPtr\n"
	 ".set _DefVRefNum,0x384\n"
	 ".globl _DefVRefNum\n"
	 ".set _CurDirStore,0x398\n"
	 ".globl _CurDirStore\n"
	 ".set _MCLKPCmiss1,0x3A0\n"
	 ".globl _MCLKPCmiss1\n"
	 ".set _MCLKPCmiss2,0x3A6\n"
	 ".globl _MCLKPCmiss2\n"
	 ".set _ToExtFS,0x3F2\n"
	 ".globl _ToExtFS\n"
	 ".set _FSFCBLen,0x3F6\n"
	 ".globl _FSFCBLen\n"
	 ".set _DSAlertRect,0x3F8\n"
	 ".globl _DSAlertRect\n"
	 ".set _JADBProc,0x6B8\n"
	 ".globl _JADBProc\n"
	 ".set _JFLUSH,0x6F4\n"
	 ".globl _JFLUSH\n"
	 ".set _JResUnknown1,0x700\n"
	 ".globl _JResUnknown1\n"
	 ".set _JResUnknown2,0x714\n"
	 ".globl _JResUnknown2\n"
	 ".set _JHideCursor,0x800\n"
	 ".globl _JHideCursor\n"
	 ".set _JShowCursor,0x804\n"
	 ".globl _JShowCursor\n"
	 ".set _JShieldCursor,0x808\n"
	 ".globl _JShieldCursor\n"
	 ".set _JScrnAddr,0x80C\n"
	 ".globl _JScrnAddr\n"
	 ".set _JScrnSize,0x810\n"
	 ".globl _JScrnSize\n"
	 ".set _JInitCrsr,0x814\n"
	 ".globl _JInitCrsr\n"
	 ".set _JSetCrsr,0x818\n"
	 ".globl _JSetCrsr\n"
	 ".set _JCrsrObscure,0x81C\n"
	 ".globl _JCrsrObscure\n"
	 ".set _JUpdateProc,0x820\n"
	 ".globl _JUpdateProc\n"
	 ".set _ScrnBase,0x824\n"
	 ".globl _ScrnBase\n"
	 ".set _MouseLocation,0x82C\n"
	 ".globl _MouseLocation\n"
	 ".set _CrsrPin,0x834\n"
	 ".globl _CrsrPin\n"
	 ".set _MainDevice,0x8A4\n"
	 ".globl _MainDevice\n"
	 ".set _DeviceList,0x8A8\n"
	 ".globl _DeviceList\n"
	 ".set _QDColors,0x8B0\n"
	 ".globl _QDColors\n"
	 ".set _CrsrVis,0x8CC\n"
	 ".globl _CrsrVis\n"
	 ".set _CrsrState,0x8D0\n"
	 ".globl _CrsrState\n"
	 ".set _mousemask,0x8D6\n"
	 ".globl _mousemask\n"
	 ".set _mouseoffset,0x8DA\n"
	 ".globl _mouseoffset\n"
	 ".set _JournalFlag,0x8DE\n"
	 ".globl _JournalFlag\n"
	 ".set _JSwapFont,0x8E0\n"
	 ".globl _JSwapFont\n"
	 ".set _WidthListHand,0x8E4\n"
	 ".globl _WidthListHand\n"
	 ".set _JournalRef,0x8E8\n"
	 ".globl _JournalRef\n"
	 ".set _CrsrThresh,0x8EC\n"
	 ".globl _CrsrThresh\n"
	 ".set _WWExist,0x8F2\n"
	 ".globl _WWExist\n"
	 ".set _QDExist,0x8F3\n"
	 ".globl _QDExist\n"
	 ".set _JFetch,0x8F4\n"
	 ".globl _JFetch\n"
	 ".set _JStash,0x8F8\n"
	 ".globl _JStash\n"
	 ".set _JIODone,0x8FC\n"
	 ".globl _JIODone\n"
	 ".set _CurApRefNum,0x900\n"
	 ".globl _CurApRefNum\n"
	 ".set _CurrentA5,0x904\n"
	 ".globl _CurrentA5\n"
	 ".set _CurStackBase,0x908\n"
	 ".globl _CurStackBase\n"
	 ".set _CurApName,0x910\n"
	 ".globl _CurApName\n"
	 ".set _CurJTOffset,0x934\n"
	 ".globl _CurJTOffset\n"
	 ".set _CurPageOption,0x936\n"
	 ".globl _CurPageOption\n"
	 ".set _HiliteMode,0x938\n"
	 ".globl _HiliteMode\n"
	 ".set _PrintErr,0x944\n"
	 ".globl _PrintErr\n"
	 ".set _graphlooksat,0x952\n"
	 ".globl _graphlooksat\n"
	 ".set _macwritespace,0x954\n"
	 ".globl _macwritespace\n"
	 ".set _ScrapSize,0x960\n"
	 ".globl _ScrapSize\n"
	 ".set _ScrapHandle,0x964\n"
	 ".globl _ScrapHandle\n"
	 ".set _ScrapCount,0x968\n"
	 ".globl _ScrapCount\n"
	 ".set _ScrapState,0x96A\n"
	 ".globl _ScrapState\n"
	 ".set _ScrapName,0x96C\n"
	 ".globl _ScrapName\n"
	 ".set _ROMFont0,0x980\n"
	 ".globl _ROMFont0\n"
	 ".set _ApFontID,0x984\n"
	 ".globl _ApFontID\n"
	 ".set _ROMlib_myfmi,0x988\n"
	 ".globl _ROMlib_myfmi\n"
	 ".set _ROMlib_fmo,0x998\n"
	 ".globl _ROMlib_fmo\n"
	 ".set _ToolScratch,0x9CE\n"
	 ".globl _ToolScratch\n"
	 ".set _WindowList,0x9D6\n"
	 ".globl _WindowList\n"
	 ".set _SaveUpdate,0x9DA\n"
	 ".globl _SaveUpdate\n"
	 ".set _PaintWhite,0x9DC\n"
	 ".globl _PaintWhite\n"
	 ".set _WMgrPort,0x9DE\n"
	 ".globl _WMgrPort\n"
	 ".set _OldStructure,0x9E6\n"
	 ".globl _OldStructure\n"
	 ".set _OldContent,0x9EA\n"
	 ".globl _OldContent\n"
	 ".set _GrayRgn,0x9EE\n"
	 ".globl _GrayRgn\n"
	 ".set _SaveVisRgn,0x9F2\n"
	 ".globl _SaveVisRgn\n"
	 ".set _DragHook,0x9F6\n"
	 ".globl _DragHook\n"
	 ".set _Scratch8,0x9FA\n"
	 ".globl _Scratch8\n"
	 ".set _OneOne,0xA02\n"
	 ".globl _OneOne\n"
	 ".set _MinusOne,0xA06\n"
	 ".globl _MinusOne\n"
	 ".set _TopMenuItem,0xA0A\n"
	 ".globl _TopMenuItem\n"
	 ".set _AtMenuBottom,0xA0C\n"
	 ".globl _AtMenuBottom\n"
	 ".set _MenuList,0xA1C\n"
	 ".globl _MenuList\n"
	 ".set _MBarEnable,0xA20\n"
	 ".globl _MBarEnable\n"
	 ".set _MenuFlash,0xA24\n"
	 ".globl _MenuFlash\n"
	 ".set _TheMenu,0xA26\n"
	 ".globl _TheMenu\n"
	 ".set _MBarHook,0xA2C\n"
	 ".globl _MBarHook\n"
	 ".set _MenuHook,0xA30\n"
	 ".globl _MenuHook\n"
	 ".set _DragPattern,0xA34\n"
	 ".globl _DragPattern\n"
	 ".set _DeskPattern,0xA3C\n"
	 ".globl _DeskPattern\n"
	 ".set _fpstate,0xA4A\n"
	 ".globl _fpstate\n"
	 ".set _TopMapHndl,0xA50\n"
	 ".globl _TopMapHndl\n"
	 ".set _SysMapHndl,0xA54\n"
	 ".globl _SysMapHndl\n"
	 ".set _SysMap,0xA58\n"
	 ".globl _SysMap\n"
	 ".set _CurMap,0xA5A\n"
	 ".globl _CurMap\n"
	 ".set _resreadonly,0xA5C\n"
	 ".globl _resreadonly\n"
	 ".set _ResLoad,0xA5E\n"
	 ".globl _ResLoad\n"
	 ".set _ResErr,0xA60\n"
	 ".globl _ResErr\n"
	 ".set _FScaleDisable,0xA63\n"
	 ".globl _FScaleDisable\n"
	 ".set _CurActivate,0xA64\n"
	 ".globl _CurActivate\n"
	 ".set _CurDeactive,0xA68\n"
	 ".globl _CurDeactive\n"
	 ".set _DeskHook,0xA6C\n"
	 ".globl _DeskHook\n"
	 ".set _TEDoText,0xA70\n"
	 ".globl _TEDoText\n"
	 ".set _TERecal,0xA74\n"
	 ".globl _TERecal\n"
	 ".set _ApplScratch,0xA78\n"
	 ".globl _ApplScratch\n"
	 ".set _GhostWindow,0xA84\n"
	 ".globl _GhostWindow\n"
	 ".set _ResumeProc,0xA8C\n"
	 ".globl _ResumeProc\n"
	 ".set _ANumber,0xA98\n"
	 ".globl _ANumber\n"
	 ".set _ACount,0xA9A\n"
	 ".globl _ACount\n"
	 ".set _DABeeper,0xA9C\n"
	 ".globl _DABeeper\n"
	 ".set _DAStrings,0xAA0\n"
	 ".globl _DAStrings\n"
	 ".set _TEScrpLength,0xAB0\n"
	 ".globl _TEScrpLength\n"
	 ".set _TEScrpHandle,0xAB4\n"
	 ".globl _TEScrpHandle\n"
	 ".set _AppPacks,0xAB8\n"
	 ".globl _AppPacks\n"
	 ".set _SysResName,0xAD8\n"
	 ".globl _SysResName\n"
	 ".set _AppParmHandle,0xAEC\n"
	 ".globl _AppParmHandle\n"
	 ".set _DSErrCode,0xAF0\n"
	 ".globl _DSErrCode\n"
	 ".set _ResErrProc,0xAF2\n"
	 ".globl _ResErrProc\n"
	 ".set _DlgFont,0xAFA\n"
	 ".globl _DlgFont\n"
	 ".set _WidthPtr,0xB10\n"
	 ".globl _WidthPtr\n"
	 ".set _SCSIFlags,0xB22\n"
	 ".globl _SCSIFlags\n"
	 ".set _WidthTabHandle,0xB2A\n"
	 ".globl _WidthTabHandle\n"
	 ".set _hyperwritesto,0xB4C\n"
	 ".globl _hyperwritesto\n"
	 ".set _MenuDisable,0xB54\n"
	 ".globl _MenuDisable\n"
	 ".set _MBDFHndl,0xB58\n"
	 ".globl _MBDFHndl\n"
	 ".set _MBSaveLoc,0xB5C\n"
	 ".globl _MBSaveLoc\n"
	 ".set _RomMapInsert,0xB9E\n"
	 ".globl _RomMapInsert\n"
	 ".set _TmpResLoad,0xB9F\n"
	 ".globl _TmpResLoad\n"
	 ".set _IntlSpec,0xBA0\n"
	 ".globl _IntlSpec\n"
	 ".set _SysFontFam,0xBA6\n"
	 ".globl _SysFontFam\n"
	 ".set _SysFontSiz,0xBA8\n"
	 ".globl _SysFontSiz\n"
	 ".set _MBarHeight,0xBAA\n"
	 ".globl _MBarHeight\n"
	 ".set _TESysJust,0xBAC\n"
	 ".globl _TESysJust\n"
	 ".set _LastFOND,0xBC2\n"
	 ".globl _LastFOND\n"
	 ".set _fondid,0xBC6\n"
	 ".globl _fondid\n"
	 ".set _FractEnable,0xBF4\n"
	 ".globl _FractEnable\n"
	 ".set _MMU32Bit,0xCB2\n"
	 ".globl _MMU32Bit\n"
	 ".set _TheGDevice,0xCC8\n"
	 ".globl _TheGDevice\n"
	 ".set _AuxWinHead,0xCD0\n"
	 ".globl _AuxWinHead\n"
	 ".set _AuxCtlHead,0xCD4\n"
	 ".globl _AuxCtlHead\n"
	 ".set _TimeDBRA,0xD00\n"
	 ".globl _TimeDBRA\n"
	 ".set _TimeSCCDB,0xD02\n"
	 ".globl _TimeSCCDB\n"
	 ".set _JVBLTask,0xD28\n"
	 ".globl _JVBLTask\n"
	 ".set _WMgrCPort,0xD2C\n"
	 ".globl _WMgrCPort\n"
	 ".set _SynListHandle,0xD32\n"
	 ".globl _SynListHandle\n"
	 ".set _MenuCInfo,0xD50\n"
	 ".globl _MenuCInfo\n"
	 ".set _DTQueue,0xD92\n"
	 ".globl _DTQueue\n"
	 ".set _JDTInstall,0xD9C\n"
	 ".globl _JDTInstall\n"
	 ".set _HiliteRGB,0xDA0\n"
	 ".globl _HiliteRGB\n"
	 ".set _TimeSCSIDB,0xDA6\n"
	 ".globl _TimeSCSIDB\n",
	 asm_fp);

  /* Close the output file. */
  if (fclose (asm_fp))
    {
      perror (filename);
      return -2;
    }

  return 0;
}


static int
create_gdb_commands (const char *filename, unsigned long num_bytes)
{
  FILE *cmd_fp;

  cmd_fp = fopen (filename, "w");
  if (cmd_fp == NULL)
    {
      perror (filename);
      return -1;
    }

  fprintf (cmd_fp,
	   "set m68k on\n"
	   "disassemble main main+%lu\n"
	   "quit\n",
	   num_bytes);

  if (fclose (cmd_fp))
    {
      perror (filename);
      return -2;
    }

  return 0;
}


static unsigned long
jump_table_entry_to_address (long a5_offset, const codeseg_t *code0,
			     const codeseg_t *seg)
{
  unsigned long jmp_table_length;
  long jmp_table_offset, jmp_table_byte_index;
  short segment_offset, segment_number;
  unsigned char *p;
  const codeseg_t *c;

  /* Extract two useful fields from the code0 resource. */
  jmp_table_length = read_long (&code0->code[8]);
  jmp_table_offset = read_long (&code0->code[12]);

  if (jmp_table_offset != 32)
    {
      fprintf (stderr, "Warning: expected 32 for jump table offset, "
	       "got %lu.\n", jmp_table_offset);
    }

  /* Figure out how many bytes into the jmp table to go. */
  jmp_table_byte_index = a5_offset - jmp_table_offset;

  /* If this isn't a legal entry, bail. */
  if ((jmp_table_byte_index % 8) != 2
      || jmp_table_byte_index >= jmp_table_length)
    return NO_ADDRESS;

  /* Grab the segment number and offset for the target.  The target
   * is the actual push instruction, two bytes into the 8 byte
   * stub.
   */
  p = &code0->code[16 + jmp_table_byte_index];
  segment_offset = read_short (p - 2);
  segment_number = read_short (p + 2);

  /* Look for a matching segment if we find one, return the
   * address of the target code.
   */
  for (c = seg; c != NULL; c = c->next)
    {
      if (c->segment == segment_number)
	return c->disasm_start + segment_offset + 4;
    }

  return NO_ADDRESS;
}


/* Extracts the specified field from a line, copying it into buf.  Returns
 * buf if successful, NULL if there weren't that many fields on that line.
 */
char *
extract_field (const char *c, int field_num, char *buf)
{
  char *d;

  buf[0] = '\0';
  while (field_num > 0)
    {
      if (isspace (*c))
	{
	  /* Move on to the next field. */
	  while (isspace (*c) && *c != '\n')
	    c++;
	  if (*c == '\n' || *c == '\0')
	    return NULL;
	  --field_num;
	}
      else
	c++;
    }

  /* Copy the specified field out to buf. */
  for (d = buf; !isspace (*c) && *c != '\0'; c++, d++)
    *d = *c;
  *d = '\0';

  return buf;
}


static char *
patch_intersegment_calls (char *code, const codeseg_t *code0,
			  const codeseg_t *seg)
{
  char *new, *s, *d;
  unsigned long len;

  /* Allocate plenty of space for the new code. */
  len = 100 + strlen (code) * 2;
  new = (char *) malloc (len);
  if (new == NULL)
    {
      fprintf (stderr, "malloc failed in patch_intersegment_calls, "
	       "%lu bytes requested.\n", len);
      exit (-106);
    }

  for (s = code, d = new; *s != '\0'; )
    {
      char opcode[1024], operand[1024], junk[1024];
      int handled_p = FALSE;

      if (extract_field (s, 1, opcode)
	  && (!strcmp (opcode, "jsr")
	      || !strcmp (opcode, "jmp"))
	  && extract_field (s, 2, operand)
	  && !strncmp (operand, "a5@(", 4)
	  && isdigit (operand[4])
	  && !extract_field (s, 3, junk))
	{
	  int i;

	  for (i = 4; operand[i] != ')' && operand[i]; i++)
	    if (!isdigit (operand[i]))
	      break;

	  if (operand[i] == ')' && operand[i + 1] == '\0')
	    {
	      unsigned long new_addr;
	      new_addr = jump_table_entry_to_address (atoi (operand + 4),
						      code0, seg);
	      if (new_addr != NO_ADDRESS)
		{
		  char addr[1024];
		  sprintf (d, "%s %s 0x%lx", extract_field (s, 0, addr),
			   opcode, new_addr);
		  d += strlen (d);
		  while (*s && *s != '\n')
		    s++;
		  handled_p = TRUE;
		}
	    }
	}

      if (!handled_p)
	{
	  /* Copy this line from in to out. */
	  while (*s && *s != '\n')
	    *d++ = *s++;
	}

      if (*s == '\n')
	*d++ = *s++;
    }

  *d = '\0';
  free (code);
  return (char *)realloc (new, strlen (new) + 1);
}


/* qsort helper function. */
static int
compare_segs (const void *p1, const void *p2)
{
  return ((*(const codeseg_t **)p1)->segment
	  - (*(const codeseg_t **)p2)->segment);
}


/* Sorts a linked list of code segments by segment number, lowest first. */
static codeseg_t *
sort_segments (codeseg_t *seg)
{
  codeseg_t **array, *c, *next;
  int num_segs, s;

  if (seg == NULL)
    return NULL;

  for (num_segs = 0, c = seg; c != NULL; c = c->next)
    num_segs++;
  array = (codeseg_t **)alloca (num_segs * sizeof array[0]);
  for (s = 0, c = seg; c != NULL; c = c->next, s++)
    array[s] = c;
  
  qsort (array, num_segs, sizeof array[0], compare_segs);

  for (s = num_segs - 1, next = NULL; s >= 0; s--)
    {
      array[s]->next = next;
      next = array[s];
    }

  return next;
}
