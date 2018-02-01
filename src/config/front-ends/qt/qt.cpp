
#include <QGuiApplication>
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
#include "rsys/refresh.h"

#include "available_geometry.h"

//#include "keycode_map.h"

#include <iostream>
#include <memory>
#include <unordered_map>

#ifdef MACOSX
void macosx_hide_menu_bar(int mouseY);
#endif
#include "../x/x_keycodes.h"

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
QGuiApplication *qapp;
QImage *qimage;
uint16_t keymod = 0;
ExecutorWindow *window;


std::unordered_map<Qt::Key,int> keycodeMap {
    { Qt::Key_Backspace, MKV_BACKSPACE },
    { Qt::Key_Tab, MKV_TAB },
    //{ Qt::Key_Clear, NOTAKEY },
    { Qt::Key_Return, MKV_RETURN },
    { Qt::Key_Escape, MKV_ESCAPE },
    { Qt::Key_Space, MKV_SPACE },
    { Qt::Key_Apostrophe, MKV_TICK },
    { Qt::Key_Comma, MKV_COMMA },
    { Qt::Key_Minus, MKV_MINUS },
    { Qt::Key_Period, MKV_PERIOD },
    { Qt::Key_Slash, MKV_SLASH },
    { Qt::Key_0, MKV_0 },
    { Qt::Key_1, MKV_1 },
    { Qt::Key_2, MKV_2 },
    { Qt::Key_3, MKV_3 },
    { Qt::Key_4, MKV_4 },
    { Qt::Key_5, MKV_5 },
    { Qt::Key_6, MKV_6 },
    { Qt::Key_7, MKV_7 },
    { Qt::Key_8, MKV_8 },
    { Qt::Key_9, MKV_9 },
    { Qt::Key_Semicolon, MKV_SEMI },
    { Qt::Key_Colon, MKV_SEMI },
    { Qt::Key_Equal, MKV_EQUAL },
    /*{ Qt::Key_Kp_0, MKV_NUM0 },
    { Qt::Key_Kp_1, MKV_NUM1 },
    { Qt::Key_Kp_2, MKV_NUM2 },
    { Qt::Key_Kp_3, MKV_NUM3 },
    { Qt::Key_Kp_4, MKV_NUM4 },
    { Qt::Key_Kp_5, MKV_NUM5 },
    { Qt::Key_Kp_6, MKV_NUM6 },
    { Qt::Key_Kp_7, MKV_NUM7 },
    { Qt::Key_Kp_8, MKV_NUM8 },
    { Qt::Key_Kp_9, MKV_NUM9 },
    { Qt::Key_Kp_PERIOD, MKV_NUMPOINT },
    { Qt::Key_Kp_DIVIDE, MKV_NUMDIVIDE },
    { Qt::Key_Kp_MULTIPLY, MKV_NUMMULTIPLY },
    { Qt::Key_Kp_MINUS, MKV_NUMMINUS },
    { Qt::Key_Kp_PLUS, MKV_NUMPLUS },*/
    { Qt::Key_Enter, MKV_NUMENTER },
    { Qt::Key_BracketLeft, MKV_LEFTBRACKET },
    { Qt::Key_Backslash, MKV_BACKSLASH },
    { Qt::Key_BracketRight, MKV_RIGHTBRACKET },
    { Qt::Key_QuoteLeft, MKV_BACKTICK },
    { Qt::Key_BraceLeft, MKV_LEFTBRACKET },
    { Qt::Key_BraceRight, MKV_RIGHTBRACKET },
    { Qt::Key_section, MKV_PARAGRAPH },
    { Qt::Key_A, MKV_a },
    { Qt::Key_B, MKV_b },
    { Qt::Key_C, MKV_c },
    { Qt::Key_D, MKV_d },
    { Qt::Key_E, MKV_e },
    { Qt::Key_F, MKV_f },
    { Qt::Key_G, MKV_g },
    { Qt::Key_H, MKV_h },
    { Qt::Key_I, MKV_i },
    { Qt::Key_J, MKV_j },
    { Qt::Key_K, MKV_k },
    { Qt::Key_L, MKV_l },
    { Qt::Key_M, MKV_m },
    { Qt::Key_N, MKV_n },
    { Qt::Key_O, MKV_o },
    { Qt::Key_P, MKV_p },
    { Qt::Key_Q, MKV_q },
    { Qt::Key_R, MKV_r },
    { Qt::Key_S, MKV_s },
    { Qt::Key_T, MKV_t },
    { Qt::Key_U, MKV_u },
    { Qt::Key_V, MKV_v },
    { Qt::Key_W, MKV_w },
    { Qt::Key_X, MKV_x },
    { Qt::Key_Y, MKV_y },
    { Qt::Key_Z, MKV_z },
    { Qt::Key_Delete, MKV_DELFORWARD },
    { Qt::Key_F1, MKV_F1 },
    { Qt::Key_F2, MKV_F2 },
    { Qt::Key_F3, MKV_F3 },
    { Qt::Key_F4, MKV_F4 },
    { Qt::Key_F5, MKV_F5 },
    { Qt::Key_F6, MKV_F6 },
    { Qt::Key_F7, MKV_F7 },
    { Qt::Key_F8, MKV_F8 },
    { Qt::Key_F9, MKV_F9 },
    { Qt::Key_F10, MKV_F10 },
    { Qt::Key_F11, MKV_F11 },
    { Qt::Key_F12, MKV_F12 },
    { Qt::Key_F13, MKV_F13 },
    { Qt::Key_F14, MKV_F14 },
    { Qt::Key_F15, MKV_F15 },
    { Qt::Key_Pause, MKV_PAUSE },
    { Qt::Key_NumLock, MKV_NUMCLEAR },
    { Qt::Key_Up, MKV_UPARROW },
    { Qt::Key_Down, MKV_DOWNARROW },
    { Qt::Key_Right, MKV_RIGHTARROW },
    { Qt::Key_Left, MKV_LEFTARROW },
    { Qt::Key_Insert, MKV_HELP },
    { Qt::Key_Home, MKV_HOME },
    { Qt::Key_End, MKV_END },
    { Qt::Key_PageUp, MKV_PAGEUP },
    { Qt::Key_PageDown, MKV_PAGEDOWN },
    { Qt::Key_CapsLock, MKV_CAPS },
    { Qt::Key_ScrollLock, MKV_SCROLL_LOCK },
    { Qt::Key_Shift, MKV_LEFTSHIFT },
#ifdef MACOSX
    { Qt::Key_Control, MKV_CLOVER },
    { Qt::Key_Alt, MKV_LEFTOPTION },
    { Qt::Key_Meta, MKV_LEFTCNTL },
#else
    { Qt::Key_Control, MKV_CLOVER },
    { Qt::Key_Alt, MKV_LEFTOPTION },
    { Qt::Key_Meta, MKV_LEFTCNTL },
#endif
    //{ Qt::Key_Help, MKV_HELP },
    { Qt::Key_Print, MKV_PRINT_SCREEN },
};


