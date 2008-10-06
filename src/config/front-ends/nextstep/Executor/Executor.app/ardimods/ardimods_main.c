/*
 * Although written by Abacus R&D, the source code in this file is not
 * copyrighted by Abacus R&D, but is in the public domain.
 */
 
/* ardimods_main.c:  major functions of ardimods */

#import <syslog.h>
#import <kernserv/kern_server_types.h>

kern_server_t instance;

/* ardimods_init:  Called when ardimods is loaded. */
void ardimods_init(void)
{
    replace_vectors();
    printf("ARDI mods 1.8 loaded (for NEXTSTEP 3.0 or later)\n");
    printf("Fast A-line Traps installed\n");
}

/* ardimods_signoff:  Called when ardimods is unloaded. */
void ardimods_signoff(void)
{
    restore_vectors();
    printf("ARDI mods 1.8 unloaded\n");
    printf("Fast A-line Traps removed\n");
}
