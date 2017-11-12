/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define Region Mac_Region
#define Cursor Mac_Cursor

#include "rsys/common.h"
#include "QuickDraw.h"

#undef Region
#undef Cursor

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#undef Time
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "rsys/iv.h"

Display *x_dpy;
int x_screen;

int depth;

Window display_window;

XFontStruct *font;

GC copy_gc, text_gc, draw_gc;

void evt_loop(void);

unsigned char *image_data;
int image_width, image_height, row_bytes;
/* x, y of the upper left portion of the image stored in `image_data' */
color_t image_color_map[256];

int current_x, current_y;

int mag_step = 1;
int scroll_step = 4;

Pixmap image_view;

void allocate_pixmap_for_image(void);
void redraw(void);

char *program_name;

int setup_connection(void)
{
    int srv_sockfd, retval;
    struct sockaddr_in srv_addr;
    int reuse_flag = 1;

    srv_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(srv_sockfd < 0)
        abort();
    retval = setsockopt(srv_sockfd, SOL_SOCKET, SO_REUSEADDR,
                        (char *)reuse_flag, sizeof reuse_flag);
#if 0
  if (retval < 0)
    abort ();
#endif

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = PORT;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(srv_sockfd, (struct sockaddr *)&srv_addr, sizeof srv_addr);
    if(retval < 0)
        abort();

    retval = listen(srv_sockfd, 5);
    if(retval < 0)
        abort();

    return srv_sockfd;
}

unsigned long
color_index_to_pixel(int color_index)
{
    unsigned long pixel;
    color_t *color;

    if(depth == 16)
    {
        color = &image_color_map[color_index];
        pixel = ((color->red & 0xF800)
                 | ((color->green >> 5) & 0x7E0)
                 | ((color->blue >> 11) & 0x1F));
    }
    else if(depth == 8)
        pixel = color_index;
    else
    {
        fprintf(stderr, "%s: unable to compute color for depth `%d'\n",
                program_name, depth);
        exit(EXIT_FAILURE);
    }
    return pixel;
}

void convert_image_to_view(void)
{
    int x, y;

    for(y = 0; y < image_height; y++)
    {
        if(y > 1
           && !memcmp(&image_data[row_bytes * (y - 1)],
                      &image_data[row_bytes * y], row_bytes))
        {
            /* just copy the last line */
            XCopyArea(x_dpy, image_view, image_view, copy_gc,
                      0, (y - 1) * mag_step,
                      image_width * mag_step, mag_step,
                      0, y * mag_step);
        }
        else if(y > 2
                && !memcmp(&image_data[row_bytes * (y - 2)],
                           &image_data[row_bytes * y], row_bytes))
        {
            /* just copy the last line */
            XCopyArea(x_dpy, image_view, image_view, copy_gc,
                      0, (y - 2) * mag_step,
                      image_width * mag_step, mag_step,
                      0, y * mag_step);
        }
        else
        {
            unsigned long pixel;
            int color_index;
            int start_x;

            color_index = image_data[row_bytes * y];
            for(start_x = 0, x = 0; x < image_width; x++)
            {
                int new_color_index;

                new_color_index = image_data[x + row_bytes * y];
                if(new_color_index != color_index)
                {
                    pixel = color_index_to_pixel(color_index);
                    XSetForeground(x_dpy, draw_gc, pixel);

                    XFillRectangle(x_dpy, image_view, draw_gc,
                                   start_x * mag_step,
                                   y * mag_step,
                                   (x - start_x) * mag_step, mag_step);
                    start_x = x;
                    color_index = new_color_index;
                }
            }

            pixel = color_index_to_pixel(color_index);
            XSetForeground(x_dpy, draw_gc, pixel);
            XFillRectangle(x_dpy, image_view, draw_gc,
                           start_x * mag_step,
                           y * mag_step,
                           (image_width - start_x) * mag_step, mag_step);
        }
    }
}

void xread(int fd, char *dst, int read_count)
{
    while(read_count)
    {
        int bytes_read;

        bytes_read = read(fd, dst, read_count);
        if(bytes_read <= 0)
            abort();
        read_count -= bytes_read;
        dst += bytes_read;
    }
}

