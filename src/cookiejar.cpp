/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Domrachev Alexandr <alexandr.domrachev@gmail.com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
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

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QCompleter>

#include <QtWebKit/QWebSettings>


static const unsigned int JAR_VERSION = 23;

static const char cookieFileName[] = "cookies";


CookieJar::CookieJar(QObject *parent)
        : QNetworkCookieJar(parent)
        , m_acceptCookies(AcceptOnlyFromSitesNavigatedTo)
{
    // load cookies and exceptions
    QString filepath = KStandardDirs::locateLocal("appdata", cookieFileName);
    KConfig iniconfig(filepath);

    KConfigGroup inigroup1 = iniconfig.group("cookielist");

    QStringList cookieStringList = inigroup1.readEntry( QString("cookies"), QStringList() );
    QList<QNetworkCookie> cookieNetworkList;
    foreach( QString str, cookieStringList )
    {
        cookieNetworkList << QNetworkCookie( str.toLocal8Bit() );
    }
    setAllCookies( cookieNetworkList );

    KConfigGroup inigroup2 = iniconfig.group("exceptions");
    m_exceptions_block = inigroup2.readEntry( QString("block") , QStringList() );
    m_exceptions_allow = inigroup2.readEntry( QString("allow"), QStringList() );
    m_exceptions_allowForSession = inigroup2.readEntry( QString("allowForSession"), QStringList() );

    qSort( m_exceptions_block.begin(), m_exceptions_block.end() );
    qSort( m_exceptions_allow.begin(), m_exceptions_allow.end() );
    qSort( m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end() );

    loadSettings();
}


CookieJar::~CookieJar()
{
    if (m_keepCookies == KeepUntilExit)
        clear();

    save();
}


void CookieJar::clear()
{
    setAllCookies(QList<QNetworkCookie>());

    save();

    emit cookiesChanged();
}


void CookieJar::loadSettings()
{
    int canAcceptCookies = ReKonfig::acceptCookies();

    switch (canAcceptCookies)
    {
    case 0:
        m_acceptCookies = AcceptAlways;
        break;
    case 1:
        m_acceptCookies = AcceptNever;
        break;
    case 2:
    default:
        m_acceptCookies = AcceptOnlyFromSitesNavigatedTo;
        break;
    }

    int canKeepCookiesUntil = ReKonfig::keepCookiesUntil();

    switch (canKeepCookiesUntil)
    {
    default:
    case 0:
        m_keepCookies = KeepUntilExpire;
        break;
    case 1:
        m_keepCookies = KeepUntilExit;
        setAllCookies(QList<QNetworkCookie>());
        break;
    case 2:
        m_keepCookies = KeepUntilTimeLimit;
        break;
    }

    emit cookiesChanged();
}


void CookieJar::save()
{
    purgeOldCookies();

    QString filepath = KStandardDirs::locateLocal("appdata", cookieFileName);
    KConfig iniconfig( filepath );

    KConfigGroup inigroup1 = iniconfig.group("cookielist");
    QList<QNetworkCookie> cookies = allCookies();
    for (int i = cookies.count() - 1; i >= 0; --i)
    {
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    }

    QStringList cookieStringList;
    foreach( QNetworkCookie cookie, cookies )
    {
        cookieStringList << QString( cookie.toRawForm() );
    }
    inigroup1.writeEntry( QString("cookies"), cookieStringList );

    KConfigGroup inigroup2 = iniconfig.group("exceptions");
    inigroup2.writeEntry( QString("block"), m_exceptions_block );
    inigroup2.writeEntry( QString("allow"), m_exceptions_allow );
    inigroup2.writeEntry( QString("allowForSession"), m_exceptions_allowForSession );

    // save cookie settings
    int n;
    switch (m_acceptCookies)
    {
    case AcceptAlways:
        n = 0;
        break;
    case AcceptNever:
        n = 1;
        break;
    case AcceptOnlyFromSitesNavigatedTo:
    default:
        n = 2;
        break;
    }
    ReKonfig::setAcceptCookies(n);


    switch (m_keepCookies)
    {
    default:
    case KeepUntilExpire:
        n = 0;
        break;
    case KeepUntilExit:
        n = 1;
        break;
    case KeepUntilTimeLimit:
        n = 2;
        break;
    }
    ReKonfig::setKeepCookiesUntil(n);
}


