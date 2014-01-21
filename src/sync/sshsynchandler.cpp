/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2014 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2013 by Radu Andries <admiral0 at tuxfamily dot org>
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
#include "sshsynchandler.h"
#include "sshsynchandler.moc"

// Auto Includes
#include "rekonq.h"

// KDE Includes
#include <KStandardDirs>
#include <klocalizedstring.h>

#include <KIO/Job>


SSHSyncHandler::SSHSyncHandler(QObject *parent)
    : SyncHandler(parent)
{
    qDebug() << "creating SSH handler...";
}


void SSHSyncHandler::initialLoadAndCheck()
{
    if (!ReKonfig::syncEnabled())
    {
        _firstTimeSynced = false;
        return;
    }

    // Bookmarks
    if (ReKonfig::syncBookmarks())
    {
        _remoteBookmarksUrl = QUrl();
        _remoteBookmarksUrl.setHost(ReKonfig::syncHost());
        _remoteBookmarksUrl.setScheme("fish");
        _remoteBookmarksUrl.setUserName(ReKonfig::syncUser());
        _remoteBookmarksUrl.setPassword(ReKonfig::syncPass());
        _remoteBookmarksUrl.setPort(ReKonfig::syncPort());
        _remoteBookmarksUrl.setPath(ReKonfig::syncPath() + QL1S("/bookmarks.xml"));

        const QString bookmarksFilePath = KStandardDirs::locateLocal("data", QL1S("konqueror/bookmarks.xml"));
        _localBookmarksUrl = KUrl(bookmarksFilePath);

        KIO::StatJob *job = KIO::stat(_remoteBookmarksUrl, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(onBookmarksStatFinished(KJob*)));
    }

    // History
    if (ReKonfig::syncHistory())
    {
        _remoteHistoryUrl = QUrl();
        _remoteHistoryUrl.setHost(ReKonfig::syncHost());
        _remoteHistoryUrl.setScheme("fish");
        _remoteHistoryUrl.setUserName(ReKonfig::syncUser());
        _remoteHistoryUrl.setPassword(ReKonfig::syncPass());
        _remoteHistoryUrl.setPort(ReKonfig::syncPort());
        _remoteHistoryUrl.setPath(ReKonfig::syncPath() + QL1S("/history"));

        const QString historyFilePath = KStandardDirs::locateLocal("appdata", "history");
        _localHistoryUrl = KUrl(historyFilePath);

        KIO::StatJob *job = KIO::stat(_remoteHistoryUrl, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(onHistoryStatFinished(KJob*)));
    }

    // Passwords
    if (ReKonfig::syncPasswords())
    {
        _remotePasswordsUrl = QUrl();
        _remotePasswordsUrl.setHost(ReKonfig::syncHost());
        _remotePasswordsUrl.setScheme("fish");
        _remotePasswordsUrl.setUserName(ReKonfig::syncUser());
        _remotePasswordsUrl.setPassword(ReKonfig::syncPass());
        _remotePasswordsUrl.setPort(ReKonfig::syncPort());
        _remotePasswordsUrl.setPath(ReKonfig::syncPath() + QL1S("/kdewallet.kwl"));

        const QString passwordsFilePath = KStandardDirs::locateLocal("data", QL1S("kwallet/kdewallet.kwl"));
        _localPasswordsUrl = KUrl(passwordsFilePath);

        KIO::StatJob *job = KIO::stat(_remotePasswordsUrl, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(onPasswordsStatFinished(KJob*)));
    }
}


bool SSHSyncHandler::syncRelativeEnabled(bool check)
{
    if (!ReKonfig::syncEnabled())
        return false;

    if (!_firstTimeSynced)
        return false;

    return check;
}


// ---------------------------------------------------------------------------------------


void SSHSyncHandler::syncBookmarks()
{
    qDebug() << "syncing now...";

    if (!syncRelativeEnabled(ReKonfig::syncBookmarks()))
        return;

    KIO::Job *job = KIO::file_copy(_localBookmarksUrl, _remoteBookmarksUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(onBookmarksSyncFinished(KJob*)));
}


