/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>*
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
#include <KDebug>

NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : AccessManager(parent)
{
}


QNetworkReply *NetworkAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    if (op == PostOperation && outgoingData)
    {
        QByteArray outgoingDataByteArray = outgoingData->peek(1024 * 1024);
        kDebug() << "*************************************************************************";
        kDebug() << outgoingDataByteArray;
        kDebug() << "*************************************************************************";
    }
        
    QNetworkRequest request(req);
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

    // Adblock
    if (op == QNetworkAccessManager::GetOperation)
    {
        QNetworkReply *reply = Application::adblockManager()->block(request);
        if (reply)
            return reply;
    }

    return AccessManager::createRequest(op,request,outgoingData);
}
