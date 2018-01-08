
#include <QApplication>
#include <QPainter>
#include <QRasterWindow>
#include <QMouseEvent>
#include <QBitmap>
#include <QScreen>

#include "rsys/common.h"
#include "rsys/host.h"
#include "rsys/vdriver.h"
#include "rsys/cquick.h" /* for ROMlib_log2 */
#include "rsys/adb.h"
#include "rsys/osevent.h"
#include "rsys/scrap.h"
#include "rsys/keyboard.h"
#include "rsys/parse.h"
#include "OSEvent.h"
#include "ToolboxEvent.h"
#include "SegmentLdr.h"
#include "ScrapMgr.h"

//#include "keycode_map.h"

#include <iostream>
#include <memory>

namespace Executor
{
/* These variables are required by the vdriver interface. */
uint8_t *vdriver_fbuf;
int vdriver_row_bytes;
int vdriver_width = 1024;
int vdriver_height = 768;
int vdriver_bpp = 8, vdriver_log2_bpp;
int vdriver_max_bpp, vdriver_log2_max_bpp;
vdriver_modes_t *vdriver_mode_list;

int host_cursor_depth = 1;

    int fakeArgc = 1;
    char programName[] = "executor";
    char *fakeArgv[] = { programName, nullptr };
}

#undef white


using namespace Executor;

namespace
{
class ExecutorWindow;
vdriver_modes_t zero_modes = { 0, 0 };
QApplication *qapp;
QImage *qimage;
uint16_t keymod = 0;
ExecutorWindow *window;

class ExecutorWindow : public QRasterWindow
{
public:
    ExecutorWindow()
    {
        resize(vdriver_width, vdriver_height);
        setFlag(Qt::FramelessWindowHint, true);
        setFlag(Qt::NoDropShadowWindowHint, true);
    }

    void paintEvent(QPaintEvent *e)
    {
        QPainter painter(this);
        painter.drawImage(0,0, *qimage);
    }

    void mouseMoveEvent(QMouseEvent *ev)
    {
        LM(MouseLocation).h = CW(ev->x());
        LM(MouseLocation).v = CW(ev->y());

        adb_apeiron_hack(false);
    }
    void mousePressRelease(QMouseEvent *ev)
    {
        bool down_p;
        int32_t when;
        Point where;

        down_p = ev->buttons() & Qt::LeftButton;
        if(down_p)
            keymod &= ~btnState;
        else
            keymod |= btnState;
        when = TickCount();
        where.h = ev->x();
        where.v = ev->y();
        ROMlib_PPostEvent(down_p ? mouseDown : mouseUp,
                            0, (GUEST<EvQElPtr> *)0, when, where,
                            keymod);
        adb_apeiron_hack(false);
    }
    void mousePressEvent(QMouseEvent *ev)
    {
        mousePressRelease(ev);
    }
    void mouseReleaseEvent(QMouseEvent *ev)
    {
        mousePressRelease(ev);
    }
};

}

void Executor::vdriver_set_rootless_region(RgnHandle rgn)
{
    ThePortGuard guard;
    GrafPort grayRegionPort;

    C_OpenPort(&grayRegionPort);
    grayRegionPort.portBits.baseAddr = RM((Ptr) vdriver_fbuf + vdriver_row_bytes * vdriver_height * 4);
    grayRegionPort.portBits.rowBytes = CW( ((vdriver_width + 31) & ~31) / 8 );
    grayRegionPort.portBits.bounds = { CW(0), CW(0), CW(vdriver_height), CW(vdriver_width) };
    grayRegionPort.portRect = grayRegionPort.portBits.bounds;

    C_SetPort(&grayRegionPort);
    C_EraseRect(&grayRegionPort.portRect);
    C_PaintRgn(rgn);

    C_ClosePort(&grayRegionPort);

    window->setMask(QBitmap::fromData(QSize(vdriver_width, vdriver_height), (const uchar*)MR(grayRegionPort.portBits.baseAddr), QImage::Format_Mono));
}

void Executor::vdriver_opt_register(void)
{
}
bool Executor::vdriver_init(int _max_width, int _max_height, int _max_bpp,
                            bool fixed_p, int *argc, char *argv[])
{
    return true;
}

bool Executor::vdriver_acceptable_mode_p(int width, int height, int bpp,
                                         bool grayscale_p, bool exact_match_p)
{
    if(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 16 || bpp == 32)
        return true;
    else
        return false;
}


bool Executor::vdriver_set_mode(int width, int height, int bpp, bool grayscale_p)
{
    qapp = new QApplication(fakeArgc, fakeArgv);
    window = new ExecutorWindow();

    QScreen *screen = qapp->screens()[0];

    printf("set_mode: %d %d %d\n", width, height, bpp);
    if(vdriver_fbuf)
        delete[] vdriver_fbuf;

    vdriver_width = screen->availableGeometry().width();
    vdriver_height = screen->availableGeometry().height();
    if(width)
        vdriver_width = width;
    if(height)
        vdriver_height = height;
    if(bpp)
        vdriver_bpp = bpp;
    vdriver_row_bytes = vdriver_width * vdriver_bpp / 8;
    vdriver_log2_bpp = ROMlib_log2[vdriver_bpp];
    vdriver_mode_list = &zero_modes;

    vdriver_max_bpp = 8; //32;
    vdriver_log2_max_bpp = 3; //5;

    vdriver_fbuf = new uint8_t[vdriver_row_bytes * vdriver_height * 5];



    qimage = new QImage(vdriver_fbuf, vdriver_width, vdriver_height, vdriver_row_bytes, QImage::Format_Indexed8);

    window->resize(vdriver_width, vdriver_height);
    window->show();
    return true;
}
void Executor::vdriver_set_colors(int first_color, int num_colors, const ColorSpec *colors)
{
    QVector<QRgb> qcolors(num_colors);
    for(int i = 0; i < num_colors; i++)
    {
        qcolors[i] = qRgb(
            CW(colors[i].rgb.red) >> 8,
            CW(colors[i].rgb.green) >> 8,
            CW(colors[i].rgb.blue) >> 8
        );
    }
    qimage->setColorTable(qcolors);
}