class ExecutorWindow : public QRasterWindow
{
public:
    ExecutorWindow()
    {
        setFlag(Qt::FramelessWindowHint, true);
        setFlag(Qt::NoDropShadowWindowHint, true);
    }

    void paintEvent(QPaintEvent *e)
    {
        QPainter painter(this);
        if(qimage)
        {
            for(const QRect& r : e->region())
                painter.drawImage(r, *qimage, r);
        }
    }

    void mouseMoveEvent(QMouseEvent *ev)
    {
#ifdef MACOSX
        macosx_hide_menu_bar(ev->y());
#endif
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
    
    void keyEvent(QKeyEvent *ev, bool down_p)
    {
        unsigned char mkvkey;
        uint16_t mod;
        LONGINT keywhat;
        int32_t when;
        Point where;

        auto p = keycodeMap.find(Qt::Key(ev->key()));
        if(p == keycodeMap.end())
            mkvkey = 0x89;// NOTAKEY
        else
            mkvkey = p->second;
        if(ev->nativeScanCode() > 1 && ev->nativeScanCode() < NELEM(x_keycode_to_mac_virt))
        {
            mkvkey = x_keycode_to_mac_virt[ev->nativeScanCode()];
        }
#ifdef MACOSX
        if(ev->nativeVirtualKey())
            mkvkey = ev->nativeVirtualKey();
#endif
        mkvkey = ROMlib_right_to_left_key_map(mkvkey);
        keymod &= ~(shiftKey | ControlKey | cmdKey | optionKey);
        Qt::KeyboardModifiers qtmods = ev->modifiers();
        if(qtmods & Qt::ShiftModifier)
            keymod |= shiftKey;
#if true || defined(MACOSX)
        if(qtmods & Qt::ControlModifier)
            keymod |= cmdKey;
        if(qtmods & Qt::AltModifier)
            keymod |= optionKey;
        if(qtmods & Qt::MetaModifier)
            keymod |= ControlKey;
#else
        if(qtmods & Qt::ControlModifier)
            keymod |= ControlKey;
        if(qtmods & Qt::AltModifier)
            keymod |= cmdKey;
        if(qtmods & Qt::MetaModifier)
            keymod |= optionKey;
#endif
        if(mkvkey == MKV_CAPS)
        {
            if(down_p)
                keymod |= alphaLock;
            else
                keymod &= ~alphaLock;
        }
        when = TickCount();
        where.h = CW(LM(MouseLocation).h);
        where.v = CW(LM(MouseLocation).v);
        keywhat = ROMlib_xlate(mkvkey, keymod, down_p);
        post_keytrans_key_events(down_p ? keyDown : keyUp,
                             keywhat, when, where,
                             keymod, mkvkey);

    }
    
    void keyPressEvent(QKeyEvent *ev)
    {
        std::cout << "press: " << std::hex << ev->key() << " " << ev->nativeScanCode() << " " << ev->nativeVirtualKey() << std::dec << std::endl;
        if(!ev->isAutoRepeat())
            keyEvent(ev, true);
    }
    void keyReleaseEvent(QKeyEvent *ev)
    {
        std::cout << "release\n";
        if(!ev->isAutoRepeat())
            keyEvent(ev, false);
    }

    bool event(QEvent *ev)
    {
        switch(ev->type())
        {
            case QEvent::FocusIn:
                sendresumeevent(true);
                break;
            case QEvent::FocusOut:
                sendsuspendevent();
                break;

            default:
                ;
        }
        return QRasterWindow::event(ev);
    }
};

}

