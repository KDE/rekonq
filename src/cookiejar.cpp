/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "cookiejar.h"
#include "cookiejar.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "autosaver.h"

// KDE Includes
#include <KConfig>
#include <KStandardDirs>
#include <KDebug>

// Qt Includes
#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QString>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>


CookieJar::CookieJar(QObject* parent)
    : QNetworkCookieJar(parent)
    , m_windowId(10)    //  m_windowId is important else doesn't connect with KCookieServer (yeah!)
    , m_kcookiejar(new QDBusInterface("org.kde.kded", "/modules/kcookiejar", "org.kde.KCookieServer"))
{
}


CookieJar::~CookieJar()
{
}


QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl & url) const
{
    QList<QNetworkCookie> cookieList;
    QDBusReply<QString> reply = m_kcookiejar->call("findCookies", url.toString() , m_windowId);

    if (reply.isValid())
    {
        cookieList << reply.value().toUtf8();
        //kDebug() << reply.value();
    }
    else
    {
        kWarning() << "Unable to communicate with the cookiejar!";
    }

    return cookieList;
}


bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> & cookieList, const QUrl & url)
{
    QByteArray cookieHeader;
    foreach(const QNetworkCookie& cookie, cookieList)
    {
        cookieHeader = "Set-Cookie: ";
        cookieHeader += cookie.toRawForm();
        m_kcookiejar->call("addCookies", url.toString(), cookieHeader, m_windowId);
        //kDebug() << "url: " << url.host() << ", cookie: " << cookieHeader;
    }

    return !m_kcookiejar->lastError().isValid();
}


void CookieJar::setWindowId(qlonglong id)
{
    m_windowId = id;
}


void CookieJar::clear()
{
    QDBusReply<void> reply = m_kcookiejar->call( "deleteAllCookies" );
    if (!reply.isValid())
    {
        kWarning() << "Unable to delete all the cookies as requested.";
        return;
    }
}
