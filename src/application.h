/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
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

// KDE Includes
#include <KUniqueApplication>

// Qt Includes
#include <QtCore/QDateTime>
#include <QtCore/QWeakPointer>

// Forward Declarations
class AdBlockManager;
class BookmarkProvider;
class DownloadItem;
class HistoryManager;
class IconManager;
class MainWindow;
class OpenSearchManager;
class SessionManager;

class KAction;

namespace ThreadWeaver
{
class Job;
}


typedef QList< QWeakPointer<MainWindow> > MainWindowList;
typedef QList<DownloadItem> DownloadList;


class DownloadItem
{
public:
    DownloadItem() {}
    explicit DownloadItem(const QString &srcUrl,
                          const QString &destUrl,
                          const QDateTime &d
                         )
            : srcUrlString(srcUrl)
            , destUrlString(destUrl)
            , dateTime(d)
    {}

    QString srcUrlString;
    QString destUrlString;
    QDateTime dateTime;
};


// ---------------------------------------------------------------------------------------------------------------

#define rApp Application::instance()

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

    HistoryManager *historyManager();
    BookmarkProvider *bookmarkProvider();
    SessionManager *sessionManager();
    AdBlockManager *adblockManager();
    OpenSearchManager *opensearchManager();
    IconManager *iconManager();

    // DOWNLOADS MANAGEMENT METHODS
    void addDownload(const QString &srcUrl, const QString &destUrl);
    DownloadList downloads();
    bool clearDownloadsHistory();

    KAction *privateBrowsingAction()
    {
        return _privateBrowsingAction;
    };

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

protected:
    // This is used to track which window was activated most recently
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    /**
     * Any actions that can be delayed until the window is visible
     */
    void postLaunch();

    void loadResolvedUrl(ThreadWeaver::Job *);

    void updateConfiguration();

    // the general place to set private browsing
    void setPrivateBrowsingMode(bool);

private:
    QWeakPointer<HistoryManager> m_historyManager;
    QWeakPointer<BookmarkProvider> m_bookmarkProvider;
    QWeakPointer<SessionManager> m_sessionManager;
    QWeakPointer<AdBlockManager> m_adblockManager;
    QWeakPointer<OpenSearchManager> m_opensearchManager;
    QWeakPointer<IconManager> m_iconManager;

    MainWindowList m_mainWindows;

    KAction *_privateBrowsingAction;
};

#endif // APPLICATION_H
