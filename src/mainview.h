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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */




#ifndef TABWIDGET_H
#define TABWIDGET_H

// KDE Includes
#include <KTabWidget>

// Forward Declarations
class WebView;
class TabBar;

class KUrl;
class KAction;
class KMenu;

class QWebFrame;
class QCompleter;
class QStackedWidget;
class QLineEdit;
class QUrl;


/**
 *   TabWidget that contains WebViews and a stack widget of associated line edits.
 *
 *   Connects up the current tab's signals to this class's signal and uses WebActionMapper
 *   to proxy the actions.
 *
 */

class MainView : public KTabWidget
{
    Q_OBJECT

public:
    MainView(QWidget *parent = 0);
    ~MainView();


signals:
    // tab widget signals
    void loadUrlPage(const KUrl &url);
    void tabsChanged();
    void lastTabClosed();

    // current tab signals
    void setCurrentTitle(const QString &url);
    void showStatusBarMessage(const QString &message);
    void linkHovered(const QString &link);
    void loadProgress(int progress);
    void geometryChangeRequested(const QRect &geometry);
    void menuBarVisibilityChangeRequested(bool visible);
    void statusBarVisibilityChangeRequested(bool visible);
    void toolBarVisibilityChangeRequested(bool visible);
    void printRequested(QWebFrame *frame);

public:
    void clear();

    KAction *recentlyClosedTabsAction() const;

    QWidget *lineEditStack() const;
    QLineEdit *currentLineEdit() const;
    WebView *currentWebView() const;
    WebView *webView(int index) const;
    QLineEdit *lineEdit(int index) const;
    int webViewIndex(WebView *webView) const;

    /** 
     * show and hide TabBar if user doesn't choose 
     * "Always Show TabBar" option
     *
     */
    void showTabBar();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);


public slots:
    void loadUrlInCurrentTab(const KUrl &url);
    WebView *newTab(bool makeCurrent = true);
    void cloneTab(int index = -1);
    void closeTab(int index = -1);
    void closeOtherTabs(int index);
    void reloadTab(int index = -1);
    void reloadAllTabs();
    void nextTab();
    void previousTab();

    // WEB slot actions
    void slotWebReload();
    void slotWebStop();
    void slotWebBack();
    void slotWebForward();
    void slotWebUndo();
    void slotWebRedo();
    void slotWebCut();
    void slotWebCopy();
    void slotWebPaste();

private slots:
    void currentChanged(int index);
    void aboutToShowRecentTabsMenu();
    void aboutToShowRecentTriggeredAction(QAction *action); // need QAction!
    void webViewLoadStarted();
    void webViewIconChanged();
    void webViewTitleChanged(const QString &title);
    void webViewUrlChanged(const QUrl &url);
    void lineEditReturnPressed();
    void windowCloseRequested();
    void moveTab(int fromIndex, int toIndex);

private:

    KAction *m_recentlyClosedTabsAction;

    KMenu *m_recentlyClosedTabsMenu;
    static const int m_recentlyClosedTabsSize = 10;
    QList<KUrl> m_recentlyClosedTabs;

    QCompleter *m_lineEditCompleter;
    QStackedWidget *m_lineEdits;
    TabBar *m_tabBar;
};

#endif