void Executor::vdriver_set_rootless_region(RgnHandle rgn)
{
    ThePortGuard guard;
    GrafPort grayRegionPort;

    C_OpenPort(&grayRegionPort);
    short rowBytes = ((vdriver_width + 31) & ~31) / 8;
    grayRegionPort.portBits.baseAddr = RM((Ptr) vdriver_fbuf + vdriver_row_bytes * vdriver_height * 4);
    grayRegionPort.portBits.rowBytes = CW( rowBytes );
    grayRegionPort.portBits.bounds = { CW(0), CW(0), CW(vdriver_height), CW(vdriver_width) };
    grayRegionPort.portRect = grayRegionPort.portBits.bounds;

    C_SetPort(&grayRegionPort);
    C_EraseRect(&grayRegionPort.portRect);
    C_PaintRgn(rgn);

    C_ClosePort(&grayRegionPort);

    window->setMask(QBitmap::fromData(
        QSize((vdriver_width + 31)&~31, vdriver_height),
        (const uchar*)MR(grayRegionPort.portBits.baseAddr), QImage::Format_Mono));
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
    qapp = new QGuiApplication(fakeArgc, fakeArgv);
#ifdef MACOSX
    macosx_hide_menu_bar(0);
    QVector<QRect> screenGeometries = getScreenGeometries();
#else
    QVector<QRect> screenGeometries = getAvailableScreenGeometries();
#endif

    printf("set_mode: %d %d %d\n", width, height, bpp);
    if(vdriver_fbuf)
        delete[] vdriver_fbuf;
    
    QRect geom = screenGeometries[0];

    vdriver_width = geom.width();
    vdriver_height = geom.height();
    if(width)
        vdriver_width = width;
    if(height)
        vdriver_height = height;
    if(bpp)
        vdriver_bpp = bpp;
    vdriver_row_bytes = vdriver_width * vdriver_bpp / 8;
    vdriver_row_bytes = (vdriver_row_bytes+3) & ~3;
    vdriver_log2_bpp = ROMlib_log2[vdriver_bpp];
    vdriver_mode_list = &zero_modes;

    vdriver_max_bpp = 8; //32;
    vdriver_log2_max_bpp = 3; //5;

    vdriver_fbuf = new uint8_t[vdriver_row_bytes * vdriver_height * 5];

    qimage = new QImage(vdriver_fbuf, vdriver_width, vdriver_height, vdriver_row_bytes,
        vdriver_bpp == 1 ? QImage::Format_Mono : QImage::Format_Indexed8);
    qimage->setColorTable({qRgb(0,0,0),qRgb(255,255,255)});

    window = new ExecutorWindow();
    window->setGeometry(geom);
#ifdef MACOSX
    window->show();//Maximized();
#else
    window->showMaximized();
#endif
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
#ifdef MACOSX   /* something went wrong on the mac; quick fix */
    window->update();
#else
    QRegion rgn;
    for(int i = 0; i < num_rects; i++)
    {
        rgn += QRect(r[i].left, r[i].top, r[i].right-r[i].left, r[i].bottom-r[i].top);
    }
    window->update(rgn);
#endif
}

void Executor::vdriver_update_screen(int top, int left, int bottom, int right,
                                     bool cursor_p)
{
#ifdef MACOSX   /* something went wrong on the mac; quick fix */
    window->update();
#else
    window->update(QRect(left, top, right-left, bottom-top));
#endif
}

void Executor::vdriver_flush_display(void)
{
}

void Executor::vdriver_shutdown(void)
{
}

void Executor::vdriver_pump_events()
{
    qapp->processEvents();
#if 0
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
    if(show_p)
        host_set_cursor(NULL, NULL, 0, 0);
    else
        window->setCursor(Qt::BlankCursor);
    return true;
}
