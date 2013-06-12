/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (c) 2001 by Dawit Alemayehu <adawit@kde.org>
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "useragentinfo.h"

// Standard Includes
#include <time.h>
#include <sys/utsname.h>

// KDE Includes
#include <KService>
#include <KServiceTypeTrader>

#include <KConfig>
#include <KConfigGroup>

#include <KProtocolManager>

// Qt includes
#include <QStringBuilder>


UserAgentInfo::UserAgentInfo()
{
    m_providers = KServiceTypeTrader::self()->query("UserAgentStrings");
}


KService::List UserAgentInfo::availableProviders() const
{
    return m_providers;
}


QString UserAgentInfo::userAgentString(int i)
{
    if (i < 0 || !providerExists(i))
    {
        kDebug() << "oh oh... wrong index on the user agent choice! INDEX = " << i;
        return QL1S("Default");
    }

    QString tmp = m_providers.at(i)->property("X-KDE-UA-FULL").toString();

    struct utsname utsn;
    uname(&utsn);

    tmp.replace(QL1S("appSysName"), QString(utsn.sysname));
    tmp.replace(QL1S("appSysRelease"), QString(utsn.release));
    tmp.replace(QL1S("appMachineType"), QString(utsn.machine));

    QStringList languageList = KGlobal::locale()->languageList();
    if (languageList.count())
    {
        int ind = languageList.indexOf(QL1S("C"));
        if (ind >= 0)
        {
            if (languageList.contains(QL1S("en")))
                languageList.removeAt(ind);
            else
                languageList.value(ind) = QL1S("en");
        }
    }

    tmp.replace(QL1S("appLanguage"), QString("%1").arg(languageList.join(", ")));
    tmp.replace(QL1S("appPlatform"), QL1S("X11"));

    return tmp;
}


QString UserAgentInfo::userAgentName(int i)
{
    if (i < 0 || !providerExists(i))
    {
        kDebug() << "oh oh... wrong index on the user agent choice! INDEX = " << i;
        return QL1S("Default");
    }

    return m_providers.at(i)->property("X-KDE-UA-NAME").toString();
}


QString UserAgentInfo::userAgentVersion(int i)
{
    if (i < 0 || !providerExists(i))
    {
        kDebug() << "oh oh... wrong index on the user agent choice! INDEX = " << i;
        return QL1S("Default");
    }

    return m_providers.at(i)->property("X-KDE-UA-VERSION").toString();
}


QString UserAgentInfo::userAgentDescription(int i)
{
    if (i < 0 || !providerExists(i))
    {
        kDebug() << "oh oh... wrong index on the user agent choice! INDEX = " << i;
        return QL1S("Default");
    }

    QString systemName = m_providers.at(i)->property("X-KDE-UA-SYSNAME").toString();
    QString systemRelease = m_providers.at(i)->property("X-KDE-UA-SYSRELEASE").toString();
    QString systemSummary;

    if (!systemName.isEmpty() && !systemRelease.isEmpty())
    {
        systemSummary = i18nc("describe UA platform, eg: firefox 3.1 \"on Windows XP\"", " on %1 %2", systemName, systemRelease);
    }

    return userAgentName(i) % QL1C(' ') % userAgentVersion(i) % systemSummary;
}


QStringList UserAgentInfo::availableUserAgents()
{
    QStringList UAs;
    int n = m_providers.count();
    for (int i = 0; i < n; ++i)
    {
        UAs << userAgentDescription(i);
    }
    return UAs;
}


bool UserAgentInfo::setUserAgentForHost(int uaIndex, const QString &host)
{
    KConfig config("kio_httprc", KConfig::NoGlobals);

    QStringList modifiedHosts = config.groupList();
    KConfigGroup hostGroup(&config, host);

    if (uaIndex == -1)
    {
        if (!hostGroup.exists())
        {
            kDebug() << "Host does NOT exists!";
            return false;
        }
        hostGroup.deleteGroup();
        KProtocolManager::reparseConfiguration();
        return true;
    }

    hostGroup.writeEntry(QL1S("UserAgent"), userAgentString(uaIndex));

    KProtocolManager::reparseConfiguration();
    return true;
}


int UserAgentInfo::uaIndexForHost(const QString &host)
{
    QString KDEUserAgent = KProtocolManager::userAgentForHost(host);

    int n = m_providers.count();
    for (int i = 0; i < n; ++i)
    {
        QString rekonqUserAgent = userAgentString(i);
        if (KDEUserAgent == rekonqUserAgent)
            return i;
    }
    return -1;
}


bool UserAgentInfo::providerExists(int i)
{
    KService::Ptr s = m_providers.at(i);
    if (s.isNull())
    {
        return false;
    }
    return true;
}
