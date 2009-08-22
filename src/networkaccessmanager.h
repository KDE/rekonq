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


#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H


// KDE Includes
#include <kio/accessmanager.h>

// Forward Declarations
class QNetworkDiskCache;


using namespace KIO;


class NetworkAccessManager : public AccessManager
{
    Q_OBJECT

public:
    NetworkAccessManager(QObject *parent = 0);
    KIO::MetaData& metaData();    
    void resetDiskCache();
    
public slots:
    void loadSettings();

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0);

private slots:
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);

#ifndef QT_NO_OPENSSL
    void slotSSLErrors(QNetworkReply *reply, const QList<QSslError> &error);
#endif

private:
    QNetworkDiskCache *m_diskCache;
    KIO::MetaData m_metaData;
};

#endif // NETWORKACCESSMANAGER_H
