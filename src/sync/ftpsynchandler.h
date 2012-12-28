/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011 by Andrea Diamantini <adjam7 at gmail dot com>
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


#ifndef FTP_SYNC_HANDLER_H
#define FTP_SYNC_HANDLER_H


// Local Includes
#include "synchandler.h"

// KDE Includes
#include <KUrl>

// Forward Declarations
class KJob;


class FTPSyncHandler : public SyncHandler
{
    Q_OBJECT

public:
    explicit FTPSyncHandler(QObject *parent = 0);

    void syncHistory();
    void syncBookmarks();
    void syncPasswords();

    void initialLoadAndCheck();

private Q_SLOTS:
    void onBookmarksSyncFinished(KJob *);
    void onBookmarksStatFinished(KJob *);

    void onHistorySyncFinished(KJob *);
    void onHistoryStatFinished(KJob *);

    void onPasswordsSyncFinished(KJob *);
    void onPasswordsStatFinished(KJob *);

Q_SIGNALS:
    void syncBookmarksFinished(bool);
    void syncHistoryFinished(bool);
    void syncPasswordsFinished(bool);

private:
    bool syncRelativeEnabled(bool);

    QUrl _remoteBookmarksUrl;
    KUrl _localBookmarksUrl;

    QUrl _remoteHistoryUrl;
    KUrl _localHistoryUrl;

    QUrl _remotePasswordsUrl;
    KUrl _localPasswordsUrl;
};

#endif // FTP_SYNC_HANDLER_H
