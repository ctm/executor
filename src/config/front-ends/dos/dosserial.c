/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#include "rsys/common.h"
#include "dosserial.h"
#include <dpmi.h>
#include <pc.h>
#include "dpmicall.h"
#include "Serial.h"

#define USE_SVA

#if defined(USE_SVA)
#include "svasync/svasync.h"
#endif

enum
{
    PARAMETER_ERROR = -2
};

/*
 * A useful macro for looking up the value of a key in a mapping table.
 * It uses the gcc-specific construct of ({ ... })
 */

#define LOOKUP_KEY(valp, thekey, map)   \
    ({                                  \
        int i;                          \
        int retval;                     \
                                        \
        retval = PARAMETER_ERROR;       \
        for(i = 0; i < NELEM(map); ++i) \
            if(map[i].key == thekey)    \
            {                           \
                *valp = map[i].val;     \
                retval = 0;             \
                break;                  \
            }                           \
        retval;                         \
    })

static struct
{
    bios_port_t val;
    uint8 key;
} serial_port_map[] = {
    { 0, 0 },
    { 1, 1 },
};

/*
 * xxx = 0 for modem, 1 for printer
 */

PRIVATE int
set_xxx_port_mapping_to_pc_port(int xxx, int pc_port)
{
    int retval;

    if(xxx >= 0 && xxx <= 1 && pc_port >= 1 && pc_port <= 4)
    {
        serial_port_map[xxx].val = pc_port - 1;
        retval = 0;
    }
    else
        retval = PARAMETER_ERROR;

    return retval;
}

PUBLIC int
set_modem_port_mapping_to_pc_port(int pc_port)
{
    int retval;

    retval = set_xxx_port_mapping_to_pc_port(0, pc_port);
    return retval;
}

PUBLIC int
set_printer_port_mapping_to_pc_port(int pc_port)
{
    int retval;

    retval = set_xxx_port_mapping_to_pc_port(1, pc_port);
    return retval;
}

/*
 * These five helper routines convert from the values we use on the outside
 * world to the values that the BIOS calls use.  If an inappropriate source
 * value is supplied, then they return PARAMETER_ERROR, otherwise they return
 * 0
 */

PRIVATE int
port_to_bios_port(bios_port_t *bios_portp, uint8 port)
{
    int retval;

    retval = LOOKUP_KEY(bios_portp, port, serial_port_map);
    return retval;
}

#if !defined(USE_SVA)

PRIVATE int
baud_to_bios_baud(bios_serial_baud_t *bios_baudp, uint32 baud)
{
    int retval;

    struct
    {
        bios_serial_baud_t val;
        uint32 key;
    } map[] = {
        /* { bios_serial_baud_110, 110, }, */
        /* { bios_serial_baud_150, 150, }, */
        {
            bios_serial_baud_300, baud300,
        },
        {
            bios_serial_baud_600, baud600,
        },
        {
            bios_serial_baud_1200, baud1200,
        },
        {
            bios_serial_baud_2400, baud2400,
        },
        {
            bios_serial_baud_4800, baud4800,
        },
        {
            bios_serial_baud_9600, baud9600,
        },
    };
    retval = LOOKUP_KEY(bios_baudp, baud, map);
    return retval;
}

#else

PRIVATE int
baud_to_sva_baud(unsigned int *sva_baudp, uint32 baud)
{
    int retval;

    struct
    {
        unsigned int val;
        uint32 key;
    } map[] = {
        {
            300, baud300,
        },
        {
            600, baud600,
        },
        {
            1200, baud1200,
        },
        {
            2400, baud2400,
        },
        {
            4800, baud4800,
        },
        {
            9600, baud9600,
        },
    };
    retval = LOOKUP_KEY(sva_baudp, baud, map);
    return retval;
}

#endif

PRIVATE int
parity_to_bios_parity(bios_serial_parity_t *bios_parityp, parity_t parity)
{
    int retval;

    struct
    {
        bios_serial_parity_t val;
        parity_t key;
    } map[] = {
        {
            bios_serial_parity_none0, noParity,
        },
        {
            bios_serial_parity_odd, oddParity,
        },
        {
            bios_serial_parity_even, evenParity,
        },
    };
    retval = LOOKUP_KEY(bios_parityp, parity, map);
    return retval;
}

