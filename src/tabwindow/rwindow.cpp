/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2014 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


// Self Includes
#include "rwindow.h"
#include "rwindow.moc"

// KDE Includes
#include <KApplication>
#include <KCmdLineArgs>
#include <KSessionManager>

// Qt Includes
#include <QCloseEvent>
#include <QDesktopWidget>


static bool s_no_query_exit = false;


class KRWSessionManager : public KSessionManager
{

public:
    KRWSessionManager()
    {
    }

    ~KRWSessionManager()
    {
    }

    bool dummyInit()
    {
        return true;
    }

    bool saveState(QSessionManager&)
    {
        KConfig* config = KApplication::kApplication()->sessionConfig();
        int n = 0;
        Q_FOREACH(RWindow * rw, RWindow::windowList())
        {
            n++;
            rw->savePropertiesInternal(config, n);
        }

        KConfigGroup group(config, "Number");
        group.writeEntry("NumberOfWindows", n);
        return true;
    }

    bool commitData(QSessionManager& sm)
    {
        // not really a fast method but the only compatible one
        if (sm.allowsInteraction())
        {
            bool canceled = false;
            ::s_no_query_exit = true;

            Q_FOREACH(RWindow * window, RWindow::windowList())
            {
                if (!window->testAttribute(Qt::WA_WState_Hidden))
                {
                    QCloseEvent e;
                    QApplication::sendEvent(window, &e);
                    canceled = !e.isAccepted();
                    if (canceled)
                        break;
                }
            }
            ::s_no_query_exit = false;
            if (canceled)
                return false;

            return true;
        }

        // the user wants it, the user gets it
        return true;
    }
};


K_GLOBAL_STATIC(KRWSessionManager, ktwsm)
K_GLOBAL_STATIC(QList<RWindow*>, sWindowList)


// ----------------------------------------------------------------------------------------------------


RWindow::RWindow(QWidget* parent)
    : QWidget(parent)
{
    // This has to be a window...
    setWindowFlags(Qt::Window);

    // Setting attributes (just to be sure...)
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_QuitOnClose, true);

    ktwsm->dummyInit();
    sWindowList->append(this);

    QString geometry;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    if (args && args->isSet("geometry"))
        geometry = args->getOption("geometry");

    if (geometry.isNull())    // if there is no geometry, it doesn't matter
    {
        KSharedConfig::Ptr cf = KGlobal::config();
        KConfigGroup cg(cf, QL1S("RekonqWindow"));
        restoreWindowSize(cg);
    }
    else
    {
        parseGeometry();
    }

    setWindowTitle(KGlobal::caption());
}


RWindow::~RWindow()
{
    sWindowList->removeAll(this);

    KSharedConfig::Ptr cf = KGlobal::config();
    KConfigGroup cg(cf, QL1S("RekonqWindow"));
    saveWindowSize(cg);
}


QSize RWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.8;
    return size;
}

QList<RWindow*> RWindow::windowList()
{
    return *sWindowList;
}


void RWindow::savePropertiesInternal(KConfig *config, int number)
{
    QString s;
    s.setNum(number);
    s.prepend(QL1S("WindowProperties"));
    KConfigGroup cg(config, s);

    // store objectName, className, Width and Height  for later restoring
    // (Only useful for session management)
    cg.writeEntry(QL1S("ObjectName"), objectName());
    cg.writeEntry(QL1S("ClassName"), metaObject()->className());

    saveWindowSize(cg);

    s.setNum(number);
    cg = KConfigGroup(config, s);
    saveProperties(cg);
}


bool RWindow::readPropertiesInternal(KConfig *config, int number)
{
    // in order they are in toolbar list
    QString s;
    s.setNum(number);
    s.prepend(QL1S("WindowProperties"));

    KConfigGroup cg(config, s);

    // restore the object name (window role)
    if (cg.hasKey(QL1S("ObjectName")))
        setObjectName(cg.readEntry("ObjectName").toLatin1());  // latin1 is right here

    restoreWindowSize(cg);

    s.setNum(number);
    KConfigGroup grp(config, s);
    readProperties(grp);

    return true;
}


void RWindow::restoreWindowSize(const KConfigGroup & _cg)
{
    int scnum = QApplication::desktop()->screenNumber(window());
    QRect desktopRect = QApplication::desktop()->screenGeometry(scnum);

    KConfigGroup cg(_cg);

    QString geometryKey = QString::fromLatin1("geometry-%1-%2").arg(desktopRect.width()).arg(desktopRect.height());
    QByteArray geometry = cg.readEntry(geometryKey, QByteArray());

    // if first time run, center window: resize && move..
    if (!restoreGeometry(QByteArray::fromBase64(geometry)))
    {
        QSize defaultSize = desktopRect.size() * 0.8;
        resize(defaultSize);

        move((desktopRect.width() - width()) / 2, (desktopRect.height() - height()) / 2);
    }

    checkPosition();
}


