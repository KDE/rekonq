/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


#ifndef REKONQ_APPLICATION_H
#define REKONQ_APPLICATION_H

// KDE Includes
#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KIcon>
#include <KUrl>
#include <KJob>
#include <kio/job.h>
#include <kio/jobclasses.h>

// Forward Declarations
class MainWindow;
class WebView;
class CookieJar;
class HistoryManager;
class NetworkAccessManager;

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
    WebView* newTab();

    KIcon icon(const KUrl &url) const;

    /**
     * This method lets you to download a file from a source remote url
     * to a local destination url.
     */
    void downloadUrl(const KUrl &srcUrl, const KUrl &destUrl);

    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static NetworkAccessManager *networkAccessManager();

private slots:

    /**
     * Any actions that can be delayed until the window is visible
     */
    void postLaunch();
    void openUrl(const KUrl &url);

private:
    static HistoryManager *s_historyManager;
    static NetworkAccessManager *s_networkAccessManager;

    MainWindow* m_mainWindow;
};

#endif // REKONQ_APPLICATION_H

