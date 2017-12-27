/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(NDEBUG)

#include "rsys/common.h"

#include "DialogMgr.h"
#include "QuickDraw.h"

#include "rsys/cquick.h"
#include "rsys/region.h"
#include "rsys/stdbits.h"
#include "rsys/dump.h"

#include "rsys/iv.h"

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace Executor;

#define print_errno_error_message(call, msg)   \
    do {                                       \
        fprintf(stderr, "%s:%d; %s: %s; %s\n", \
                __FILE__, __LINE__,            \
                call, strerror(errno),         \
                msg);                          \
    } while(0)

void send_image(int width, int height,
                int row_bytes, int send_row_bytes,
                char *base_addr,
                CTabHandle ctab)
{
    image_header_t header;

    int sockfd, retval, i;
    struct sockaddr_in srv_addr;
    char hostname[1024];
    struct hostent *host;

    header.width = width;
    header.height = height;
    header.row_bytes = send_row_bytes;

    memset(header.image_color_map, '\0', sizeof header.image_color_map);

    for(i = 0; i <= CTAB_SIZE(ctab); i++)
    {
        color_t *color;
        ColorSpec *color_spec;

        color = &header.image_color_map[i];
        color_spec = &CTAB_TABLE(ctab)[i];

        color->red = CW(color_spec->rgb.red);
        color->green = CW(color_spec->rgb.green);
        color->blue = CW(color_spec->rgb.blue);
    }
    /* connect to server, and send */
    gethostname(hostname, 1024);
    host = gethostbyname(hostname);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0)
    {
        print_errno_error_message("socket", "unable to send image");
        return;
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = PORT;
    srv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    retval = connect(sockfd, (struct sockaddr *)&srv_addr,
                     sizeof srv_addr);
    if(retval < 0)
    {
        print_errno_error_message("connect", "unable to send image");
        return;
    }

    retval = write(sockfd, &header, sizeof header);
    if(retval < 0)
    {
        print_errno_error_message("write", "unable to send image");
        return;
    }

    for(i = 0; i < height; i++)
    {
        retval = write(sockfd, base_addr, send_row_bytes);
        base_addr += row_bytes;
        if(retval < 0)
        {
            print_errno_error_message("write", "unable to send image");
            return;
        }
    }

    close(sockfd);
}

void dump_image(BitMap *bogo_bitmap, Rect *rect);

void dump_grafport_image(GrafPort *port)
{
    Rect r, bounds;

    r = PORT_RECT(port);
    bounds = PORT_BOUNDS(port);
    OffsetRect(&bounds,
               CW(r.left) - CW(bounds.left),
               CW(r.top) - CW(bounds.top));
    SectRect(&r, &bounds, &r);
    dump_image(&port->portBits, &r);
}

void dump_image(BitMap *bogo_bitmap, Rect *rect)
{
    struct cleanup_info cleanup;
    int width, height, row_bytes;
    char *base_addr, *_base_addr;
    PixMap dummy_space;
    PixMap *pixmap = &dummy_space;
    int depth;
    int map_row_bytes;

    canonicalize_bogo_map(bogo_bitmap, &pixmap, &cleanup);

    width = RECT_WIDTH(rect);
    height = RECT_HEIGHT(rect);

    /* destination must be 8bpp */
    depth = CW(pixmap->pixelSize);
    if(depth != 8)
    {
        int map_width, map_height;
        static PixMap new_pixmap;
        ColorTable *conv_table;
        int i;

        map_width = RECT_WIDTH(&pixmap->bounds);
        map_height = RECT_HEIGHT(&pixmap->bounds);

        map_row_bytes = ((width * 8 + 31) / 32) * 4;
        base_addr = (char *)alloca(map_row_bytes * height);

        new_pixmap.rowBytes = CW(map_row_bytes);
        new_pixmap.baseAddr = RM((Ptr)base_addr);

        new_pixmap.pixelSize = CWC(8);
        new_pixmap.cmpCount = CWC(1);
        new_pixmap.cmpSize = CWC(8);

        new_pixmap.pmTable = NULL;

        conv_table = (ColorTable *)alloca(CTAB_STORAGE_FOR_SIZE(1 << depth));
        for(i = 0; i < (1 << depth); i++)
            conv_table->ctTable[i].value = CW(i);

        convert_pixmap(pixmap, &new_pixmap, rect,
                       conv_table);
        new_pixmap.pmTable = pixmap->pmTable;
        pixmap = &new_pixmap;
    }
    else
    {
        map_row_bytes = CW(pixmap->rowBytes
                           & ~ROWBYTES_FLAG_BITS_X);
        base_addr = (char *)MR(pixmap->baseAddr);
    }

    row_bytes = width;
    _base_addr
        = &base_addr[(CW(rect->top) - CW(pixmap->bounds.top)) * map_row_bytes
                     + (CW(rect->left) - CW(pixmap->bounds.left))];
    send_image(width, height,
               map_row_bytes, row_bytes,
               _base_addr,
               MR(pixmap->pmTable));

    canonicalize_bogo_map_cleanup((BitMap *)&pixmap, &cleanup);
}

void dump_rgn_as_image(RgnHandle rh)
{
    BitMap bm;
    int row_bytes;
    char *baseaddr;

    bm.bounds = RGN_BBOX(rh);
    row_bytes = ((RECT_WIDTH(&bm.bounds) + 31) / 32) * 4;
    bm.rowBytes = CW(row_bytes);
    baseaddr = (char *)alloca(row_bytes * RECT_HEIGHT(&bm.bounds));
    bm.baseAddr = RM((Ptr)baseaddr);
    memset(baseaddr, '\377', row_bytes * RECT_HEIGHT(&bm.bounds));

    CopyBits(&bm, &bm,
             &bm.bounds, &bm.bounds,
             srcXor, rh);

    dump_image(&bm, &bm.bounds);
}

#endif /* !NDEBUG */
