/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Pierre Rossi <pierre dot rossi at gmail dot com>
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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

// KDE Includes
#include <kio/accessmanager.h>
#include <KIO/CopyJob>

// Qt Includes
#include <QObject>
#include <QWidget>

// Forward Includes
class KUrl;


class REKONQ_TESTS_EXPORT DownloadManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Entry point.
     * Access to DownloadManager class by using
     * DownloadManager::self()->thePublicMethodYouNeed()
     */
    static DownloadManager *self();

    ~DownloadManager();

    DownloadList downloads() const
    {
        return m_downloadList;
    }

    bool clearDownloadsHistory();

    bool downloadResource(const KUrl &url,
                          const KIO::MetaData &metaData = KIO::MetaData(),
                          QWidget *parent = 0,
                          bool forceDirRequest = false,
                          const QString &suggestedName = QString(),
                          bool registerDownload = true);

    void downloadLinksWithKGet(const QVariant &contentList);

    void removeDownloadItem(int index);

private:
    explicit DownloadManager(QObject *parent = 0);
    
    void init();

    DownloadItem* addDownload(KIO::CopyJob *job);

Q_SIGNALS:
    void newDownloadAdded(QObject *item);

private:
    DownloadList m_downloadList;

    bool m_needToSave;

    static QWeakPointer<DownloadManager> s_downloadManager;
};

#endif // DOWNLOADMANAGER_H
