/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2011-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Config Includes
#include <config-qca2.h>
#include <config-qtoauth.h>

// Local Includes
#include "application.h"
#include "bookmarkmanager.h"
#include "historymanager.h"

#include "syncassistant.h"
#include "ftpsynchandler.h"
#include "googlesynchandler.h"

#if (defined HAVE_QCA2 && defined HAVE_QTOAUTH)
#include "operasynchandler.h"
#endif

// KDE Includes
#include <KLocalizedString>

// Qt Includes
#include <QPointer>


QWeakPointer<SyncManager> SyncManager::s_syncManager;


SyncManager *SyncManager::self()
{
    if (s_syncManager.isNull())
    {
        s_syncManager = new SyncManager(qApp);
    }
    return s_syncManager.data();
}


// ----------------------------------------------------------------------------------------------


SyncManager::SyncManager(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}


SyncManager::~SyncManager()
{
    if (!_syncImplementation.isNull())
    {
        delete _syncImplementation.data();
        _syncImplementation.clear();
    }
}


void SyncManager::loadSettings()
{
    if (ReKonfig::syncEnabled())
    {
        // reset syncer
        if (!_syncImplementation.isNull())
        {
            delete _syncImplementation.data();
            _syncImplementation.clear();
        }

        switch (ReKonfig::syncType())
        {
        case 0:
            _syncImplementation = new FTPSyncHandler(this);
            break;
        case 1:
            _syncImplementation = new GoogleSyncHandler(this);
            break;
#if (defined HAVE_QCA2 && defined HAVE_QTOAUTH)
        case 2:
            _syncImplementation = new OperaSyncHandler(this);
            break;
#endif
        default:
            kDebug() << "/dev/null";
            return;
        }


        // --- Connect syncmanager to bookmarks & history manager

        // bookmarks
        ReKonfig::syncBookmarks()
        ? connect(BookmarkManager::self(), SIGNAL(bookmarksUpdated()), this, SLOT(syncBookmarks()))
        : disconnect(BookmarkManager::self(), SIGNAL(bookmarksUpdated()), this, SLOT(syncBookmarks()))
        ;

        // history
        ReKonfig::syncHistory()
        ? connect(HistoryManager::self(), SIGNAL(historySaved()), this, SLOT(syncHistory()))
        : disconnect(HistoryManager::self(), SIGNAL(historySaved()), this, SLOT(syncHistory()))
        ;

        _syncImplementation.data()->initialLoadAndCheck();
        // NOTE: password sync will be called just on save
    }
    else
    {
        // bookmarks
        disconnect(BookmarkManager::self(), SIGNAL(bookmarksUpdated()), this, SLOT(syncBookmarks()));

        // history
        disconnect(HistoryManager::self(), SIGNAL(historySaved()), this, SLOT(syncHistory()));
    }
}


void SyncManager::showSettings()
{
    QPointer<SyncAssistant> dialog = new SyncAssistant();
    dialog->exec();

    dialog->deleteLater();
}


// ---------------------------------------------------------------------------------------


void SyncManager::syncBookmarks()
{
    if (!_syncImplementation.isNull())
    {
        _syncImplementation.data()->syncBookmarks();
    }
}


void SyncManager::syncHistory()
{
    if (!_syncImplementation.isNull())
    {
        _syncImplementation.data()->syncHistory();
    }
}


void SyncManager::syncPasswords()
{
    if (!_syncImplementation.isNull())
    {
        _syncImplementation.data()->syncPasswords();
    }
}

