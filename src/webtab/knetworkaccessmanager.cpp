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

// KDE Includes
#include <KSharedConfig>
#include <KConfigGroup>

#include <KProtocolManager>

// Qt Includes
#include <QNetworkProxy>


KNetworkAccessManager::KNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
    const QString &proxyUrl = KProtocolManager::proxyFor( QL1S("http:") );
    if (proxyUrl.isEmpty() || proxyUrl == QL1S("DIRECT"))
         return;

    const QUrl url(proxyUrl);
    QNetworkProxy::ProxyType proxyType;
    if (url.scheme() == QL1S("http"))
    {
        proxyType = QNetworkProxy::HttpProxy;
    }
    else 
    {
        if (url.scheme() == QL1S("socks"))
        {
            proxyType = QNetworkProxy::Socks5Proxy;
        }
        else
        {
            return;
        }
    }
    
    setProxy(QNetworkProxy(proxyType, url.host(), url.port(), url.userName(), url.password()));
}
