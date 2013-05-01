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


#ifndef JS_API_H
#define JS_API_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "api_bookmarks.h"
#include "api_cookies.h"
#include "api_history.h"
#include "api_management.h"
#include "api_tabs.h"
#include "api_windows.h"

// Qt Includes
#include <QObject>


class REKONQ_TESTS_EXPORT JSApi : public QObject
{
    Q_OBJECT

public:
    JSApi(QObject *parent = 0);

public Q_SLOTS:
    Bookmarks* bookmarks();
    Cookies* cookies();
    History* history();
    Management* management();
    Tabs* tabs();
    Windows* windows();

private:
    Bookmarks* _bookmarks;
    Cookies* _cookies;
    History* _history;
    Management* _management;
    Tabs* _tabs;
    Windows* _windows;
};


Q_DECLARE_METATYPE(Bookmarks*)
Q_DECLARE_METATYPE(Cookies*)
Q_DECLARE_METATYPE(History*)
Q_DECLARE_METATYPE(Management*)
Q_DECLARE_METATYPE(Tabs*)
Q_DECLARE_METATYPE(Windows*)

#endif // JS_API_H