void accept_connection(int fd)
{
    int cli_sockfd;
    socklen_t cli_len;
    struct sockaddr_in cli_addr;
    image_header_t header;

    cli_len = sizeof cli_addr;
    cli_sockfd = accept(fd, (struct sockaddr *)&cli_addr, &cli_len);
    if(cli_sockfd < 0)
        abort();

    xread(cli_sockfd, (char *)&header, sizeof header);

    image_width = header.width;
    image_height = header.height;
    row_bytes = header.row_bytes;
    memcpy(image_color_map, header.image_color_map,
           sizeof image_color_map);

    image_data = realloc(image_data, row_bytes * image_height);

    xread(cli_sockfd, (char *)image_data, row_bytes * image_height);

    allocate_pixmap_for_image();
    convert_image_to_view();

    /* display the sucker */
    redraw();

    close(cli_sockfd);
}

void draw_image_view()
{
    XCopyArea(x_dpy, image_view, display_window, copy_gc,
              current_x * mag_step, current_y * mag_step, 256, 256, 0, 0);
}

int pointer_x, pointer_y;

void draw_pointer_info()
{
    int x, y;
    char buf[256];
    color_t *color_for_point;
    int pixel;

    /* clear echo area */
    XFillRectangle(x_dpy, display_window, copy_gc,
                   0, 256, 256, font->ascent + font->descent + 2);

    x = (pointer_x / mag_step) + current_x;
    y = (pointer_y / mag_step) + current_y;

    if(x < image_width && y < image_height)
        ;
    else
        return;

    pixel = image_data[x + row_bytes * y];
    color_for_point = &image_color_map[pixel];

    sprintf(buf, "(%3d, %3d); %2x; %04x:%04x:%04x", x, y, pixel,
            color_for_point->red,
            color_for_point->blue,
            color_for_point->green);
    XDrawString(x_dpy, display_window, text_gc,
                2, 256 + font->ascent + 1, buf, strlen(buf));
}

void allocate_pixmap_for_image(void)
{
    if(image_view)
        XFreePixmap(x_dpy, image_view);

    image_view = XCreatePixmap(x_dpy, display_window,
                               image_width * mag_step,
                               image_height * mag_step, depth);
}

void set_mag_step(int new_mag_step)
{
    if(new_mag_step < 1
       || new_mag_step > 32
       || new_mag_step == mag_step)
        return;
    mag_step = new_mag_step;
    allocate_pixmap_for_image();
    convert_image_to_view();

    redraw();
}

void redraw(void)
{
    draw_image_view();
    draw_pointer_info();
}

