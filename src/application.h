/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 3, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



#ifndef APPLICATION_H
#define APPLICATION_H

// KDE Includes
#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KIcon>
#include <KUrl>
#include <KJob>
#include <kio/job.h>
#include <kio/jobclasses.h>

// Qt Includes
#include <QPointer>

// Forward Declarations
class KIcon;
class KUrl;
class BookmarkProvider;
class CookieJar;
class HistoryManager;
class MainWindow;
class NetworkAccessManager;
class WebView;


namespace Rekonq
{
    /**
     * @short Open link options
     * Different modes of opening new tab
     */
    enum OpenType
    {
        Default,    ///< open url according to users settings
        New,        ///< open url in new tab and make it current
        Background  ///< open url in new tab in background
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
    WebView *newWebView(Rekonq::OpenType type = Rekonq::Default);

    static KIcon icon(const KUrl &url);

    static KUrl guessUrlFromString(const QString &url);

    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static NetworkAccessManager *networkAccessManager();
    static BookmarkProvider *bookmarkProvider();

public slots:
    /**
     * Save application's configuration
     * @see ReKonfig::self()->writeConfig();
     */
    void slotSaveConfiguration() const;


private slots:

    /**
     * Any actions that can be delayed until the window is visible
     */
    void postLaunch();


private:
    static QPointer<HistoryManager> s_historyManager;
    static QPointer<NetworkAccessManager> s_networkAccessManager;
    static QPointer<BookmarkProvider> s_bookmarkProvider;

    QPointer<MainWindow> m_mainWindow;
};

#endif // APPLICATION_H