void CookieJar::purgeOldCookies()
{
    QList<QNetworkCookie> cookies = allCookies();
    if (cookies.isEmpty())
        return;
    int oldCount = cookies.count();
    QDateTime now = QDateTime::currentDateTime();
    for (int i = cookies.count() - 1; i >= 0; --i)
    {
        if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
            cookies.removeAt(i);
    }
    if (oldCount == cookies.count())
        return;
    setAllCookies(cookies);

    emit cookiesChanged();
}


QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        QList<QNetworkCookie> noCookies;
        return noCookies;
    }

    return QNetworkCookieJar::cookiesForUrl(url);
}


bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return false;

    QString host = url.host();
    bool eBlock = qBinaryFind(m_exceptions_block.begin(), m_exceptions_block.end(), host) != m_exceptions_block.end();
    bool eAllow = qBinaryFind(m_exceptions_allow.begin(), m_exceptions_allow.end(), host) != m_exceptions_allow.end();
    bool eAllowSession = qBinaryFind(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end(), host) != m_exceptions_allowForSession.end();

    bool addedCookies = false;
    // pass exceptions
    bool acceptInitially = (m_acceptCookies != AcceptNever);
    if ((acceptInitially && !eBlock) || (!acceptInitially && (eAllow || eAllowSession)))
    {
        // pass url domain == cookie domain
        QDateTime soon = QDateTime::currentDateTime();
        soon = soon.addDays(90);
        foreach(QNetworkCookie cookie, cookieList)
        {
            QList<QNetworkCookie> lst;
            if (m_keepCookies == KeepUntilTimeLimit
                    && !cookie.isSessionCookie()
                    && cookie.expirationDate() > soon)
            {
                cookie.setExpirationDate(soon);
            }
            lst += cookie;
            if (QNetworkCookieJar::setCookiesFromUrl(lst, url))
            {
                addedCookies = true;
            }
            else
            {
                // finally force it in if wanted
                if (m_acceptCookies == AcceptAlways)
                {
                    QList<QNetworkCookie> cookies = allCookies();
                    cookies += cookie;
                    setAllCookies(cookies);
                    addedCookies = true;
                }
            }
        }
    }

    if (addedCookies)
    {
        save();
        emit cookiesChanged();
    }
    return addedCookies;
}


CookieJar::AcceptPolicy CookieJar::acceptPolicy() const
{
    return m_acceptCookies;
}


void CookieJar::setAcceptPolicy(AcceptPolicy policy)
{
    if (policy == m_acceptCookies)
        return;
    m_acceptCookies = policy;

    save();
}


CookieJar::KeepPolicy CookieJar::keepPolicy() const
{
    return m_keepCookies;
}


void CookieJar::setKeepPolicy(KeepPolicy policy)
{
    if (policy == m_keepCookies)
        return;
    m_keepCookies = policy;

    save();
}


QStringList CookieJar::blockedCookies() const
{
    return m_exceptions_block;
}


QStringList CookieJar::allowedCookies() const
{
    return m_exceptions_allow;
}


QStringList CookieJar::allowForSessionCookies() const
{
    return m_exceptions_allowForSession;
}


void CookieJar::setBlockedCookies(const QStringList &list)
{
    m_exceptions_block = list;
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());

    save();
}


void CookieJar::setAllowedCookies(const QStringList &list)
{
    m_exceptions_allow = list;
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());

    save();
}


void CookieJar::setAllowForSessionCookies(const QStringList &list)
{
    m_exceptions_allowForSession = list;
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());

    save();
}
