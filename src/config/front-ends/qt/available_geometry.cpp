#include <QVector>
#include <QRect>
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <QEventLoop>
#include <QResizeEvent>

#include "host-os-config.h"

QVector<QRect> getScreenGeometries()
{
    QVector<QRect> geometries;
    for(QScreen *screen : QGuiApplication::screens())
        geometries.push_back(screen->geometry());

    return geometries;
}

#ifndef LINUX

/* Actually, this should be the Qt-on-anything-but-X11 case. */

QVector<QRect> getAvailableScreenGeometries()
{
    QVector<QRect> geometries;
    for(QScreen *screen : QGuiApplication::screens())
        geometries.push_back(screen->availableGeometry());

    return geometries;
}

#else

/* Figure out available screen geometries by opening
   invisible maximized windows on each screen.
   
   QScreen::availableGeometry() is documented not to work
   on multi-screen X11 systems.

   This version should still work even where it isn't needed.
 */

template <class F>
struct SizeTestWindow : QWindow
{
    F f;
    SizeTestWindow(F f)
        : f(f)
    {
    }
    void resizeEvent(QResizeEvent *evt) override
    {
        f(evt->size());
    }
};

template <class F>
QWindow* makeSizeTestWindow(F f) { return new SizeTestWindow<F>(f); }

QVector<QRect> getAvailableScreenGeometries()
{
    QVector<QWindow*> windows;
    int windowCount = 0;
    QEventLoop loop;

    for(QScreen *screen : QGuiApplication::screens())
    {
        ++windowCount;
        QWindow *window = makeSizeTestWindow(
            [&loop, &windowCount](QSize sz) {
                if(sz.width() > 100 && sz.height() > 100)
                {
                    if(--windowCount <= 0)
                        loop.exit();
                }
            }
        );
        windows.push_back(window);
        window->setFlag(Qt::FramelessWindowHint, true);
        window->setFlag(Qt::NoDropShadowWindowHint, true);
        window->setOpacity(0);
        window->resize(100,100);
        window->setPosition(screen->availableGeometry().topLeft());
        window->showMaximized();
    }
    loop.exec();

    QVector<QRect> geometries;
    for(QWindow *w : windows)
    {
        geometries.push_back(w->geometry());
        delete w;
    }
    return geometries;
}
#endif 
