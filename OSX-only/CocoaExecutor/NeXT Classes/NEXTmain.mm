#include "rsys/common.h"
#include "MacTypes.h"

#include <libc.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#import <Cocoa/Cocoa.h>
#include "rsys/keyboards.h"
#include "MemoryMgr.h"

//#import <kernserv/kern_loader.h>
//#import <kernserv/kern_loader_error.h>

#include <mach/mach_error.h>
#include <mach/mach.h>

#import "MacAppClass.h"
#include "rsys/next.h"
#include "rsys/setuid.h"
#include "rsys/blockinterrupts.h"
#include <syn68k_public.h>
#include "ourstuff.h"
#include "contextswitch.h"

#warning "punt this #include"
#include "rsys/memory_layout.h"

using namespace Executor;

char *Executor::romlib_sp;
char *Executor::nextstep_sp;

char ROMlib_040;

extern "C" void ROMlib_dummywincall( void );

void ROMlib_calldummies( void )
{
    SETUPA5;

    ROMlib_dummywincall();
    RESTOREA5;
}

void ROMlib_determine040ness( void )
{
    kern_return_t ret;
    struct host_basic_info basic_info;
    unsigned int count;

    count=HOST_BASIC_INFO_COUNT;
    ret = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&basic_info,
								       &count);
    if (ret != KERN_SUCCESS)
	mach_error("host_info() call failed", ret);
    else
	ROMlib_040 = basic_info.cpu_type    == CPU_TYPE_MC680x0 &&
		     basic_info.cpu_subtype == CPU_SUBTYPE_MC68040;
}


#define NS_2_0_VERSION  1
#define NS_3_0_VERSION  2

void ROMlib_checkadb( void )
{
    extern int ROMlib_keyboardisadb( void );

    SETUPA5;
    ROMlib_keyboard_type = ROMlib_get_keyboard_type();
    RESTOREA5;
}

/*
 * NOTE:  Use of SETUPA5 and RESTOREA5 is to be done very carefully
 *	  in routines that call contextswitch.  The potential problem
 *	  is that you save a5, then context switch, where the Mac side
 *	  of things modifies a5 and then you restore a5 afterward...
 */

void Executor::contextswitch(char **from_spp, char **to_spp)
{
    static virtual_int_state_t block;
    static char blockinitted = FALSE;

    if (from_spp == &romlib_sp) {
	block = block_virtual_ints();
	blockinitted = TRUE;
    } else
	if (blockinitted)
	    restore_virtual_ints (block);

/*
 * ASM NOTE:  We need to either have this available in 80486 assembly,
 *	      or we need to use some thread routines.
 */

#if defined(mc68000)
    asm("movel #cont, sp@-"
	"\n\tmoveml d2-d7/a2-a6, sp@-"
	"\n\tfmovem fp0-fp7, sp@-"
	"\n\tfmovem fpcr/fpsr, sp@-"
	"\n\tmovel sp, %0"
	"\n\tmovel %1, sp"
	"\n\tfmovem sp@+, fpcr/fpsr"
	"\n\tfmovem sp@+, fp0-fp7"
	"\n\tmoveml sp@+, d2-d7/a2-a6"
	"\n\trts"
	"\n\tcont:" : "=m" (*from_spp) : "m" (*to_spp));
#warning This code is too processor-specific!
#elif defined(i386) || defined(i486)
    asm("pushl $cont\n\t"
	"pushal\n\t"
	"pushfl\n\t"
	"subl $108,%%esp\n\t"
	"fnsave (%%esp)\n\t"
	"push %%ss\n\t"
/*	"push %%cs\n\t"	*/
	"push %%ds\n\t"
	"push %%es\n\t"
	"push %%fs\n\t"
	"push %%gs\n\t"
	"movl	%%esp, %0\n\t"
	"movl	%1, %%esp\n\t"
	"pop %%gs\n\t"
	"pop %%fs\n\t"
	"pop %%es\n\t"
	"pop %%ds\n\t"
/*	"pop %%cs\n\t"	*/
	"pop %%ss\n\t"
	"frstor (%%esp)\n\t"
	"addl $108,%%esp\n\t"
	"popfl\n\t"
	"popal\n\t"
	"ret\n\t"
	"cont:" : "=m" (*from_spp) : "m" (*to_spp));
#else
#error This will not compile
#endif
}

extern int ExecutorArgc;
extern char **ExecutorArgv;

extern "C" void calloldmain();

void calloldmain()
{
    oldmain(ExecutorArgc, ExecutorArgv);
    exit(-1);	/* shouldn't ever get here */
}

#if defined(BINCOMPAT) && !defined(SYN68K)

#define ARDIMODS	"/ardimods/ardimods_reloc"

#include <signal.h>

#include <mach/mach.h>

#include <kernserv/kern_loader.h>
#include <stdio.h>
#include <stdlib.h>