PRIVATE int
stop_to_bios_stop(bios_serial_stop_bits_t *bios_stopp, uint32 stop_bits)
{
    int retval;

    struct
    {
        bios_serial_stop_bits_t val;
        uint32 key;
    } map[] = {
        /* NOTE: 1 = 1, 2 = 1.5, 3 = 2 */
        {
            bios_serial_stop_bits_1, 1,
        },
        {
            bios_serial_stop_bits_more_than_1, 2,
        }, /* slight cheat here, since if
					   anyone ever uses a data length
					   of five bits, then "more than 1"
					   is really 1.5 */
        {
            bios_serial_stop_bits_more_than_1, 3,
        }
    };
    retval = LOOKUP_KEY(bios_stopp, stop_bits, map);
    return retval;
}

PRIVATE int
length_to_bios_length(bios_serial_length_t *bios_lengthp, uint32 data_length)
{
    int retval;

    struct
    {
        bios_serial_length_t val;
        uint32 key;
    } map[] = {
        {
            bios_serial_length_5, data5,
        },
        {
            bios_serial_length_6, data6,
        },
        {
            bios_serial_length_7, data7,
        },
        {
            bios_serial_length_8, data8,
        },
    };
    retval = LOOKUP_KEY(bios_lengthp, data_length, map);
    return retval;
}

/*
 * These five macros convert from the values we use on the outside world
 * to the values that the BIOS calls use.  If an inappropriate source value
 * is supplied, they automatically force the calling function to return
 *  PARAMETER_ERROR
 */

#define PORT_TO_BIOS_PORT(bios_portp, port)          \
    do                                               \
    {                                                \
        if(port_to_bios_port(bios_portp, port) != 0) \
            return PARAMETER_ERROR;                  \
    } while(0)

#define BAUD_TO_BIOS_BAUD(bios_baudp, baud)          \
    do                                               \
    {                                                \
        if(baud_to_bios_baud(bios_baudp, baud) != 0) \
            return PARAMETER_ERROR;                  \
    } while(0)

#define BAUD_TO_SVA_BAUD(sva_baudp, baud)          \
    do                                             \
    {                                              \
        if(baud_to_sva_baud(sva_baudp, baud) != 0) \
            return PARAMETER_ERROR;                \
    } while(0)

#define PARITY_TO_BIOS_PARITY(bios_parityp, parity)          \
    do                                                       \
    {                                                        \
        if(parity_to_bios_parity(bios_parityp, parity) != 0) \
            return PARAMETER_ERROR;                          \
    } while(0)

#define STOP_TO_BIOS_STOP(bios_stopp, stop_bits)          \
    do                                                    \
    {                                                     \
        if(stop_to_bios_stop(bios_stopp, stop_bits) != 0) \
            return PARAMETER_ERROR;                       \
    } while(0)

#define LENGTH_TO_BIOS_LENGTH(bios_lengthp, data_length)          \
    do                                                            \
    {                                                             \
        if(length_to_bios_length(bios_lengthp, data_length) != 0) \
            return PARAMETER_ERROR;                               \
    } while(0)

PRIVATE unsigned int shadow_hand;

PRIVATE void
shadow_SVAsyncHand(unsigned int Hand)
{
    shadow_hand = Hand;
    SVAsyncHand(Hand);
}

enum
{
    DEFAULT_BAUD = 9600
};
enum
{
    DEFAULT_COM = BITS_8 | STOP_1 | NO_PARITY
};
enum
{
    DEFAULT_HAND = DTR | RTS
};

PRIVATE void
make_sure_sva_initted(void)
{
    static bool initted = false;

    if(!initted)
    {
#warning poor implementation of port mapping
        SVAsyncInit(serial_port_map[0].val + COM1);
        atexit(SVAsyncStop);
        SVAsyncFifoInit();
        SVAsyncSet(DEFAULT_BAUD, DEFAULT_COM);
        shadow_SVAsyncHand(DEFAULT_HAND);
        initted = true;
    }
}

/*
 * These are the serial port accessing routines that the BIOS directly
 * provides.  We build all other routines on top of these (at least until
 * we write our own drivers).
 */

