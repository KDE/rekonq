/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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

// KDE Includes
#include <KUniqueApplication>
#include <KIcon>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <ThreadWeaver/Job>

// Qt Includes
#include <QWeakPointer>
#include <QList>

// Forward Declarations
class KIcon;
class KUrl;
class BookmarkProvider;
class HistoryManager;
class MainWindow;
class SessionManager;
class AdBlockManager;
class WebView;


typedef QList< QWeakPointer<MainWindow> > MainWindowList;


/**
  *
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

    static KIcon icon(const KUrl &url);

    static HistoryManager *historyManager();
    static BookmarkProvider *bookmarkProvider();
    static SessionManager *sessionManager();
    static AdBlockManager *adblockManager();

public slots:
    /**
     * Save application's configuration
     *
     * @see ReKonfig::self()->writeConfig();
     */
    void saveConfiguration() const;

    void loadUrl(const KUrl& url,
                 const Rekonq::OpenType& type = Rekonq::CurrentTab
                );

    void newWindow();
    void removeMainWindow(MainWindow *window);

private slots:

    /**
     * Any actions that can be delayed until the window is visible
     */
    void postLaunch();

    void loadResolvedUrl(ThreadWeaver::Job *);

    void updateConfiguration();

private:
    static QWeakPointer<HistoryManager> s_historyManager;
    static QWeakPointer<BookmarkProvider> s_bookmarkProvider;
    static QWeakPointer<SessionManager> s_sessionManager;
    static QWeakPointer<AdBlockManager> s_adblockManager;

    MainWindowList m_mainWindows;
};

#endif // APPLICATION_H
