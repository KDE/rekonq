/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Pierre Rossi <pierre dot rossi at gmail dot com>
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


#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "downloaditem.h"

// Qt Includes
#include <QObject>



// ---------------------------------------------------------------------------------


class REKONQ_TESTS_EXPORT DownloadManager : public QObject
{
    Q_OBJECT

public:
    DownloadManager(QObject *parent = 0);

    DownloadList downloads() const { return m_downloadList; }
    DownloadItem* addDownload(const QString &srcUrl, const QString &destUrl);
    bool clearDownloadsHistory();

Q_SIGNALS:
    void newDownloadAdded(QObject *item);

private:
    DownloadList m_downloadList;

    void init();
};

#endif // DOWNLOADMANAGER_H
