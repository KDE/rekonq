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


NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : AccessManager(parent)
    , _parentPage( qobject_cast<WebPage *>(parent) )
{
}


QNetworkReply *NetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QNetworkReply *reply = 0;
    
    QNetworkRequest req = request;
    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
//     if (!m_acceptLanguage.isEmpty())
//         req.setRawHeader("Accept-Language", m_acceptLanguage);
    
    switch(op)
    {
    case QNetworkAccessManager::HeadOperation:
        kDebug() << "HEAD OPERATION";
        break;
    
    case QNetworkAccessManager::GetOperation:
        kDebug() << "GET OPERATION";
        reply = Application::adblockManager()->block(req, _parentPage);
        if (reply)
            return reply;
        break;
    
    case QNetworkAccessManager::PutOperation:
        kDebug() << "PUT OPERATION";
        break;
    
    case QNetworkAccessManager::PostOperation:
        kDebug() << "POST OPERATION";
        break;
    
    case QNetworkAccessManager::DeleteOperation:
        kDebug() << "DELETE OPERATION";
        break;
        
    default:
        kDebug() << "UNKNOWN OPERATION";
        break;
    }

    return AccessManager::createRequest(op,req,outgoingData);
}