void RWindow::saveWindowSize(const KConfigGroup & _cg) const
{
    int scnum = QApplication::desktop()->screenNumber(window());
    QRect desktopRect = QApplication::desktop()->screenGeometry(scnum);

    int w, h;
    if (isMaximized())
    {
        w = desktopRect.width() + 1;
        h = desktopRect.height() + 1;
    }
    else
    {
        w = width();
        h = height();
    }

    KConfigGroup cg(_cg);

    QString widthString = QString::fromLatin1("Width %1").arg(desktopRect.width());
    cg.writeEntry(widthString, w);

    QString heightString = QString::fromLatin1("Height %1").arg(desktopRect.height());
    cg.writeEntry(heightString, h);

    // geometry is saved separately for each resolution
    QString geometryKey = QString::fromLatin1("geometry-%1-%2").arg(desktopRect.width()).arg(desktopRect.height());
    QByteArray geometry = saveGeometry();
    cg.writeEntry(geometryKey, geometry.toBase64());
}


void RWindow::parseGeometry()
{
    QString cmdlineGeometry;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    if (args->isSet("geometry"))
        cmdlineGeometry = args->getOption("geometry");

    Q_ASSERT(!cmdlineGeometry.isNull());

// #if defined Q_WS_X11
//     int x, y;
//     int w, h;
//     int m = XParseGeometry( cmdlineGeometry.toLatin1(), &x, &y, (unsigned int*)&w, (unsigned int*)&h);
//     if (parsewidth) {
//         const QSize minSize = minimumSize();
//         const QSize maxSize = maximumSize();
//         if ( !(m & WidthValue) )
//             w = width();
//         if ( !(m & HeightValue) )
//             h = height();
//          w = qMin(w,maxSize.width());
//          h = qMin(h,maxSize.height());
//          w = qMax(w,minSize.width());
//          h = qMax(h,minSize.height());
//          resize(w, h);
//     } else {
//         if ( (m & XNegative) )
//             x = KApplication::desktop()->width()  + x - w;
//         else if ( (m & XValue) )
//             x = geometry().x();
//         if ( (m & YNegative) )
//             y = KApplication::desktop()->height() + y - h;
//         else if ( (m & YValue) )
//             y = geometry().y();
//
//         move(x, y);
//     }
// #endif
}


void RWindow::resizeEvent(QResizeEvent *event)
{
    if (!isFullScreen())
        saveAutoSaveSettings();
    QWidget::resizeEvent(event);
}


void RWindow::saveAutoSaveSettings()
{
    kDebug() << "AUTO SAVING SETTINGS...";

    KSharedConfig::Ptr cf = KGlobal::config();
    KConfigGroup cg(cf, QL1S("RekonqWindow"));
    saveWindowSize(cg);
}


bool RWindow::canBeRestored(int number)
{
    if (!qApp->isSessionRestored())
        return false;
    KConfig *config = kapp->sessionConfig();
    if (!config)
        return false;

    KConfigGroup group(config, "Number");
    const int n = group.readEntry("NumberOfWindows", 1);
    return number >= 1 && number <= n;
}


bool RWindow::restore(int number, bool show)
{
    if (!canBeRestored(number))
        return false;
    KConfig *config = kapp->sessionConfig();
    if (readPropertiesInternal(config, number))
    {
        if (show)
            RWindow::show();
        return true;
    }
    return false;
}


void RWindow::checkPosition()
{
    // no need to check trivial positions...
    if (isMaximized())
        return;

    QList<RWindow*> wList = RWindow::windowList();
    int wNumber = wList.count();

    // no need to check first window...
    if (wNumber <= 1)
        return;

    int div = wNumber % 4;

    int scnum = QApplication::desktop()->screenNumber(window());
    QRect desktopRect = QApplication::desktop()->screenGeometry(scnum);

    switch (div)
    {
    case 2:
        // left down
        move(desktopRect.width() - width(),  desktopRect.height() - height());
        break;
    case 3:
        // right down
        move(0, desktopRect.height() - height());
        break;
    case 0:
        // left top
        move(desktopRect.width() - width(), 0);
        break;
    case 1:
        // right top
        move(0, 0);
        break;
    default:
        kDebug() << "OOPS...THIS SHOULD NEVER HAPPEN!!";
        break;
    }
}