void handle_x_evts(int fd)
{
    XEvent evt;

    while(XCheckMaskEvent(x_dpy, ~0L, &evt))
    {
        switch(evt.type)
        {
            case Expose:
                draw_image_view();
                draw_pointer_info();
                break;
            case MotionNotify:
                pointer_x = evt.xmotion.x;
                pointer_y = evt.xmotion.y;
                draw_pointer_info();
                break;
            case KeyPress:
            {
                KeySym key;
                int n_bytes;
                char buf[81];

                n_bytes = XLookupString((XKeyEvent *)&evt,
                                        buf, 80, &key, NULL);

                if(n_bytes == 1)
                {
                    switch(*buf)
                    {
                        case '-':
                            set_mag_step(mag_step >> 1);
                            break;
                        case '+':
                            set_mag_step(mag_step << 1);
                            break;
                        case '8':
                            if(current_y > 0)
                                current_y -= scroll_step;
                            else
                                return;
                            break;
                        case '2':
                            current_y += scroll_step;
                            break;
                        case '4':
                            if(current_x > 0)
                                current_x -= scroll_step;
                            else
                                return;
                            break;
                        case '6':
                            current_x += scroll_step;
                            break;
                    }
                }
                redraw();
                break;
            }
            default:
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    XSetWindowAttributes xswa;
    XSizeHints size_hints;
    char *font_name = "-adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1";
    XGCValues gc_values;

    int width, height;

    struct
    {
        int fd;
        void (*handler)(int fd);
    } fds[2];
    int n_fds = 2;

    program_name = argv[0];

    x_dpy = XOpenDisplay("");
    if(x_dpy == NULL)
    {
        fprintf(stderr, "%s: could not open x server `%s'.\n",
                program_name, XDisplayName(""));
        exit(EXIT_FAILURE);
    }
    x_screen = XDefaultScreen(x_dpy);

    font = XLoadQueryFont(x_dpy, font_name);
    if(!font)
    {
        fprintf(stderr, "%s: fatal error: failed to load font `%s'\n",
                program_name, font_name);
        exit(EXIT_FAILURE);
    }

    fds[0].fd = setup_connection();
    fds[0].handler = &accept_connection;

    fds[1].fd = XConnectionNumber(x_dpy);
    fds[1].handler = &handle_x_evts;

    width = 256;
    height = 256 + 2 + font->ascent + font->descent;

    /* size and base size are set below after parsing geometry */
    size_hints.flags = PSize;

    size_hints.width = width;
    size_hints.height = height;

    /* make a display window */
    display_window = XCreateWindow(x_dpy, XRootWindow(x_dpy, x_screen),
                                   0, 0, width, height,
                                   0, 0, InputOutput, CopyFromParent, 0, &xswa);

    depth = XDefaultDepth(x_dpy, x_screen);

    image_view = XCreatePixmap(x_dpy, display_window, 256, 256, depth);

    gc_values.function = GXcopy;
    gc_values.foreground = XBlackPixel(x_dpy, x_screen);
    gc_values.background = XWhitePixel(x_dpy, x_screen);

    copy_gc = XCreateGC(x_dpy, display_window,
                        (GCFunction | GCForeground
                         | GCBackground),
                        &gc_values);
    draw_gc = XCreateGC(x_dpy, display_window,
                        (GCFunction | GCForeground
                         | GCBackground),
                        &gc_values);

    gc_values.foreground = XWhitePixel(x_dpy, x_screen);
    gc_values.background = XBlackPixel(x_dpy, x_screen);
    gc_values.font = font->fid;
    text_gc = XCreateGC(x_dpy, display_window,
                        (GCFunction | GCForeground
                         | GCBackground | GCFont),
                        &gc_values);

    XSelectInput(x_dpy, display_window, (ExposureMask | PointerMotionMask
                                         | KeyPressMask));
#if 1
    {
        int x, y;

        image_data = malloc(256 * 256);
        image_height = image_width = row_bytes = 256;
        for(x = 0; x < 256; x++)
            for(y = 0; y < 256; y++)
                image_data[x + y * row_bytes] = y;

        image_color_map[0] = (color_t){ 65535, 65535, 65535 };
        image_color_map[1] = (color_t){ 65535, 65535, 52428 };
        image_color_map[2] = (color_t){ 65535, 65535, 39321 };
        image_color_map[3] = (color_t){ 65535, 65535, 26214 };
        image_color_map[4] = (color_t){ 65535, 65535, 13107 };
        image_color_map[5] = (color_t){ 65535, 65535, 0 };
        image_color_map[6] = (color_t){ 65535, 52428, 65535 };
        image_color_map[7] = (color_t){ 65535, 52428, 52428 };
        image_color_map[8] = (color_t){ 65535, 52428, 39321 };
        image_color_map[9] = (color_t){ 65535, 52428, 26214 };
        image_color_map[10] = (color_t){ 65535, 52428, 13107 };
        image_color_map[11] = (color_t){ 65535, 52428, 0 };
        image_color_map[12] = (color_t){ 65535, 39321, 65535 };
        image_color_map[13] = (color_t){ 65535, 39321, 52428 };
        image_color_map[14] = (color_t){ 65535, 39321, 39321 };
        image_color_map[15] = (color_t){ 65535, 39321, 26214 };
        image_color_map[16] = (color_t){ 65535, 39321, 13107 };
        image_color_map[17] = (color_t){ 65535, 39321, 0 };
        image_color_map[18] = (color_t){ 65535, 26214, 65535 };
        image_color_map[19] = (color_t){ 65535, 26214, 52428 };
        image_color_map[20] = (color_t){ 65535, 26214, 39321 };
        image_color_map[21] = (color_t){ 65535, 26214, 26214 };
        image_color_map[22] = (color_t){ 65535, 26214, 13107 };
        image_color_map[23] = (color_t){ 65535, 26214, 0 };
        image_color_map[24] = (color_t){ 65535, 13107, 65535 };
        image_color_map[25] = (color_t){ 65535, 13107, 52428 };
        image_color_map[26] = (color_t){ 65535, 13107, 39321 };
        image_color_map[27] = (color_t){ 65535, 13107, 26214 };
        image_color_map[28] = (color_t){ 65535, 13107, 13107 };
        image_color_map[29] = (color_t){ 65535, 13107, 0 };
        image_color_map[30] = (color_t){ 65535, 0, 65535 };
        image_color_map[31] = (color_t){ 65535, 0, 52428 };
        image_color_map[32] = (color_t){ 65535, 0, 39321 };
        image_color_map[33] = (color_t){ 65535, 0, 26214 };
        image_color_map[34] = (color_t){ 65535, 0, 13107 };
        image_color_map[35] = (color_t){ 65535, 0, 0 };
        image_color_map[36] = (color_t){ 52428, 65535, 65535 };
        image_color_map[37] = (color_t){ 52428, 65535, 52428 };
        image_color_map[38] = (color_t){ 52428, 65535, 39321 };
        image_color_map[39] = (color_t){ 52428, 65535, 26214 };
        image_color_map[40] = (color_t){ 52428, 65535, 13107 };
        image_color_map[41] = (color_t){ 52428, 65535, 0 };
        image_color_map[42] = (color_t){ 52428, 52428, 65535 };
        image_color_map[43] = (color_t){ 52428, 52428, 52428 };
        image_color_map[44] = (color_t){ 52428, 52428, 39321 };
        image_color_map[45] = (color_t){ 52428, 52428, 26214 };
        image_color_map[46] = (color_t){ 52428, 52428, 13107 };
        image_color_map[47] = (color_t){ 52428, 52428, 0 };
        image_color_map[48] = (color_t){ 52428, 39321, 65535 };
        image_color_map[49] = (color_t){ 52428, 39321, 52428 };
        image_color_map[50] = (color_t){ 52428, 39321, 39321 };
        image_color_map[51] = (color_t){ 52428, 39321, 26214 };
        image_color_map[52] = (color_t){ 52428, 39321, 13107 };
        image_color_map[53] = (color_t){ 52428, 39321, 0 };
        image_color_map[54] = (color_t){ 52428, 26214, 65535 };
        image_color_map[55] = (color_t){ 52428, 26214, 52428 };
        image_color_map[56] = (color_t){ 52428, 26214, 39321 };
        image_color_map[57] = (color_t){ 52428, 26214, 26214 };
        image_color_map[58] = (color_t){ 52428, 26214, 13107 };
        image_color_map[59] = (color_t){ 52428, 26214, 0 };
        image_color_map[60] = (color_t){ 52428, 13107, 65535 };
        image_color_map[61] = (color_t){ 52428, 13107, 52428 };
        image_color_map[62] = (color_t){ 52428, 13107, 39321 };
        image_color_map[63] = (color_t){ 52428, 13107, 26214 };
        image_color_map[64] = (color_t){ 52428, 13107, 13107 };
        image_color_map[65] = (color_t){ 52428, 13107, 0 };
        image_color_map[66] = (color_t){ 52428, 0, 65535 };
        image_color_map[67] = (color_t){ 52428, 0, 52428 };
        image_color_map[68] = (color_t){ 52428, 0, 39321 };
        image_color_map[69] = (color_t){ 52428, 0, 26214 };
        image_color_map[70] = (color_t){ 52428, 0, 13107 };
        image_color_map[71] = (color_t){ 52428, 0, 0 };
        image_color_map[72] = (color_t){ 39321, 65535, 65535 };
        image_color_map[73] = (color_t){ 39321, 65535, 52428 };
        image_color_map[74] = (color_t){ 39321, 65535, 39321 };
        image_color_map[75] = (color_t){ 39321, 65535, 26214 };
        image_color_map[76] = (color_t){ 39321, 65535, 13107 };
        image_color_map[77] = (color_t){ 39321, 65535, 0 };
        image_color_map[78] = (color_t){ 39321, 52428, 65535 };
        image_color_map[79] = (color_t){ 39321, 52428, 52428 };
        image_color_map[80] = (color_t){ 39321, 52428, 39321 };
        image_color_map[81] = (color_t){ 39321, 52428, 26214 };
        image_color_map[82] = (color_t){ 39321, 52428, 13107 };
        image_color_map[83] = (color_t){ 39321, 52428, 0 };
        image_color_map[84] = (color_t){ 39321, 39321, 65535 };
        image_color_map[85] = (color_t){ 39321, 39321, 52428 };
        image_color_map[86] = (color_t){ 39321, 39321, 39321 };
        image_color_map[87] = (color_t){ 39321, 39321, 26214 };
        image_color_map[88] = (color_t){ 39321, 39321, 13107 };
        image_color_map[89] = (color_t){ 39321, 39321, 0 };
        image_color_map[90] = (color_t){ 39321, 26214, 65535 };
        image_color_map[91] = (color_t){ 39321, 26214, 52428 };
        image_color_map[92] = (color_t){ 39321, 26214, 39321 };
        image_color_map[93] = (color_t){ 39321, 26214, 26214 };
        image_color_map[94] = (color_t){ 39321, 26214, 13107 };
        image_color_map[95] = (color_t){ 39321, 26214, 0 };
        image_color_map[96] = (color_t){ 39321, 13107, 65535 };
        image_color_map[97] = (color_t){ 39321, 13107, 52428 };
        image_color_map[98] = (color_t){ 39321, 13107, 39321 };
        image_color_map[99] = (color_t){ 39321, 13107, 26214 };
        image_color_map[100] = (color_t){ 39321, 13107, 13107 };
        image_color_map[101] = (color_t){ 39321, 13107, 0 };
        image_color_map[102] = (color_t){ 39321, 0, 65535 };
        image_color_map[103] = (color_t){ 39321, 0, 52428 };
        image_color_map[104] = (color_t){ 39321, 0, 39321 };
        image_color_map[105] = (color_t){ 39321, 0, 26214 };
        image_color_map[106] = (color_t){ 39321, 0, 13107 };
        image_color_map[107] = (color_t){ 39321, 0, 0 };
        image_color_map[108] = (color_t){ 26214, 65535, 65535 };
        image_color_map[109] = (color_t){ 26214, 65535, 52428 };
        image_color_map[110] = (color_t){ 26214, 65535, 39321 };
        image_color_map[111] = (color_t){ 26214, 65535, 26214 };
        image_color_map[112] = (color_t){ 26214, 65535, 13107 };
        image_color_map[113] = (color_t){ 26214, 65535, 0 };
        image_color_map[114] = (color_t){ 26214, 52428, 65535 };
        image_color_map[115] = (color_t){ 26214, 52428, 52428 };
        image_color_map[116] = (color_t){ 26214, 52428, 39321 };
        image_color_map[117] = (color_t){ 26214, 52428, 26214 };
        image_color_map[118] = (color_t){ 26214, 52428, 13107 };
        image_color_map[119] = (color_t){ 26214, 52428, 0 };
        image_color_map[120] = (color_t){ 26214, 39321, 65535 };
        image_color_map[121] = (color_t){ 26214, 39321, 52428 };
        image_color_map[122] = (color_t){ 26214, 39321, 39321 };
        image_color_map[123] = (color_t){ 26214, 39321, 26214 };
        image_color_map[124] = (color_t){ 26214, 39321, 13107 };
        image_color_map[125] = (color_t){ 26214, 39321, 0 };
        image_color_map[126] = (color_t){ 26214, 26214, 65535 };
        image_color_map[127] = (color_t){ 26214, 26214, 52428 };
        image_color_map[128] = (color_t){ 26214, 26214, 39321 };
        image_color_map[129] = (color_t){ 26214, 26214, 26214 };
        image_color_map[130] = (color_t){ 26214, 26214, 13107 };
        image_color_map[131] = (color_t){ 26214, 26214, 0 };
        image_color_map[132] = (color_t){ 26214, 13107, 65535 };
        image_color_map[133] = (color_t){ 26214, 13107, 52428 };
        image_color_map[134] = (color_t){ 26214, 13107, 39321 };
        image_color_map[135] = (color_t){ 26214, 13107, 26214 };
        image_color_map[136] = (color_t){ 26214, 13107, 13107 };
        image_color_map[137] = (color_t){ 26214, 13107, 0 };
        image_color_map[138] = (color_t){ 26214, 0, 65535 };
        image_color_map[139] = (color_t){ 26214, 0, 52428 };
        image_color_map[140] = (color_t){ 26214, 0, 39321 };
        image_color_map[141] = (color_t){ 26214, 0, 26214 };
        image_color_map[142] = (color_t){ 26214, 0, 13107 };
        image_color_map[143] = (color_t){ 26214, 0, 0 };
        image_color_map[144] = (color_t){ 13107, 65535, 65535 };
        image_color_map[145] = (color_t){ 13107, 65535, 52428 };
        image_color_map[146] = (color_t){ 13107, 65535, 39321 };
        image_color_map[147] = (color_t){ 13107, 65535, 26214 };
        image_color_map[148] = (color_t){ 13107, 65535, 13107 };
        image_color_map[149] = (color_t){ 13107, 65535, 0 };
        image_color_map[150] = (color_t){ 13107, 52428, 65535 };
        image_color_map[151] = (color_t){ 13107, 52428, 52428 };
        image_color_map[152] = (color_t){ 13107, 52428, 39321 };
        image_color_map[153] = (color_t){ 13107, 52428, 26214 };
        image_color_map[154] = (color_t){ 13107, 52428, 13107 };
        image_color_map[155] = (color_t){ 13107, 52428, 0 };
        image_color_map[156] = (color_t){ 13107, 39321, 65535 };
        image_color_map[157] = (color_t){ 13107, 39321, 52428 };
        image_color_map[158] = (color_t){ 13107, 39321, 39321 };
        image_color_map[159] = (color_t){ 13107, 39321, 26214 };
        image_color_map[160] = (color_t){ 13107, 39321, 13107 };
        image_color_map[161] = (color_t){ 13107, 39321, 0 };
        image_color_map[162] = (color_t){ 13107, 26214, 65535 };
        image_color_map[163] = (color_t){ 13107, 26214, 52428 };
        image_color_map[164] = (color_t){ 13107, 26214, 39321 };
        image_color_map[165] = (color_t){ 13107, 26214, 26214 };
        image_color_map[166] = (color_t){ 13107, 26214, 13107 };
        image_color_map[167] = (color_t){ 13107, 26214, 0 };
        image_color_map[168] = (color_t){ 13107, 13107, 65535 };
        image_color_map[169] = (color_t){ 13107, 13107, 52428 };
        image_color_map[170] = (color_t){ 13107, 13107, 39321 };
        image_color_map[171] = (color_t){ 13107, 13107, 26214 };
        image_color_map[172] = (color_t){ 13107, 13107, 13107 };
        image_color_map[173] = (color_t){ 13107, 13107, 0 };
        image_color_map[174] = (color_t){ 13107, 0, 65535 };
        image_color_map[175] = (color_t){ 13107, 0, 52428 };
        image_color_map[176] = (color_t){ 13107, 0, 39321 };
        image_color_map[177] = (color_t){ 13107, 0, 26214 };
        image_color_map[178] = (color_t){ 13107, 0, 13107 };
        image_color_map[179] = (color_t){ 13107, 0, 0 };
        image_color_map[180] = (color_t){ 0, 65535, 65535 };
        image_color_map[181] = (color_t){ 0, 65535, 52428 };
        image_color_map[182] = (color_t){ 0, 65535, 39321 };
        image_color_map[183] = (color_t){ 0, 65535, 26214 };
        image_color_map[184] = (color_t){ 0, 65535, 13107 };
        image_color_map[185] = (color_t){ 0, 65535, 0 };
        image_color_map[186] = (color_t){ 0, 52428, 65535 };
        image_color_map[187] = (color_t){ 0, 52428, 52428 };
        image_color_map[188] = (color_t){ 0, 52428, 39321 };
        image_color_map[189] = (color_t){ 0, 52428, 26214 };
        image_color_map[190] = (color_t){ 0, 52428, 13107 };
        image_color_map[191] = (color_t){ 0, 52428, 0 };
        image_color_map[192] = (color_t){ 0, 39321, 65535 };
        image_color_map[193] = (color_t){ 0, 39321, 52428 };
        image_color_map[194] = (color_t){ 0, 39321, 39321 };
        image_color_map[195] = (color_t){ 0, 39321, 26214 };
        image_color_map[196] = (color_t){ 0, 39321, 13107 };
        image_color_map[197] = (color_t){ 0, 39321, 0 };
        image_color_map[198] = (color_t){ 0, 26214, 65535 };
        image_color_map[199] = (color_t){ 0, 26214, 52428 };
        image_color_map[200] = (color_t){ 0, 26214, 39321 };
        image_color_map[201] = (color_t){ 0, 26214, 26214 };
        image_color_map[202] = (color_t){ 0, 26214, 13107 };
        image_color_map[203] = (color_t){ 0, 26214, 0 };
        image_color_map[204] = (color_t){ 0, 13107, 65535 };
        image_color_map[205] = (color_t){ 0, 13107, 52428 };
        image_color_map[206] = (color_t){ 0, 13107, 39321 };
        image_color_map[207] = (color_t){ 0, 13107, 26214 };
        image_color_map[208] = (color_t){ 0, 13107, 13107 };
        image_color_map[209] = (color_t){ 0, 13107, 0 };
        image_color_map[210] = (color_t){ 0, 0, 65535 };
        image_color_map[211] = (color_t){ 0, 0, 52428 };
        image_color_map[212] = (color_t){ 0, 0, 39321 };
        image_color_map[213] = (color_t){ 0, 0, 26214 };
        image_color_map[214] = (color_t){ 0, 0, 13107 };
        image_color_map[215] = (color_t){ 61166, 0, 0 };
        image_color_map[216] = (color_t){ 56797, 0, 0 };
        image_color_map[217] = (color_t){ 48059, 0, 0 };
        image_color_map[218] = (color_t){ 43690, 0, 0 };
        image_color_map[219] = (color_t){ 34952, 0, 0 };
        image_color_map[220] = (color_t){ 30583, 0, 0 };
        image_color_map[221] = (color_t){ 21845, 0, 0 };
        image_color_map[222] = (color_t){ 17476, 0, 0 };
        image_color_map[223] = (color_t){ 8738, 0, 0 };
        image_color_map[224] = (color_t){ 4369, 0, 0 };
        image_color_map[225] = (color_t){ 0, 61166, 0 };
        image_color_map[226] = (color_t){ 0, 56797, 0 };
        image_color_map[227] = (color_t){ 0, 48059, 0 };
        image_color_map[228] = (color_t){ 0, 43690, 0 };
        image_color_map[229] = (color_t){ 0, 34952, 0 };
        image_color_map[230] = (color_t){ 0, 30583, 0 };
        image_color_map[231] = (color_t){ 0, 21845, 0 };
        image_color_map[232] = (color_t){ 0, 17476, 0 };
        image_color_map[233] = (color_t){ 0, 8738, 0 };
        image_color_map[234] = (color_t){ 0, 4369, 0 };
        image_color_map[235] = (color_t){ 0, 0, 61166 };
        image_color_map[236] = (color_t){ 0, 0, 56797 };
        image_color_map[237] = (color_t){ 0, 0, 48059 };
        image_color_map[238] = (color_t){ 0, 0, 43690 };
        image_color_map[239] = (color_t){ 0, 0, 34952 };
        image_color_map[240] = (color_t){ 0, 0, 30583 };
        image_color_map[241] = (color_t){ 0, 0, 21845 };
        image_color_map[242] = (color_t){ 0, 0, 17476 };
        image_color_map[243] = (color_t){ 0, 0, 8738 };
        image_color_map[244] = (color_t){ 0, 0, 4369 };
        image_color_map[245] = (color_t){ 61166, 61166, 61166 };
        image_color_map[246] = (color_t){ 56797, 56797, 56797 };
        image_color_map[247] = (color_t){ 48059, 48059, 48059 };
        image_color_map[248] = (color_t){ 43690, 43690, 43690 };
        image_color_map[249] = (color_t){ 34952, 34952, 34952 };
        image_color_map[250] = (color_t){ 30583, 30583, 30583 };
        image_color_map[251] = (color_t){ 21845, 21845, 21845 };
        image_color_map[252] = (color_t){ 17476, 17476, 17476 };
        image_color_map[253] = (color_t){ 8738, 8738, 8738 };
        image_color_map[254] = (color_t){ 4369, 4369, 4369 };
        image_color_map[255] = (color_t){ 0, 0, 0 };

        convert_image_to_view();
    }
#endif

    XMapRaised(x_dpy, display_window);

    /* accept_connection (fds[0].fd); */
    handle_x_evts(/* dummy */ -1);

    while(1)
    {
        int n_fds_available, i, max_fd;
        fd_set read_set;

        max_fd = 0;
        __FD_ZERO(&read_set);
        for(i = 0; i < n_fds; i++)
        {
            max_fd = MAX(max_fd, fds[i].fd);
            __FD_SET(fds[i].fd, &read_set);
        }

        n_fds_available = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if(n_fds_available < 0)
            abort();

        for(i = 0; i < n_fds; i++)
            if(__FD_ISSET(fds[i].fd, &read_set))
                (*fds[i].handler)(fds[i].fd);
    }
}
