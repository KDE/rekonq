/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "api_windows.h"
#include "api_windows.moc"

// Local Includes
#include "application.h"

#include "rekonqwindow.h"


Windows::Windows(QObject *parent)
    : QObject(parent)
{
    setObjectName("windows");    
}




RekonqWindow *Windows::windowFor(int winId)
{
    // with negative numbers, return current window
    if (winId < 0) 
    {
        return Application::instance()->rekonqWindow();
    }
 
    WindowId wId = qAbs(winId);
    
    RekonqWindowList wList = rApp->rekonqWindowList();
    Q_FOREACH(const QWeakPointer<RekonqWindow> ptr, wList)
    {
        RekonqWindow *w = ptr.data();
        if (w->id() == wId)
            return w;
    }
    
    return 0;
}


// -------------------------- CHROME.WINDOWS API ----------------------------


// Gets details about a window.
QVariantMap Windows::get(WindowId windowId, QVariantMap getInfo /*, function callback */ )
{
    Q_UNUSED(getInfo);

    RekonqWindow *theWindow = windowFor(windowId);
    
    if (!theWindow)
        return QVariantMap();

    QVariantMap map;

    map["id"] = theWindow->id();

    map["incognito"] = theWindow->isPrivateBrowsingMode();

    return map;
}


// Gets the current window.
QVariantMap Windows::getCurrent(QVariantMap getInfo /*, function callback */ )
{
    Q_UNUSED(getInfo);

    RekonqWindow *window = rApp->rekonqWindow();
    if (!window)
        return QVariantMap();
    
    return get(window->id());    
}


// Gets the window that was most recently focused — typically the window 'on top'.
QVariantMap Windows::getLastFocused(QVariantMap getInfo /*, function callback */ )
{
    Q_UNUSED(getInfo);

    RekonqWindow *window = rApp->rekonqWindow();
    if (!window)
        return QVariantMap();
    
    return get(window->id());    
}


// Gets all windows.
QVariantMap Windows::getAll(QVariantMap getInfo /*, function callback */ )
{
    kDebug() << "UNIMPLEMENTED";
    Q_UNUSED(getInfo);

    return QVariantMap();
}


// Creates (opens) a new browser with any optional sizing, position or default URL provided.
void Windows::create(QVariantMap createData /*, function callback */ )
{
    Q_UNUSED(createData);
    
    rApp->newWindow();
}


// Updates the properties of a window. Specify only the properties that you want to change; unspecified properties will be left unchanged.
QVariantMap Windows::update(WindowId windowId, QVariantMap updateInfo /*, function callback */)
{
    Q_UNUSED(updateInfo);

    RekonqWindow *theWindow = windowFor(windowId);
    
    if (!theWindow)
        return QVariantMap();

    kDebug() << "UNIMPLEMENTED";
    return QVariantMap();
}


// Removes (closes) a window, and all the tabs inside it.
QVariantMap Windows::remove(WindowId windowId /*, function callback */)
{
    RekonqWindow *theWindow = windowFor(windowId);
    
    if (!theWindow)
        return QVariantMap();

    QVariantMap map = get(windowId);
    
    theWindow->close();
    return map;
}
