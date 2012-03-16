/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef APPLICATION_H
#define APPLICATION_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "downloaditem.h"

// KDE Includes
#include <KUniqueApplication>

// Qt Includes
#include <QtCore/QDateTime>
#include <QtCore/QWeakPointer>

class QWebHistory;

// Forward Declarations
class AdBlockManager;
class BookmarkManager;
class DownloadManager;
class HistoryManager;
class IconManager;
class MainWindow;
class OpenSearchManager;
class SessionManager;
class UserAgentManager;
class SyncManager;
class WebTab;

class KAction;


typedef QList< QWeakPointer<MainWindow> > MainWindowList;


// ---------------------------------------------------------------------------------------------------------------


#define rApp Application::instance()

/**
  * Rekonq Application class
  */
class REKONQ_TESTS_EXPORT Application : public KUniqueApplication
{
    Q_OBJECT

public:
    Application();
    ~Application();

    int newInstance();
    static Application *instance();

    MainWindow *mainWindow();
    MainWindow *newMainWindow(bool withTab = true);
    MainWindowList mainWindowList();

    HistoryManager *historyManager();
    BookmarkManager *bookmarkManager();
    SessionManager *sessionManager();
    AdBlockManager *adblockManager();
    OpenSearchManager *opensearchManager();
    IconManager *iconManager();
    DownloadManager *downloadManager();
    UserAgentManager *userAgentManager();
    SyncManager *syncManager();

    KAction *privateBrowsingAction()
    {
        return _privateBrowsingAction;
    };
    
public Q_SLOTS:
    /**
     * Save application's configuration
     *
     * @see ReKonfig::self()->writeConfig();
     */
    void saveConfiguration() const;

    /**
     * @short load url
     *
     * @param url The url to load
     * @param type the type where loading the url. @see Rekonq::OpenType
     */
    void loadUrl(const KUrl& url,
                 const Rekonq::OpenType& type = Rekonq::CurrentTab
                );

    void newWindow();
    void removeMainWindow(MainWindow *window);

protected:
    // This is used to track which window was activated most recently
    bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void updateConfiguration();

    // the general place to set private browsing
    void setPrivateBrowsingMode(bool);

    void queryQuit();

    void createWebAppShortcut();

private:
    QWeakPointer<HistoryManager> m_historyManager;
    QWeakPointer<BookmarkManager> m_bookmarkManager;
    QWeakPointer<SessionManager> m_sessionManager;
    QWeakPointer<AdBlockManager> m_adblockManager;
    QWeakPointer<OpenSearchManager> m_opensearchManager;
    QWeakPointer<IconManager> m_iconManager;
    QWeakPointer<DownloadManager> m_downloadManager;
    QWeakPointer<UserAgentManager> m_userAgentManager;
    QWeakPointer<SyncManager> m_syncManager;

    MainWindowList m_mainWindows;

    KAction *_privateBrowsingAction;
};

#endif // APPLICATION_H
