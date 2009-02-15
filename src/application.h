/* ============================================================
 *
 * This file is a part of the rekonq project
 *
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


#ifndef REKONQ_APPLICATION_H
#define REKONQ_APPLICATION_H

// KDE Includes
#include <KApplication>
#include <KCmdLineArgs>
#include <KIcon>
#include <KUrl>
#include <KJob>
#include <kio/job.h>
#include <kio/jobclasses.h>

// Qt Includes
#include <QPointer>

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

class MainWindow;
class CookieJar;
class HistoryManager;
class NetworkAccessManager;

class Application : public KApplication
{
    Q_OBJECT

public:
    Application(KCmdLineArgs*, const QString &);
    ~Application();
    static Application *instance();

    bool isTheOnlyBrowser() const;
    MainWindow *mainWindow();
    QList<MainWindow*> mainWindows();
    KIcon icon(const KUrl &url) const;
    void downloadUrl(const KUrl &srcUrl, const KUrl &destUrl);

    void saveSession();
    bool canRestoreSession() const;

    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static NetworkAccessManager *networkAccessManager();

public slots:
    MainWindow *newMainWindow();
    void restoreLastSession();

private slots:
    void postLaunch();
    void openUrl(const KUrl &url);
    void newLocalSocketConnection();

private:
    void clean();

    static HistoryManager *s_historyManager;
    static NetworkAccessManager *s_networkAccessManager;

    QList<QPointer<MainWindow> > m_mainWindows;
    QLocalServer *m_localServer;
    QByteArray m_lastSession;
    mutable KIcon m_defaultIcon;
};

#endif // REKONQ_APPLICATION_H