void Executor::vdriver_get_colors(int first_color, int num_colors, ColorSpec *colors)
{
    QVector<QRgb> qcolors = qimage->colorTable();
    for(int i = 0; i < num_colors; i++)
    {
        colors[i].value = CW(first_color + i);
        int r = qRed(qcolors[i]), g = qGreen(qcolors[i]), b = qBlue(qcolors[i]);
        colors[i].rgb.red = CW(r << 8 | r);
        colors[i].rgb.green = CW(g << 8 | g);
        colors[i].rgb.blue = CW(b << 8 | b);
    }

}
void Executor::vdriver_update_screen_rects(int num_rects, const vdriver_rect_t *r,
                                           bool cursor_p)
{
    window->update();
}

void Executor::vdriver_update_screen(int top, int left, int bottom, int right,
                                     bool cursor_p)
{
    window->update();
}

void Executor::vdriver_flush_display(void)
{
}

void Executor::vdriver_shutdown(void)
{
}

void Executor::host_flush_shadow_screen(void)
{
}


void Executor::vdriver_pump_events()
{
    qapp->processEvents();
#if 0
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                LM(MouseLocation).h = CW(event.motion.x);
                LM(MouseLocation).v = CW(event.motion.y);

                adb_apeiron_hack(false);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                bool down_p;
                int32_t when;
                Point where;

                down_p = (event.button.state == SDL_PRESSED);
                if(down_p)
                    keymod &= ~btnState;
                else
                    keymod |= btnState;
                when = TickCount();
                where.h = event.button.x;
                where.v = event.button.y;
                ROMlib_PPostEvent(down_p ? mouseDown : mouseUp,
                                  0, (GUEST<EvQElPtr> *)0, when, where,
                                  keymod);
                adb_apeiron_hack(false);
            }
            break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                bool down_p;
                unsigned char mkvkey;
                uint16_t mod;
                LONGINT keywhat;
                int32_t when;
                Point where;

                init_sdlk_to_mkv();
                down_p = (event.key.state == SDL_PRESSED);

                /*if(use_scan_codes)
                    mkvkey = ibm_virt_to_mac_virt[event.key.keysym.scancode];
                else*/
                {
                    auto p = sdlk_to_mkv.find(event.key.keysym.sym);
                    if(p == sdlk_to_mkv.end())
                        mkvkey = NOTAKEY;
                    else
                        mkvkey = p->second;
                }
                mkvkey = ROMlib_right_to_left_key_map(mkvkey);
                if(isModifier(mkvkey, &mod))
                {
                    if(down_p)
                        keymod |= mod;
                    else
                        keymod &= ~mod;
                }
                when = TickCount();
                where.h = CW(LM(MouseLocation).h);
                where.v = CW(LM(MouseLocation).v);
                keywhat = ROMlib_xlate(mkvkey, keymod, down_p);
                post_keytrans_key_events(down_p ? keyDown : keyUp,
                                         keywhat, when, where,
                                         keymod, mkvkey);
            }
            break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                //if(!we_lost_clipboard())
                sendresumeevent(false);
                //else
                //{
                //    ZeroScrap();
                //    sendresumeevent(true);
                //}
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                sendsuspendevent();
                break;
            case SDL_QUIT:
                if(ConfirmQuit())
                    ExitToShell();
                break;
        }
    }
#endif
}

void Executor::ROMlib_SetTitle(char *title)
{
}

char *
Executor::ROMlib_GetTitle(void)
{
    static char str[] = "Foo";
    return str;
}

void Executor::ROMlib_FreeTitle(char *title)
{
}


void Executor::host_set_cursor(char *cursor_data,
                               unsigned short cursor_mask[16],
                               int hotspot_x, int hotspot_y)
{
    static QCursor theCursor(Qt::ArrowCursor);

    if(cursor_data)
    {
        uchar data2[32];
        uchar *mask2 = (uchar*)cursor_mask;
        std::copy(cursor_data, cursor_data+32, data2);
        for(int i = 0; i<32; i++)
            mask2[i] |= data2[i];
        QBitmap crsr = QBitmap::fromData(QSize(16, 16), (const uchar*)data2, QImage::Format_Mono);
        QBitmap mask = QBitmap::fromData(QSize(16, 16), (const uchar*)mask2, QImage::Format_Mono);
        
        theCursor = QCursor(crsr, mask, hotspot_x, hotspot_y);
    }
    window->setCursor(theCursor);   // TODO: should we check for visibility?
}

int Executor::host_set_cursor_visible(int show_p)
{
 /*   if(show_p)
        host_set_cursor(NULL, NULL, 0, 0);
    else
        window->setCursor(Qt::BlankCursor);*/
    return true;
}
