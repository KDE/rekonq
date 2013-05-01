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
#include "jsapi.h"
#include "jsapi.moc"


JSApi::JSApi(QObject *parent)
    : QObject(parent)
    , _bookmarks(new Bookmarks(this))
    , _cookies(new Cookies(this))
    , _history(new History(this))
    , _management(new Management(this))
    , _tabs(new Tabs(this))
    , _windows(new Windows(this))
{
    setObjectName("chrome");
    
    qRegisterMetaType<Bookmarks*>();
    qRegisterMetaType<Cookies*>();
    qRegisterMetaType<History*>();
    qRegisterMetaType<Management*>();
    qRegisterMetaType<Tabs*>();
    qRegisterMetaType<Windows*>();
}


Bookmarks* JSApi::bookmarks()
{
    kDebug() << "BOOKMARKS";
    return _bookmarks;
}


Cookies* JSApi::cookies()
{
    kDebug() << "COOKIES";
    return _cookies;
}


History* JSApi::history()
{
    kDebug() << "HISTORY";
    return _history;
}


Management* JSApi::management()
{
    kDebug() << "MANAGEMENT";
    return _management;
}


Tabs* JSApi::tabs()
{
    kDebug() << "TABS";
    return _tabs;
}


Windows* JSApi::windows()
{
    kDebug() << "WINDOWS";
    return _windows;
}
