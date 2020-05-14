/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qglobal.h"
#include "qdesktopwidget.h"
#include "qdesktopwidget_p.h"
#include "qscreen.h"
#include "qwidget_p.h"
#include "qwindow.h"

#include <private/qhighdpiscaling_p.h>
#include <qpa/qplatformscreen.h>

QT_BEGIN_NAMESPACE

QDesktopScreenWidget::QDesktopScreenWidget(QScreen *screen, const QRect &geometry)
    : QWidget(nullptr, Qt::Desktop), m_screen(screen)
{
    setVisible(false);
    if (QWindow *winHandle = windowHandle())
        winHandle->setScreen(screen);
    setScreenGeometry(geometry);
}

void QDesktopScreenWidget::setScreenGeometry(const QRect &geometry)
{
    m_geometry = geometry;
    setGeometry(geometry);
}

int QDesktopScreenWidget::screenNumber() const
{
    const QDesktopWidgetPrivate *desktopWidgetP
        = static_cast<const QDesktopWidgetPrivate *>(qt_widget_private(QApplication::desktop()));
    return desktopWidgetP->screens.indexOf(const_cast<QDesktopScreenWidget *>(this));
}

const QRect QDesktopWidgetPrivate::screenGeometry(const QWidget *widget)
{
    if (Q_UNLIKELY(!widget)) {
        qWarning("QDesktopWidget::screenGeometry(): Attempt "
                 "to get the screen geometry of a null widget");
        return QRect();
    }
    QRect rect = QWidgetPrivate::screenGeometry(widget);
    if (rect.isNull())
        return screenGeometry(screenNumber(widget));
    else return rect;
}

const QRect QDesktopWidgetPrivate::availableGeometry(const QWidget *widget)
{
    if (Q_UNLIKELY(!widget)) {
        qWarning("QDesktopWidget::availableGeometry(): Attempt "
                 "to get the available geometry of a null widget");
        return QRect();
    }
    QRect rect = QWidgetPrivate::screenGeometry(widget);
    if (rect.isNull())
        return availableGeometry(screenNumber(widget));
    else
        return rect;
}

QDesktopScreenWidget *QDesktopWidgetPrivate::widgetForScreen(QScreen *qScreen) const
{
    foreach (QDesktopScreenWidget *widget, screens) {
        if (widget->assignedScreen() == qScreen)
            return widget;
    }
    return nullptr;
}

void QDesktopWidgetPrivate::_q_updateScreens()
{
    Q_Q(QDesktopWidget);
    const QList<QScreen *> screenList = QGuiApplication::screens();
    const int targetLength = screenList.length();

    // Re-build our screens list. This is the easiest way to later compute which signals to emit.
    // Create new screen widgets as necessary. While iterating, keep the old list in place so
    // that widgetForScreen works.
    // Furthermore, we note which screens have changed, and compute the overall virtual geometry.
    QList<QDesktopScreenWidget *> newScreens;
    QList<int> changedScreens;
    QRegion virtualGeometry;

    for (int i = 0; i < targetLength; ++i) {
        QScreen *qScreen = screenList.at(i);
        const QRect screenGeometry = qScreen->geometry();
        QDesktopScreenWidget *screenWidget = widgetForScreen(qScreen);
        if (screenWidget) {
            // an old screen. update geometry and remember the index in the *new* list
            if (screenGeometry != screenWidget->screenGeometry()) {
                screenWidget->setScreenGeometry(screenGeometry);
                changedScreens.push_back(i);
            }
        } else {
            // a new screen, create a widget and connect the signals.
            screenWidget = new QDesktopScreenWidget(qScreen, screenGeometry);
            QObject::connect(qScreen, SIGNAL(geometryChanged(QRect)),
                             q, SLOT(_q_updateScreens()), Qt::QueuedConnection);
            QObject::connect(qScreen, SIGNAL(destroyed()),
                             q, SLOT(_q_updateScreens()), Qt::QueuedConnection);
        }
        // record all the screens and the overall geometry.
        newScreens.push_back(screenWidget);
        virtualGeometry += screenGeometry;
    }

    // Now we apply the accumulated updates.
    screens.swap(newScreens); // now [newScreens] is the old screen list
    Q_ASSERT(screens.size() == targetLength);
    q->setGeometry(virtualGeometry.boundingRect());

    // Delete the QDesktopScreenWidget that are not used any more.
    foreach (QDesktopScreenWidget *screen, newScreens) {
        if (!screens.contains(screen))
            delete screen;
    }
}

QDesktopWidget::QDesktopWidget()
    : QWidget(*new QDesktopWidgetPrivate, nullptr, Qt::Desktop)
{
    Q_D(QDesktopWidget);
    setObjectName(QLatin1String("desktop"));
    d->_q_updateScreens();
    connect(qApp, SIGNAL(screenAdded(QScreen*)), this, SLOT(_q_updateScreens()));
}

QDesktopWidget::~QDesktopWidget()
{
}

QWidget *QDesktopWidget::screen(int screen)
{
    Q_D(QDesktopWidget);
    if (screen < 0 || screen >= d->screens.length())
        return d->screens.at(0);
    return d->screens.at(screen);
}

const QRect QDesktopWidgetPrivate::availableGeometry(int screenNo)
{
    const QScreen *screen = QDesktopWidgetPrivate::screen(screenNo);
    return screen ? screen->availableGeometry() : QRect();
}

const QRect QDesktopWidgetPrivate::screenGeometry(int screenNo)
{
    const QScreen *screen = QDesktopWidgetPrivate::screen(screenNo);
    return screen ? screen->geometry() : QRect();
}

int QDesktopWidgetPrivate::screenNumber(const QWidget *w)
{
    if (!w)
        return 0;

    const QList<QScreen *> allScreens = QGuiApplication::screens();
    QList<QScreen *> screens = allScreens;
    if (screens.isEmpty()) // This should never happen
        return 0;

    // If there is more than one virtual desktop
    if (screens.count() != screens.constFirst()->virtualSiblings().count()) {
        // Find the root widget, get a QScreen from it and use the
        // virtual siblings for checking the window position.
        if (const QScreen *winScreen = qt_widget_private(const_cast<QWidget *>(w))->associatedScreen())
            screens = winScreen->virtualSiblings();
    }

    // Get the screen number from window position using screen geometry
    // and proper screens.
    QRect frame = w->frameGeometry();
    if (!w->isWindow())
        frame.moveTopLeft(w->mapToGlobal(QPoint(0, 0)));

    QScreen *widgetScreen = nullptr;
    int largestArea = 0;
    foreach (QScreen *screen, screens) {
        const QRect deviceIndependentScreenGeometry =
            QHighDpi::fromNativePixels(screen->handle()->geometry(), screen);
        const QRect intersected = deviceIndependentScreenGeometry.intersected(frame);
        int area = intersected.width() * intersected.height();
        if (largestArea < area) {
            widgetScreen = screen;
            largestArea = area;
        }
    }
    return allScreens.indexOf(widgetScreen);
}

int QDesktopWidgetPrivate::screenNumber(const QPoint &p)
{
    QScreen *screen = QGuiApplication::screenAt(p);
    return screen ? QGuiApplication::screens().indexOf(screen) : 0;
}

QScreen *QDesktopWidgetPrivate::screen(int screenNo)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screenNo == -1)
        screenNo = 0;
    if (screenNo < 0 || screenNo >= screens.size())
        return nullptr;
    return screens.at(screenNo);
}

QT_END_NAMESPACE

#include "moc_qdesktopwidget.cpp"
#include "moc_qdesktopwidget_p.cpp"