void ROMlib_load_ardi_mods( void )
{
    port_t loader_port;
    port_t kernel_task;
    kern_return_t r;
    extern char ROMlib_startdir[];
    extern short ROMlib_startdirlen;
    char *ardimods;
    extern void mustbesetuid( void );

    SETUPA5;
    ardimods = alloca( ROMlib_startdirlen + sizeof(ARDIMODS) - 1);
    bcopy(ROMlib_startdir, ardimods, ROMlib_startdirlen - 1);
    bcopy(ARDIMODS, ardimods + ROMlib_startdirlen - 1, sizeof(ARDIMODS));
    if (geteuid()) {
	mustbesetuid();
	fprintf(stderr,
	       "This program must be setuid-root in order to load ardimods\n");
	exit(1);
    }
    /* Get kern_loader's port. */
    r = kern_loader_look_up(&loader_port);
    if (r != KERN_SUCCESS) {
	kern_loader_error("Couldn't get loader_port to load ardi mods", r);
	exit(1);
    }

    /* Get the kernel's task port. */
    r = task_by_unix_pid(task_self(), 0, &kernel_task);
    if (r != KERN_SUCCESS) {
	kern_loader_error("Couldn't get kernel_task to load ardi mods", r);
	exit(1);
    }

    kern_loader_delete_server(loader_port, kernel_task, "ardimods");


    /* Add the server. */
    r = kern_loader_add_server(loader_port, kernel_task, ardimods);
    if (r != KERN_SUCCESS) {
	kern_loader_error("Call to kern_loader_abort failed "
					       "(ardi mods won't load)", r);
	exit(1);
    }
    RESTOREA5;
}

void ROMlib_install_ardi_mods(void)
{
    void *oldfp;

    oldfp = signal(SIGILL, (void *) ROMlib_load_ardi_mods);

/*
 * ASM NOTE: In order to merge ROMlib-b and ROMlib-g, we really should
 *	     have a flag somewhere that says whether or not we need the
 *	     kernel mods.
 */
    *(long *) 0x54 = 0;		/* prevent page faults */
    asm("movew sr, d0" : : : "d0"); /* will generate SIGILL if ardimods are */
				/* not installed */
    signal(SIGILL, oldfp);
}
#endif


/* We allocate some "low" memory, whose addresses don't have any bits
 * set in the high byte.  We'll try to use this memory for applzone,
 * etc.  when possible.
 */
static char *low_memory_start, *low_memory_end;


/* This doles out the "low" memory allocated at startup time. */
void *
mmap_permanent_memory (unsigned long mem)
{
  void *retval;

  if (low_memory_start + mem > low_memory_end)
    retval = NULL;
  else
    {
      retval = low_memory_start;
      low_memory_start += ((mem + 7) & ~7);  /* keep divisible by 8. */
    }

  return retval;
}

void Executor::nextmain(void)
{
  long ourstackstart;
  kern_return_t err;
  vm_address_t addr_we_want;
  
  addr_we_want = SYS_ZONE_START;
  low_memory_start = (char *) addr_we_want;
  low_memory_end = low_memory_start + SYS_ZONE_SIZE + APPL_ZONE_SIZE;
  
  err = vm_allocate (mach_task_self(), &addr_we_want,
                     (SYS_ZONE_SIZE + APPL_ZONE_SIZE
                      + MAC_STACK_SIZE + MAGIC_SIZE),
                     0);
  if (err != KERN_SUCCESS)
  {
    fprintf(stderr, "vm_allocate returned value of %s\n", mach_error_string(err));
    exit(1);
  }
  
#if defined(SYN68K)
  ourstackstart = NATIVE_STACK_START + NATIVE_STACK_SIZE;
#else
  ourstackstart = MAC_STACK_START + MAC_STACK_SIZE;
#endif
  
  /*
   * ASM NOTE: see the note in in contextswitch
   */
  
#if defined(mc68000)
  asm("    movel %1,			a0"
      "\n\tmovel #_calloldmain,	a0@-"
      "\n\tmoveml d2-d7/a2-a6,	a0@-"
      "\n\tfmovem fp0-fp7,		a0@-"
      "\n\tfmovem fpcr/fpsr,		a0@-"
      "\n\tmovel a0, 			%0"
      : "=g" (romlib_sp) : "g" (ourstackstart) : "a0");
#warning This code is too processor-specific!
#elif defined(i386) || defined(i486)
  asm("    movl %%esp,		%%eax"
      "\n\tmovl %1,			%%esp"
      "\n\tpushl $_calloldmain"
      "\n\tpushal"
      "\n\tpushfl"
      "\n\tsubl $108,%%esp"
      "\n\tfnsave (%%esp)"
      "\n\tpush %%ss"
      "\n\tpush %%ds"
      "\n\tpush %%es"
      "\n\tpush %%fs"
      "\n\tpush %%gs"
      "\n\tmovl %%esp, 		%0"
      "\n\tmovl %%eax,		%%esp"
      : "=g" (romlib_sp) : "g" (ourstackstart) : "%eax");
#else
#error This will not compile
#endif
}