PRIVATE int
dos_serial_bios_init_port(uint8 port,
                          uint32 baud,
                          uint32 parity,
                          uint32 stop_bits,
                          uint32 data_length,
                          line_status_t *line_statusp,
                          modem_status_t *modem_statusp)
{
    int retval;
    bios_port_t bios_port;
    bios_serial_parity_t bios_parity;
    bios_serial_stop_bits_t bios_stop;
    bios_serial_length_t bios_length;
    bios_serial_init_u bios;
#if !defined(USE_SVA)
    bios_serial_baud_t bios_baud;
    __dpmi_regs regs;
#else
    unsigned int sva_baud;
#endif

    PORT_TO_BIOS_PORT(&bios_port, port);
    PARITY_TO_BIOS_PARITY(&bios_parity, parity);
    STOP_TO_BIOS_STOP(&bios_stop, stop_bits);
    LENGTH_TO_BIOS_LENGTH(&bios_length, data_length);

    bios.ub = 0;
    bios.init.bios_serial_init_parity = bios_parity;
    bios.init.bios_serial_init_stop_bits = bios_stop;
    bios.init.bios_serial_init_data_length = bios_length;

#if !defined(USE_SVA)
    BAUD_TO_BIOS_BAUD(&bios_baud, baud);
    bios.init.bios_serial_init_baud = bios_baud;

    dpmi_zero_regs(&regs);
    regs.h.ah = 0;
    regs.h.al = bios.ub;
    regs.x.dx = bios_port;

    retval = __dpmi_int(0x14, &regs);
    if(retval >= 0)
    {
        if(line_statusp)
        {
            line_status_u line;

            line.ub = regs.h.ah;
            *line_statusp = line.status;
        }

        if(modem_statusp)
        {
            modem_status_u modem;

            modem.ub = regs.h.al;
            *modem_statusp = modem.status;
        }
    }
    return retval;
#else
    BAUD_TO_SVA_BAUD(&sva_baud, baud);
    make_sure_sva_initted();
    SVAsyncSet(sva_baud, bios.ub);
    retval = 0;
    return retval;
#endif
}

PRIVATE int
dos_serial_bios_send_byte(uint8 port,
                          uint8 byte,
                          line_status_t *line_statusp)
{
    int retval;
#if !defined(USE_SVA)
    bios_port_t bios_port;
    __dpmi_regs regs;

    PORT_TO_BIOS_PORT(&bios_port, port);

    dpmi_zero_regs(&regs);
    regs.h.ah = 1;
    regs.h.al = byte;
    regs.x.dx = bios_port;
    retval = __dpmi_int(0x14, &regs);
    if(retval >= 0 && line_statusp)
    {
        line_status_u line;

        line.ub = regs.h.ah;
        *line_statusp = line.status;
    }
    return retval;
#else
    make_sure_sva_initted();
    SVAsyncOut(byte);
    retval = 0;
    return retval;
#endif
}

PRIVATE int
dos_serial_bios_receive_byte(uint8 port,
                             uint8 *bytep,
                             line_status_t *line_statusp)
{
#if !defined(USE_SVA)
    int retval;
    bios_port_t bios_port;
    __dpmi_regs regs;

    PORT_TO_BIOS_PORT(&bios_port, port);

    dpmi_zero_regs(&regs);
    regs.h.ah = 2;
    regs.x.dx = bios_port;
    retval = __dpmi_int(0x14, &regs);
    if(retval >= 0)
    {
        if(line_statusp)
        {
            line_status_u line;

            line.ub = regs.h.ah;
            *line_statusp = line.status;
        }
        *bytep = regs.h.al;
    }
    return retval;
#else

#warning currently SVA only allows one port at a time

    int c;
    int retval;

    make_sure_sva_initted();
    c = SVAsyncIn();
    if(c == -1)
        retval = -1;
    else
    {
        *bytep = c;
        retval = 0;
    }

    return retval;
#endif
}

PRIVATE int
dos_serial_bios_read_status(uint8 port,
                            line_status_t *line_statusp,
                            modem_status_t *modem_statusp)
{
    int retval;
    bios_port_t bios_port;
    __dpmi_regs regs;

    PORT_TO_BIOS_PORT(&bios_port, port);

    dpmi_zero_regs(&regs);
    regs.h.ah = 3;
    regs.x.dx = bios_port;
    retval = __dpmi_int(0x14, &regs);

    if(retval >= 0)
    {
        if(line_statusp)
        {
            line_status_u line;

            line.ub = regs.h.ah;
            *line_statusp = line.status;
        }

        if(modem_statusp)
        {
            modem_status_u modem;

            modem.ub = regs.h.al;
            *modem_statusp = modem.status;
        }
    }

    return retval;
}

/*
 * Here are the public routines.  They're made to slip comfortably into
 * the existing "serial.c" code (which is quite a hack)
 *
 * The first implementation of these routines is incredibly naive.  One thing
 * that we can do if the first cut isn't sufficient is to always pay attention
 * to the line status, and whenever there's a character that can be read,
 * read it and squirrel it away for future use.  That can be done in all of
 * these routines, even the write routine.
 */

