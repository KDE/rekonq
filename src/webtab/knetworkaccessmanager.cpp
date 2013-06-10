/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
        proxy.setType(QNetworkProxy::NoProxy);
    else
        proxy.setType(QNetworkProxy::Socks5Proxy);

    QString proxyHost = proxyGroup.readEntry( QL1S("socksProxy"), QString("") );
    QStringList proxyInfoList = proxyHost.split(QL1C(' '));
    kDebug() << proxyInfoList;
    if (proxyInfoList.isEmpty())
        return;
    
    proxy.setHostName(proxyInfoList.at(0));
    
    if (proxyInfoList.count() == 2)
        proxy.setPort(proxyInfoList.at(1).toInt());
    
//     proxy.setUser("username");
//     proxy.setPassword("password");
//     
    setProxy(proxy);
}
