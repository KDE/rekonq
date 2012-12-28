/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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



#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KUrl>

// Qt Includes
#include <QObject>

// Forward Declarations
class KDirLister;
class KFileItemList;
class KJob;

class QNetworkRequest;
class QWebFrame;


class REKONQ_TESTS_EXPORT ProtocolHandler : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolHandler(QObject *parent = 0);

    /**
     * This function handles all the protocols that have to be handled before
     * WebKit does
     */
    bool preHandling(const QNetworkRequest &request, QWebFrame *frame);

    /**
     * This function handles all the protocols that have to be handled after
     * WebKit tried to
     */
    bool postHandling(const QNetworkRequest &request, QWebFrame *frame);

    void setWindow(QWidget *);

Q_SIGNALS:
    void downloadUrl(const KUrl &);

private Q_SLOTS:
    void showResults(const KFileItemList &);
    void slotMostLocalUrlResult(KJob *);

private:
    QString dirHandling(const KFileItemList &list);

    KDirLister *_lister;
    QWebFrame *_frame;
    KUrl _url;

    QWidget *_webwin;
};

#endif  // PROTOCOL_HANDLER_H
