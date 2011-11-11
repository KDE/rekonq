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


// Self Includes
#include "syncmanager.h"
#include "syncmanager.moc"

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KStandardDirs>
#include <KMessageBox>
#include <klocalizedstring.h>

#include <KIO/Job>
#include <KIO/JobUiDelegate>


SyncManager::SyncManager(QObject *parent)
    : QObject(parent)
    , _firstTimeSynced(false)
{
    if (ReKonfig::syncEnabled())
    {
        // sync every minute
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(sync()));
        timer->start(60 * 1000);
    }
}


SyncManager::~SyncManager()
{
    sync();
}


void SyncManager::firstTimeSync()
{
    // Bookmarks
    if (ReKonfig::syncBookmarks())
    {
        _remoteBookmarksUrl = QUrl(ReKonfig::syncHost());
        _remoteBookmarksUrl.setUserName(ReKonfig::syncUser());
        _remoteBookmarksUrl.setPassword(ReKonfig::syncPass());
        _remoteBookmarksUrl.setPath(QL1S("/data/") + ReKonfig::syncUser() + QL1S("/files/bookmarks.xml"));
        kDebug() << "REMOTE BK URL: " << _remoteBookmarksUrl;

        const QString bookmarksFilePath = KStandardDirs::locateLocal("data", QL1S("konqueror/bookmarks.xml"));
        _localBookmarksUrl = KUrl(bookmarksFilePath);
        kDebug() << "LOCAL BK URL: " << _localBookmarksUrl;

        KIO::StatJob *job = KIO::stat(_remoteBookmarksUrl, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onBookmarksStatFinished(KJob *)));

        _firstTimeSynced = true;
    }

    // History
    if (ReKonfig::syncHistory())
    {
        _remoteHistoryUrl = QUrl(ReKonfig::syncHost());
        _remoteHistoryUrl.setUserName(ReKonfig::syncUser());
        _remoteHistoryUrl.setPassword(ReKonfig::syncPass());
        _remoteHistoryUrl.setPath(QL1S("/data/") + ReKonfig::syncUser() + QL1S("/files/history"));
        kDebug() << "REMOTE HISTORY URL: " << _remoteHistoryUrl;

        const QString historyFilePath = KStandardDirs::locateLocal("appdata", "history");
        _localHistoryUrl = KUrl(historyFilePath);
        kDebug() << "LOCAL HISTORY URL: " << _localHistoryUrl;

        KIO::StatJob *job = KIO::stat(_remoteHistoryUrl, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onHistoryStatFinished(KJob *)));

        _firstTimeSynced = true;
    }

    // Passwords
    if (ReKonfig::syncPasswords())
    {
        _remotePasswordsUrl = QUrl(ReKonfig::syncHost());
        _remotePasswordsUrl.setUserName(ReKonfig::syncUser());
        _remotePasswordsUrl.setPassword(ReKonfig::syncPass());
        _remotePasswordsUrl.setPath(QL1S("/data/") + ReKonfig::syncUser() + QL1S("/files/kdewallet.kwl"));
        kDebug() << "REMOTE PSWD URL: " << _remotePasswordsUrl;

        const QString passwordsFilePath = KStandardDirs::locateLocal("data", QL1S("kwallet/kdewallet.kwl"));
        _localPasswordsUrl = KUrl(passwordsFilePath);
        kDebug() << "LOCAL PSWD URL: " << _localPasswordsUrl;

        KIO::StatJob *job = KIO::stat(_remotePasswordsUrl, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onPasswordsStatFinished(KJob *)));

        _firstTimeSynced = true;
    }
}


void SyncManager::sync()
{
    kDebug() << "syncing now...";

    if (!ReKonfig::syncEnabled())
        return;

    if (!_firstTimeSynced)
    {
        kDebug() << "need to sync for the first time...";
        firstTimeSync();
        return;
    }

    // Bookmarks
    if (ReKonfig::syncBookmarks())
    {
        KIO::Job *job = KIO::file_copy(_localBookmarksUrl, _remoteBookmarksUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onBookmarksSyncFinished(KJob *)));
    }

    // History
    if (ReKonfig::syncHistory())
    {
        KIO::Job *job = KIO::file_copy(_localHistoryUrl, _remoteHistoryUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onHistorySyncFinished(KJob *)));
    }

    // Passwords
    if (ReKonfig::syncPasswords())
    {
        KIO::Job *job = KIO::file_copy(_localPasswordsUrl, _remotePasswordsUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onPasswordsSyncFinished(KJob *)));
    }
}


// ---------------------------------------------------------------------------------------


void SyncManager::onBookmarksStatFinished(KJob *job)
{
    if (job->error() ||
            KMessageBox::questionYesNo(0,
                                       i18n("A remote bookmarks file has just present in your remote server."),
                                       i18n("Server notification"),
                                       KGuiItem(i18n("Overwrite it")),
                                       KGuiItem(i18n("Copy it locally"))
                                      )
       )
    {
        KIO::Job *job = KIO::file_copy(_localBookmarksUrl, _remoteBookmarksUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onBookmarksSyncFinished(KJob *)));
    }
    else
    {
        KIO::Job *job = KIO::file_copy(_remoteBookmarksUrl, _localBookmarksUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onBookmarksSyncFinished(KJob *)));
    }
}


void SyncManager::onBookmarksSyncFinished(KJob *job)
{
    if (job->error())
    {
        job->uiDelegate()->showErrorMessage();
        emit syncBookmarksFinished(false);
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    ReKonfig::setLastSyncDateTime(now);
    emit syncBookmarksFinished(true);
}


// ---------------------------------------------------------------------------------------


void SyncManager::onHistoryStatFinished(KJob *job)
{
    if (job->error() ||
            KMessageBox::questionYesNo(0,
                                       i18n("A remote history file has just present in your remote server."),
                                       i18n("Server notification"),
                                       KGuiItem(i18n("Overwrite it")),
                                       KGuiItem(i18n("Copy it locally"))
                                      )
       )
    {
        KIO::Job *job = KIO::file_copy(_localHistoryUrl, _remoteHistoryUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onHistorySyncFinished(KJob *)));
    }
    else
    {
        KIO::Job *job = KIO::file_copy(_remoteHistoryUrl, _localHistoryUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onHistorySyncFinished(KJob *)));
    }
}


void SyncManager::onHistorySyncFinished(KJob *job)
{
    if (job->error())
    {
        job->uiDelegate()->showErrorMessage();
        emit syncHistoryFinished(false);
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    ReKonfig::setLastSyncDateTime(now);
    emit syncHistoryFinished(true);
}


// ---------------------------------------------------------------------------------------


void SyncManager::onPasswordsStatFinished(KJob *job)
{
    if (job->error() ||
            KMessageBox::questionYesNo(0,
                                       i18n("A remote passwords file has just present in your remote server."),
                                       i18n("Server notification"),
                                       KGuiItem(i18n("Overwrite it")),
                                       KGuiItem(i18n("Copy it locally"))
                                      )
       )
    {
        KIO::Job *job = KIO::file_copy(_localPasswordsUrl, _remotePasswordsUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onPasswordsSyncFinished(KJob *)));
    }
    else
    {
        KIO::Job *job = KIO::file_copy(_remotePasswordsUrl, _localPasswordsUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob *)), this, SLOT(onPasswordsSyncFinished(KJob *)));
    }
}


void SyncManager::onPasswordsSyncFinished(KJob *job)
{
    if (job->error())
    {
        job->uiDelegate()->showErrorMessage();
        emit syncPasswordsFinished(false);
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    ReKonfig::setLastSyncDateTime(now);
    emit syncPasswordsFinished(true);
}
