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


QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
    stream << JAR_VERSION;
    stream << quint32(list.size());
    for (int i = 0; i < list.size(); ++i)
        stream << list.at(i).toRawForm();
    return stream;
}


QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
    list.clear();

    quint32 version;
    stream >> version;

    if (version != JAR_VERSION)
        return stream;

    quint32 count;
    stream >> count;
    for (quint32 i = 0; i < count; ++i)
    {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
        if (newCookies.count() == 0 && value.length() != 0)
        {
            kDebug() << "CookieJar: Unable to parse saved cookie:" << value;
        }
        for (int j = 0; j < newCookies.count(); ++j)
            list.append(newCookies.at(j));
        if (stream.atEnd())
            break;
    }
    return stream;
}


CookieJar::CookieJar(QObject *parent)
        : QNetworkCookieJar(parent)
        , m_loaded(false)
        , m_saveTimer(new AutoSaver(this))
        , m_acceptCookies(AcceptOnlyFromSitesNavigatedTo)
{
    load();
}


CookieJar::~CookieJar()
{
    if (m_keepCookies == KeepUntilExit)
        clear();
    m_saveTimer->saveIfNeccessary();
}


void CookieJar::clear()
{
    setAllCookies(QList<QNetworkCookie>());
    m_saveTimer->changeOccurred();
    emit cookiesChanged();
}


void CookieJar::load()
{
    if (m_loaded)
        return;

    // load cookies and exceptions
    QString filepath = KStandardDirs::locateLocal("appdata", "cookies.ini");
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
    QSettings cookieSettings(filepath, QSettings::IniFormat);
    setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieSettings.value(QLatin1String("cookies"))));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    m_exceptions_block = cookieSettings.value(QLatin1String("block")).toStringList();
    m_exceptions_allow = cookieSettings.value(QLatin1String("allow")).toStringList();
    m_exceptions_allowForSession = cookieSettings.value(QLatin1String("allowForSession")).toStringList();
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());

    loadSettings();

    save();
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

    m_loaded = true;
    emit cookiesChanged();
}


void CookieJar::save()
{
    if (!m_loaded)
        return;
    purgeOldCookies();

    QString filepath = KStandardDirs::locateLocal("appdata", "cookies.ini");
    QSettings cookieSettings(filepath, QSettings::IniFormat);
    QList<QNetworkCookie> cookies = allCookies();
    for (int i = cookies.count() - 1; i >= 0; --i)
    {
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    }

    cookieSettings.setValue(QLatin1String("cookies"), qVariantFromValue<QList<QNetworkCookie> >(cookies));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    cookieSettings.setValue(QLatin1String("block"), m_exceptions_block);
    cookieSettings.setValue(QLatin1String("allow"), m_exceptions_allow);
    cookieSettings.setValue(QLatin1String("allowForSession"), m_exceptions_allowForSession);

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
    CookieJar *that = const_cast<CookieJar*>(this);
    if (!m_loaded)
        that->load();

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
    if (!m_loaded)
        load();

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
#if 0
                else
                    kWarning() << "setCookiesFromUrl failed" << url << cookieList.value(0).toRawForm();
#endif
            }
        }
    }

    if (addedCookies)
    {
        m_saveTimer->changeOccurred();
        emit cookiesChanged();
    }
    return addedCookies;
}


CookieJar::AcceptPolicy CookieJar::acceptPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_acceptCookies;
}


void CookieJar::setAcceptPolicy(AcceptPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_acceptCookies)
        return;
    m_acceptCookies = policy;
    m_saveTimer->changeOccurred();
}


CookieJar::KeepPolicy CookieJar::keepPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_keepCookies;
}


void CookieJar::setKeepPolicy(KeepPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_keepCookies)
        return;
    m_keepCookies = policy;
    m_saveTimer->changeOccurred();
}


QStringList CookieJar::blockedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_block;
}


QStringList CookieJar::allowedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allow;
}


QStringList CookieJar::allowForSessionCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allowForSession;
}


void CookieJar::setBlockedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_block = list;
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
    m_saveTimer->changeOccurred();
}


void CookieJar::setAllowedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allow = list;
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
    m_saveTimer->changeOccurred();
}


void CookieJar::setAllowForSessionCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allowForSession = list;
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());
    m_saveTimer->changeOccurred();
}
