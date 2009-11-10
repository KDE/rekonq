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


#ifndef MAINVIEW_H
#define MAINVIEW_H


// Local Includes
#include "rekonqprivate_export.h"
#include "webview.h"
#include "webpage.h"
#include "application.h"
#include "history.h"
#include "mainwindow.h"

// KDE Includes
#include <KTabWidget>

// Qt Includes
#include <QtGui/QToolButton>

// Forward Declarations
class QUrl;
class QWebFrame;
class QLabel;
class QMouseEvent;

class TabBar;
class UrlBar;


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

public:

    UrlBar *urlBar() const;
    WebView *webView(int index) const;

    TabBar *tabBar() const;
    WebView *currentWebView() const;
    int webViewIndex(WebView *webView) const;

    /**
     * show and hide TabBar if user doesn't choose
     * "Always Show TabBar" option
     *
     */
    void updateTabBar();
    
    QToolButton *addTabButton() const;
    void clear();

    /**
     * This function creates a new empty tab
     * with a webview inside
     * @param focused   decide if you wannna give focus 
     *                  (or not) to this new tab (default true)
     * @param nearParent  decide if you wanna create new tab near current or not
     * @return the webview embedded in the new tab
     */
    WebView *newWebView(bool focused = true, bool nearParent = false);

    QList<HistoryItem> recentlyClosedTabs();

signals:
    // tab widget signals
    void tabsChanged();
    void lastTabClosed();

    // current tab signals
    void setCurrentTitle(const QString &url);
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

    void slotCloneTab(int index = -1);
    void slotCloseTab(int index = -1);
    void slotCloseOtherTabs(int index);
    void slotReloadTab(int index = -1);
    void slotReloadAllTabs();
    void nextTab();
    void previousTab();

    // WEB slot actions
    void slotWebReload();
    void slotWebStop();

private slots:
    void slotCurrentChanged(int index);

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

    UrlBar *m_urlBar;
    TabBar *m_tabBar;

    QString m_loadingGitPath;

    // the new tab button
    QToolButton *m_addTabButton;

    int m_currentTabIndex;

    QList<HistoryItem> m_recentlyClosedTabs;

    MainWindow *m_parentWindow;
};

#endif // MAINVIEW_H