void SSHSyncHandler::onBookmarksStatFinished(KJob *job)
{
    if (job->error())
    {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST)
        {
            KIO::Job *job = KIO::file_copy(_localBookmarksUrl, _remoteBookmarksUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(job, SIGNAL(finished(KJob*)), this, SLOT(onBookmarksSyncFinished(KJob*)));

            emit syncStatus(Rekonq::Bookmarks, true, i18n("Remote bookmarks file does not exists. Exporting local copy..."));
            _firstTimeSynced = true;
        }
        else
        {
            emit syncStatus(Rekonq::Bookmarks, false, job->errorString());
        }
    }
    else
    {
        KIO::Job *job = KIO::file_copy(_remoteBookmarksUrl, _localBookmarksUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(onBookmarksSyncFinished(KJob*)));

        emit syncStatus(Rekonq::Bookmarks, true, i18n("Remote bookmarks file exists. Syncing local copy..."));
        _firstTimeSynced = true;
    }
}


void SSHSyncHandler::onBookmarksSyncFinished(KJob *job)
{
    if (job->error())
    {
        emit syncStatus(Rekonq::Bookmarks, false, job->errorString());
        emit syncBookmarksFinished(false);
        return;
    }

    emit syncBookmarksFinished(true);
}


// ---------------------------------------------------------------------------------------


void SSHSyncHandler::syncHistory()
{
    qDebug() << "syncing now...";

    if (!syncRelativeEnabled(ReKonfig::syncHistory()))
        return;

    KIO::Job *job = KIO::file_copy(_localHistoryUrl, _remoteHistoryUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(onHistorySyncFinished(KJob*)));
}


void SSHSyncHandler::onHistoryStatFinished(KJob *job)
{
    if (job->error())
    {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST)
        {
            KIO::Job *job = KIO::file_copy(_localHistoryUrl, _remoteHistoryUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(job, SIGNAL(finished(KJob*)), this, SLOT(onHistorySyncFinished(KJob*)));

            emit syncStatus(Rekonq::History, true, i18n("Remote history file does not exists. Exporting local copy..."));
            _firstTimeSynced = true;
        }
        else
        {
            emit syncStatus(Rekonq::History, false, job->errorString());
        }
    }
    else
    {
        KIO::Job *job = KIO::file_copy(_remoteHistoryUrl, _localHistoryUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(onHistorySyncFinished(KJob*)));

        emit syncStatus(Rekonq::History, true, i18n("Remote history file exists. Syncing local copy..."));
        _firstTimeSynced = true;
    }
}


void SSHSyncHandler::onHistorySyncFinished(KJob *job)
{
    if (job->error())
    {
        emit syncStatus(Rekonq::History, false, job->errorString());
        emit syncHistoryFinished(false);
        return;
    }

    emit syncHistoryFinished(true);
}


// ---------------------------------------------------------------------------------------


void SSHSyncHandler::syncPasswords()
{
    qDebug() << "syncing now...";

    if (!syncRelativeEnabled(ReKonfig::syncPasswords()))
        return;

    KIO::Job *job = KIO::file_copy(_localPasswordsUrl, _remotePasswordsUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(onPasswordsSyncFinished(KJob*)));
}


void SSHSyncHandler::onPasswordsStatFinished(KJob *job)
{
    if (job->error())
    {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST)
        {
            KIO::Job *job = KIO::file_copy(_localPasswordsUrl, _remotePasswordsUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(job, SIGNAL(finished(KJob*)), this, SLOT(onPasswordsSyncFinished(KJob*)));

            emit syncStatus(Rekonq::Passwords, true, i18n("Remote passwords file does not exists. Exporting local copy..."));
            _firstTimeSynced = true;
        }
        else
        {
            emit syncStatus(Rekonq::Passwords, false, job->errorString());
        }
    }
    else
    {
        KIO::Job *job = KIO::file_copy(_remotePasswordsUrl, _localPasswordsUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(onPasswordsSyncFinished(KJob*)));

        emit syncStatus(Rekonq::Passwords, true, i18n("Remote passwords file exists. Syncing local copy..."));
        _firstTimeSynced = true;
    }
}


void SSHSyncHandler::onPasswordsSyncFinished(KJob *job)
{
    if (job->error())
    {
        emit syncStatus(Rekonq::Passwords, false, job->errorString());
        emit syncPasswordsFinished(false);
        return;
    }

    emit syncPasswordsFinished(true);
}