PUBLIC ssize_t
serial_bios_read(LONGINT fd, void *buf, size_t count)
{
    ssize_t retval;
    uint8 *bufp;

    retval = 0;
    bufp = buf;
    while(count-- > 0)
    {
        if(dos_serial_bios_receive_byte(fd, bufp, 0) < 0)
            break;
        ++retval;
        ++bufp;
    }
    return retval;
}

PUBLIC ssize_t
serial_bios_write(LONGINT fd, void *buf, size_t count)
{
    ssize_t retval;
    uint8 *bufp;

    retval = 0;
    bufp = buf;
    while(count-- > 0)
    {
        if(dos_serial_bios_send_byte(fd, *bufp, 0) < 0)
            break;
        ++retval;
        ++bufp;
    }
    return retval;
}

PUBLIC OSErr
serial_bios_serset(LONGINT fd, INTEGER param)
{
    OSErr retval;
    uint32 mac_baud, mac_data_length, mac_parity, mac_stop_bits;

    mac_baud = param & 0x3FF;
    mac_data_length = param & 0xc00;
    mac_parity = param & 0x3000;
    mac_stop_bits = ((param >> 14) & 0x3);

    dos_serial_bios_init_port(fd, mac_baud, mac_parity, mac_stop_bits,
                              mac_data_length, 0, 0);

    retval = noErr;
    return retval;
}

PUBLIC OSErr
serial_bios_serxhshake(LONGINT fd, SerShk *sershkp)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = noErr;
    return retval;
}

/*
 * In order to explicitly set the baud without setting anything else, we'll
 * need to keep track of the last settings we've made for each port.  That's
 * not hard to do, but we just don't do it yet.
 */

PUBLIC OSErr
serial_bios_setbaud(LONGINT fd, INTEGER baud)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = noErr;
    return retval;
}

PUBLIC OSErr
serial_bios_ctlbrk(LONGINT fd, INTEGER flag)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = noErr;
    return retval;
}

PUBLIC OSErr
serial_bios_setflow(LONGINT fd, LONGINT flag)
{
    OSErr retval;

    warning_unimplemented(NULL_STRING);
    retval = noErr;
    return retval;
}

enum
{
    SERIAL_ADDRESSES = 0x400
}; /* BIOS Address containing 4 shorts, each
				      one being an i/o port address for a
				      serial port.  See _The Undocumented PC_
				      page 578 */

enum
{
    MODEM_CONTROL_REGISTER_OFFSET = 4
}; /* offset from i/o port to modem
					      control register.  See _TUPC_
					      p. 612 */

PRIVATE OSErr
serial_bios_dtr(LONGINT fd, int value)
{
#if !defined(USE_SVA)
    bios_port_t bios_port;
    unsigned long offset;
    unsigned short port;
    modem_control_register_u modem_control;
    OSErr retval;

    PORT_TO_BIOS_PORT(&bios_port, fd);
    offset = SERIAL_ADDRESSES + (bios_port * 2);
    _dosmemgetw(offset, 1, &port);
    port += MODEM_CONTROL_REGISTER_OFFSET;
    modem_control.ub = inportb(port);
    modem_control.reg.modem_control_dtr = value;
    outportb(port, modem_control.ub);
    retval = noErr;
    return retval;
#else
    OSErr retval;
    unsigned int hand;

    if(value)
        hand = shadow_hand | DTR;
    else
        hand = shadow_hand & ~DTR;

    shadow_SVAsyncHand(hand);

    retval = noErr;
    return retval;
#endif
}

PUBLIC OSErr
serial_bios_setdtr(LONGINT fd)
{
    OSErr retval;

    retval = serial_bios_dtr(fd, 1);
    return retval;
}

PUBLIC OSErr
serial_bios_clrdtr(LONGINT fd)
{
    OSErr retval;

    retval = serial_bios_dtr(fd, 0);
    return retval;
}

/*
 * In the current kludgey implementation, we only differentiate between no
 * characters ready to read and one.  Hopefully that will be enough to drive
 * the scanner.
 */

PUBLIC int
serial_bios_fionread(LONGINT fd, LONGINT *np)
{
#if !defined(USE_SVA)
    int retval;
    line_status_t status;

    if((dos_serial_bios_read_status(fd, &status, 0) >= 0) && status.line_data_ready)
        *np = 1;
    else
        *np = 0;

    retval = 0;
    return retval;
#else
    int retval;

    make_sure_sva_initted();
    *np = SVAsyncInStat();
    retval = 0;
    return retval;
#endif
}
