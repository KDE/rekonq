/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "knetworkaccessmanager.h"
#include "knetworkaccessmanager.moc"

// KDE Includes
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QNetworkProxy>


KNetworkAccessManager::KNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
    // Proxy
    QNetworkProxy proxy;
    
    KConfig config("kioslaverc", KConfig::NoGlobals);
    
    KConfigGroup proxyGroup(&config, QL1S("Proxy Settings"));
    
    int proxyType = proxyGroup.readEntry( QL1S("ProxyType"), 0);
    kDebug() << "PROXY TYPE: " << proxyType;

    if (proxyType == 0)
        return;

    QStringList httpProxy = proxyGroup.readEntry( QL1S("httpProxy"), QString("") ).split(QL1C(' '));
    QStringList socksProxy = proxyGroup.readEntry( QL1S("socksProxy"), QString("") ).split(QL1C(' '));
    
    QStringList proxyInfoList;

    if (!httpProxy.isEmpty()) 
    {
        proxy.setType(QNetworkProxy::HttpProxy);
        proxyInfoList = httpProxy;
    } 
    else 
    {
        if (!socksProxy.isEmpty()) 
        {
            proxy.setType(QNetworkProxy::Socks5Proxy);
            proxyInfoList = socksProxy;
        }
    }
    
    if (proxyInfoList.isEmpty())
        return;
    
    // else
    proxyInfoList.first().remove("http://");

    // set host
    QString proxyHost = proxyInfoList.at(0);
    kDebug() << "PROXY HOST: " << proxyHost;
    proxy.setHostName(proxyHost);
    
    // set port
    if (proxyInfoList.count() == 2)
    {
        int proxyPort = proxyInfoList.at(1).toInt();
        kDebug() << "PROXY PORT: " << proxyPort;
        proxy.setPort(proxyPort);
    }
//     proxy.setUser("username");
//     proxy.setPassword("password");
//     
    setProxy(proxy);
}
