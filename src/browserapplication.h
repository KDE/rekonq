/* ============================================================
 *
 * This file is a part of the reKonq project
 *
 * Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
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


#ifndef BROWSERAPPLICATION_H
#define BROWSERAPPLICATION_H

// KDE Includes
#include <KApplication>
#include <KCmdLineArgs>
#include <KIcon>
#include <KUrl>

// Qt Includes
#include <QPointer>

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

class BrowserMainWindow;
class CookieJar;
class DownloadManager;
class HistoryManager;
class NetworkAccessManager;

class BrowserApplication : public KApplication
{
    Q_OBJECT

public:
    BrowserApplication(KCmdLineArgs*, const QString &);
    ~BrowserApplication();
    static BrowserApplication *instance();
    void loadSettings();

    bool isTheOnlyBrowser() const;
    BrowserMainWindow *mainWindow();
    QList<BrowserMainWindow*> mainWindows();
    KIcon icon(const KUrl &url) const;

    void saveSession();
    bool canRestoreSession() const;

    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static DownloadManager *downloadManager();
    static NetworkAccessManager *networkAccessManager();

public slots:
    BrowserMainWindow *newMainWindow();
    void restoreLastSession();

private slots:
    void postLaunch();
    void openUrl(const KUrl &url);
    void newLocalSocketConnection();

private:
    void clean();
    void installTranslator(const QString &name);

    static HistoryManager *s_historyManager;
    static DownloadManager *s_downloadManager;
    static NetworkAccessManager *s_networkAccessManager;

    QList<QPointer<BrowserMainWindow> > m_mainWindows;
    QLocalServer *m_localServer;
    QByteArray m_lastSession;
    mutable KIcon m_defaultIcon;
};

#endif // BROWSERAPPLICATION_H

