# ARDIMODS V1.3 Makefile, accept no substitutes

SERVER_OBJ= ardimods_main.o ardimods.o
CFLAGS= -g -DKERNEL -DKERNEL_FEATURES -DMACH 
ALL=ardimods_reloc
DESTDIR=/LocalApps

#
# NOTE: the install directive leaves
#	ardimods_reloc in the current directory (which is where it needs
#	to be).
#

# NOTE:  FASTALINETRAPS is no longer an option; it's always turned on.
#	 There is no reason to turn it off.

all:	$(ALL)

install:	ardimods_reloc

ardimods_reloc: ${SERVER_OBJ} LoadCommands UnloadCommands
	kl_ld -n ardimods -l LoadCommands -u UnloadCommands -i instance \
		-o $@ ${SERVER_OBJ}

clean:
	rm -f *.o
	rm -f $(ALL)
	
ardimods.o ardimods_main.o:	Makefile

.c.o:
	${CC} ${CFLAGS} ${ARDIOPTIONS} -c $*.c

.s.o:
	$(CC) ${ARDIOPTIONS} -c $*.s
