/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2014 by Andrea Diamantini <adjam7 at gmail dot com>
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

// Qt Includes
#include <QApplication>
#include <QStringList>
#include <QUrl>
#include <QPointer>

// Forward Declarations
class RekonqWindow;
class WebTab;
class WebPage;

class QCommandLineParser;

typedef QList< QPointer<RekonqWindow> > RekonqWindowList;
typedef QList<WebTab *> WebAppList;


// ---------------------------------------------------------------------------------------------------------------


#define rApp Application::instance()

/**
  * Rekonq Application class
  */
class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();

    static Application *instance();

    int webAppInstance(const QStringList &);
    int windowInstance(const QStringList &, bool);

    RekonqWindow *rekonqWindow();
    RekonqWindowList rekonqWindowList();

    WebAppList webAppList();
    
    void bookmarksToolbarToggled(bool);

    void setCmdLineParser(QCommandLineParser * parser);

    void handleCmdLine(const QString & cwd = QString());

public Q_SLOTS:
    /**
     * Save application's configuration
     *
     * @see ReKonfig::self()->save();
     */
    void saveConfiguration() const;

    /**
     * @short load url
     *
     * @param url The url to load
     * @param type the type where loading the url. @see Rekonq::OpenType
     */
    void loadUrl(const QUrl& url,
                 const Rekonq::OpenType& type = Rekonq::CurrentTab
                );

    RekonqWindow *newWindow(bool withTab = true, bool PrivateBrowsingMode = false);
    RekonqWindow *newWindow(WebPage *pg);

    WebTab *newWebApp();

    void createWebAppShortcut(const QString & urlString = QString(), const QString & titleString = QString());

    void activateRequested(const QStringList &  arguments, const QString & cwd);

protected:
    // This is used to track which window was activated most recently
    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void toggleBookmarksToolbar(bool);

private:
    void setWindowInfo(RekonqWindow *);
    
private Q_SLOTS:
    void updateConfiguration();

    // clear private data
    void clearPrivateData();

    void queryQuit();

    void newPrivateBrowsingWindow();

    void pageCreated(WebPage *);

    void setCrashRecoverNeed(bool);
    bool isCrashRecoverNeeded() const;

private:
    RekonqWindowList m_rekonqWindows;
    WebAppList m_webApps;
    QCommandLineParser * parser;
};

#endif // APPLICATION_H
