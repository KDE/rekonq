/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
* Copyright (C) 2010 by Matthieu Gicquel <matgic78 at gmail dot com>
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


#ifndef MAINVIEW_H
#define MAINVIEW_H


// Rekonq Includes
#include "rekonq_defines.h"

// Local Includes
#include "historymanager.h"

// KDE Includes
#include <KTabWidget>

// Forward Declarations
class HistoryItem;
class MainWindow;
class StackedUrlBar;
class TabBar;
class UrlBar;
class WebTab;

class QLabel;
class QToolButton;
class QUrl;
class QWebFrame;


/**
 * This class represent rekonq Main View.
 * It contains all WebViews and the url bar.
 *
 */

class REKONQ_TESTS_EXPORT MainView : public KTabWidget
{
    Q_OBJECT

public:
    MainView(MainWindow *parent);
    ~MainView();

    inline StackedUrlBar *widgetBar() const { return m_widgetBar; }
    UrlBar *urlBar() const;
    WebTab *webTab(int index) const;

    TabBar *tabBar() const;
    WebTab *currentWebTab() const;

    /**
     * show and hide TabBar if user doesn't choose
     * "Always Show TabBar" option
     *
     */
    void updateTabBar();

    void setTabBarHidden(bool hide);

    inline QToolButton *addTabButton() const { return m_addTabButton; }

    /**
     * This function creates a new empty tab
     * with a webview inside
     * @param focused   decide if you wannna give focus
     *                  (or not) to this new tab (default true)
     * @return the webview embedded in the new tab
     */
    WebTab *newWebTab(bool focused = true);

    inline QList<HistoryItem> recentlyClosedTabs() { return m_recentlyClosedTabs; }

signals:
    // tab widget signals
    void tabsChanged();
    void lastTabClosed();

    // current tab signals
    void currentTitle(const QString &url);
    void showStatusBarMessage(const QString &message, Rekonq::Notify status = Rekonq::Info);
    void linkHovered(const QString &link);
    void browserTabLoading(bool);

    void printRequested(QWebFrame *frame);

public slots:
    /**
     * Core browser slot. This create a new tab with a WebView inside
     * for browsing and follows rekonq settings about opening there a
     * home/blank/rekonq page
     *
     */
    void newTab();

    // Indexed slots
    void cloneTab(int index = -1);
    void closeTab(int index = -1, bool del = true);
    void closeOtherTabs(int index = -1);
    void reloadTab(int index = -1);
    /**
     * Detaches tab at @c index to a new window.
     * If @c toWindow is not null, the tab is instead
     * added to existing MainWindow @c toWindow.
     */
    void detachTab(int index = -1, MainWindow *toWindow = NULL);

    void reloadAllTabs();
    void nextTab();
    void previousTab();

    void openLastClosedTab();
    void openClosedTab();
    void switchToTab();

    // WEB slot actions
    void webReload();
    void webStop();

private slots:
    void currentChanged(int index);

    void webViewLoadStarted();
    void webViewLoadFinished(bool ok);
    void webViewIconChanged();
    void webViewTitleChanged(const QString &title);
    void webViewUrlChanged(const QUrl &url);

    void windowCloseRequested();

    void postLaunch();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    void updateTabButtonPosition();

    /**
     * This function creates (if not exists) and returns a QLabel
     * with a loading QMovie.
     * Imported from Arora's code.
     *
     * @param index the tab index where inserting the animated label
     * @param addMovie creates or not a loading movie
     *
     * @return animated label's pointer
     */
    QLabel *animatedLoading(int index, bool addMovie);


// --------------------------------------------------------------------------

    StackedUrlBar *const m_widgetBar;

    QString m_loadingGitPath;

    // the new tab button
    QToolButton *const m_addTabButton;

    int m_currentTabIndex;

    QList<HistoryItem> m_recentlyClosedTabs;

    MainWindow *const m_parentWindow;
};

#endif // MAINVIEW_H
