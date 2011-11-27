/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009-2010 by Richard J. Moore <rich@kde.org>
* Copyright (C) 2010-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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



#ifndef NETWORK_ANALYZER_H
#define NETWORK_ANALYZER_H


// Rekonq Includes
#include "rekonq_defines.h"

// Qt Includes
#include <QtCore/QMap>
#include <QtCore/QList>

#include <QtGui/QWidget>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QPoint>

// Forward Declarations
class QTreeWidgetItem;
class QSignalMapper;
class QTreeWidget;


class NetworkAnalyzer : public QWidget
{
    Q_OBJECT

public:
    NetworkAnalyzer(QWidget *parent = 0);

private Q_SLOTS:
    void addRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QNetworkReply *reply);

    void clear();
    void requestFinished(QObject *replyObject);
    void showItemDetails(QTreeWidgetItem *item);
    void copyURL();
    void popupContextMenu(const QPoint &pos);

private:
    QMap<QNetworkReply *, QNetworkRequest> _requestMap;
    QMap<QTreeWidgetItem *, QNetworkRequest> _itemRequestMap;
    QMap<QNetworkReply *, QTreeWidgetItem *> _itemMap;
    QMap<QTreeWidgetItem *, QPair< QList<QByteArray>, QList<QByteArray> > > _itemReplyMap;

    QSignalMapper *_mapper;
    QTreeWidget *_requestList;
};

#endif // NETWORK_ANALYZER_H
