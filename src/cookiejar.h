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


#ifndef RECOOKIEJAR_H
#define RECOOKIEJAR_H

// Qt Includes
#include <QtNetwork/QNetworkCookieJar>

// Forward Declarations
class QUrl;

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT

public:
    CookieJar(QObject* parent = 0);
    virtual ~CookieJar();
    void setWindowId(qlonglong id);

    virtual QList<QNetworkCookie> cookiesForUrl(const QUrl & url) const;
    virtual bool setCookiesFromUrl(const QList<QNetworkCookie> & cookieList, const QUrl & url);

    void clear();

private:
    qlonglong m_windowId;
};

#endif // RECOOKIEJAR_H
