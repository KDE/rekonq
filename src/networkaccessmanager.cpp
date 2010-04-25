/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "networkaccessmanager.h"
#include "networkaccessmanager.moc"

// Local Includes
#include "application.h"
#include "adblockmanager.h"
#include "webpage.h"

// KDE Includes
#include <KDebug>
#include <KLocale>
#include <KProtocolManager>

// Defines
#define QL1S(x)  QLatin1String(x)


NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : AccessManager(parent)
{
    QString c = KGlobal::locale()->country();
    if(c == QL1S("C"))
        c = QL1S("en_US");
    if(c != QL1S("en_US"))
        c.append( QL1S(", en_US") );
    
    _acceptLanguage = c.toLatin1();
}


QNetworkReply *NetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    WebPage *parentPage = qobject_cast<WebPage *>( parent() );
    
    QNetworkReply *reply = 0;
    
    QNetworkRequest req = request;
    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    req.setRawHeader("Accept-Language", _acceptLanguage);

    KIO::CacheControl cc = KProtocolManager::cacheControl();
    switch(cc)
    {
    case KIO::CC_CacheOnly:      // Fail request if not in cache.
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
        break;
        
    case KIO::CC_Refresh:        // Always validate cached entry with remote site.
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
        break;
        
    case KIO::CC_Reload:         // Always fetch from remote site
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        break;

    case KIO::CC_Cache:          // Use cached entry if available.
    case KIO::CC_Verify:         // Validate cached entry with remote site if expired.
    default:
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        break;
    }
    

    if( op == QNetworkAccessManager::GetOperation )
    {
        reply = Application::adblockManager()->block(req, parentPage);
        if (reply)
            return reply;
    }

    return AccessManager::createRequest(op,req,outgoingData);
}
