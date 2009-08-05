/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
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


#ifndef COOKIEJAR_H
#define COOKIEJAR_H


// Qt Includes
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkCookieJar>

// Forward Declarations
class QUrl;


class CookieJar : public QNetworkCookieJar
{
    friend class CookieModel;
    Q_OBJECT

    Q_PROPERTY(AcceptPolicy acceptPolicy READ acceptPolicy WRITE setAcceptPolicy)
    Q_PROPERTY(KeepPolicy keepPolicy READ keepPolicy WRITE setKeepPolicy)
    Q_PROPERTY(QStringList blockedCookies READ blockedCookies WRITE setBlockedCookies)
    Q_PROPERTY(QStringList allowedCookies READ allowedCookies WRITE setAllowedCookies)
    Q_PROPERTY(QStringList allowForSessionCookies READ allowForSessionCookies WRITE setAllowForSessionCookies)

    Q_ENUMS(KeepPolicy)
    Q_ENUMS(AcceptPolicy)

signals:
    void cookiesChanged();

public:
    enum AcceptPolicy
    {
        AcceptAlways,
        AcceptNever,
        AcceptOnlyFromSitesNavigatedTo
    };

    enum KeepPolicy
    {
        KeepUntilExpire,
        KeepUntilExit,
        KeepUntilTimeLimit
    };

    CookieJar(QObject *parent = 0);
    ~CookieJar();

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

    AcceptPolicy acceptPolicy() const;
    void setAcceptPolicy(AcceptPolicy policy);

    KeepPolicy keepPolicy() const;
    void setKeepPolicy(KeepPolicy policy);

    QStringList blockedCookies() const;
    QStringList allowedCookies() const;
    QStringList allowForSessionCookies() const;

    void setBlockedCookies(const QStringList &list);
    void setAllowedCookies(const QStringList &list);
    void setAllowForSessionCookies(const QStringList &list);

public slots:
    void clear();
    void loadSettings();

private slots:
    void save();

private:
    void purgeOldCookies();

    AcceptPolicy m_acceptCookies;
    KeepPolicy m_keepCookies;

    QStringList m_exceptions_block;
    QStringList m_exceptions_allow;
    QStringList m_exceptions_allowForSession;
};

#endif // COOKIEJAR_H
