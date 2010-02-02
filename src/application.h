/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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


// KDE Includes
#include <KUniqueApplication>
#include <KIcon>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <ThreadWeaver/Job>

// Qt Includes
#include <QPointer>
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


typedef QList< QPointer<MainWindow> > MainWindowList;


namespace Rekonq
{
    /**
     * @short notifying message status
     * Different message status
     */

    enum Notify
    {
        Success,    ///< url successfully (down)loaded
        Error,      ///< url failed to (down)load
        Download,   ///< downloading url
        Info        ///< information, (default)
    };

    /**
     * @short Open link options
     * Different modes of opening new tab
     */
    enum OpenType
    {
        CurrentTab,     ///< open url in current tab
        SettingOpenTab, ///< open url according to users settings
        NewCurrentTab,  ///< open url in new tab and make it current
        NewBackTab,     ///< open url in new tab in background
        NewWindow       ///< open url in new window
    };

}


/**
  *
  */
class Application : public KUniqueApplication
{
    Q_OBJECT

public:
    Application();
    ~Application();
    int newInstance();
    static Application *instance();

    MainWindow *mainWindow();
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

    MainWindow *newMainWindow();

    void loadUrl( const KUrl& url,
                  const Rekonq::OpenType& type = Rekonq::CurrentTab
                );
           
    void loadUrl( const QString& urlString,
                  const Rekonq::OpenType& type = Rekonq::CurrentTab
                );    

    void removeMainWindow(MainWindow *window);

private slots:

    /**
     * Any actions that can be delayed until the window is visible
     */
    void postLaunch();

    void loadResolvedUrl(ThreadWeaver::Job *);
    
private:
    static QPointer<HistoryManager> s_historyManager;
    static QPointer<BookmarkProvider> s_bookmarkProvider;
    static QPointer<SessionManager> s_sessionManager;
    static QPointer<AdBlockManager> s_adblockManager;
    
    MainWindowList m_mainWindows;
};

#endif // APPLICATION_H
