/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012-2013 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include <KUrl>

// Qt Includes
#include <QWeakPointer>

// Forward Declarations
class RekonqWindow;

class WebTab;
class WebPage;

#include <config-kactivities.h>

#ifdef HAVE_KACTIVITIES
namespace KActivities {
    class Consumer;
}
#endif

typedef QList< QWeakPointer<RekonqWindow> > RekonqWindowList;
typedef QList<WebTab *> WebAppList;

#ifdef HAVE_KACTIVITIES
typedef QHash< QString, RekonqWindowList > ActivityTabsMap;
#endif


// ---------------------------------------------------------------------------------------------------------------


#define rApp Application::instance()

/**
  * Rekonq Application class
  */
class Application : public KUniqueApplication
{
    Q_OBJECT

public:
    Application();
    ~Application();

    int newInstance();
    static Application *instance();

    RekonqWindow *rekonqWindow(const QString & activityID = QString());
    RekonqWindowList rekonqWindowList();

    /**
     * @returns the list of windows associated with activity whose id is @param activityID
     * @param activityID the ID of the activity  (if empty, it is interpreted as the current activity)
     * @note If activities are disabled, returns the function returns the list of all tabs.
     */
    RekonqWindowList tabsForActivity(const QString & activityID = QString());
    
    /**
     * @returns the true if there are windows associated with activity whose id is @param activityID
     * @param activityID the ID of the activity  (if empty, it is interpreted as the current activity)
     * @note If activities are disabled, returns true if there are any tabs.
     */
    bool haveWindowsForActivity(const QString & activityID = QString());

    WebAppList webAppList();
    
    void bookmarksToolbarToggled(bool);

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

    RekonqWindow *newWindow(bool withTab = true, bool PrivateBrowsingMode = false);
    RekonqWindow *newWindow(WebPage *pg);

    WebTab *newWebApp();

    void createWebAppShortcut(const QString & urlString = QString(), const QString & titleString = QString());

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

private:
    RekonqWindowList m_rekonqWindows;
    WebAppList m_webApps;
    
#ifdef HAVE_KACTIVITIES
    ActivityTabsMap m_activityRekonqWindowsMap;
    KActivities::Consumer *m_activityConsumer;
#endif

};

#endif // APPLICATION_H
