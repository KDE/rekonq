/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007 David Faure <faure@kde.org>
* Copyright (C) 2009-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef REKONQ_DEFINES_H
#define REKONQ_DEFINES_H


// ----------------------------------------------------------------------------------------------------
// UNIT TESTS NEED

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

/* Classes from the rekonq application, which are exported only for unit tests */
#ifndef REKONQ_TESTS_EXPORT
/* We are building this library */
#define REKONQ_TESTS_EXPORT KDE_EXPORT
#else
/* We are using this library */
#define REKONQ_TESTS_EXPORT KDE_IMPORT
#endif


// ----------------------------------------------------------------------------------------------------
// DEFINES

#define QL1S(x)  QLatin1String(x)
#define QL1C(x)  QLatin1Char(x)

#ifndef ASSERT_NOT_REACHED
#  ifndef QT_NO_DEBUG
#    define ASSERT_NOT_REACHED(msg) qt_assert(#msg,__FILE__,__LINE__); kDebug() << #msg
#  else
#    define ASSERT_NOT_REACHED(msg) kDebug() << #msg
#  endif
#endif //ASSERT_NOT_REACHED


// --------------------------------------------------------------------------------------
// ENUMS


namespace Rekonq
{

/**
* @short Open link options
* Different modes of opening new tab
*/
enum OpenType
{
    CurrentTab,         ///< open url in current tab
    NewTab,             ///< open url according to users settings
    NewFocusedTab,      ///< open url in new tab and focus it
    NewBackGroundTab,   ///< open url in new background tab
    NewWindow,          ///< open url in new window
    NewPrivateWindow,   ///< open url in new private window
    WebApp              ///< open url in a web app window
};

/**
* @short data to be synced
* Different data we can sync
*/
enum SyncData
{
    Bookmarks,
    History,
    Passwords
};

}


// ----------------------------------------------------------------------------------------------------
// INCLUDES

#include <KDebug>



// ----------------------------------------------------------------------------------------------------

#endif  // REKONQ_DEFINES_H
