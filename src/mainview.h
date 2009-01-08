/* ============================================================
 *
 * This file is a part of the rekonq project
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



#ifndef TABWIDGET_H
#define TABWIDGET_H

// KDE Includes
#include <KAction>

// Qt Includes
#include <QWebPage>

class WebView;
/**
 *   A proxy object that connects a single browser action
 *   to one child webpage action at a time.
 *
 *   Example usage: used to keep the main window stop action in sync with
 *   the current tabs webview's stop action.
 */
class WebActionMapper : public QObject
{
    Q_OBJECT

public:
    WebActionMapper(KAction *root, QWebPage::WebAction webAction, QObject *parent);
    QWebPage::WebAction webAction() const;
    void addChild(KAction *action);
    void updateCurrent(QWebPage *currentParent);

private slots:
    void rootTriggered();
    void childChanged();
    void rootDestroyed();
    void currentDestroyed();

private:
    QWebPage *m_currentParent;
    KAction *m_root;
    QWebPage::WebAction m_webAction;
};


// ----------------------------------------------------------------------------------------------------------------------------

// Local Includes
#include "tabbar.h"

// KDE Includes
#include <KUrl>
#include <KMenu>
#include <KTabWidget>

// Qt Includes
#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QCompleter;
class QMenu;
class QStackedWidget;
QT_END_NAMESPACE

/**
 *   TabWidget that contains WebViews and a stack widget of associated line edits.
 *
 *   Connects up the current tab's signals to this class's signal and uses WebActionMapper
 *   to proxy the actions.
 */
class MainView : public KTabWidget
{
    Q_OBJECT

public:
    MainView(QWidget *parent = 0);
    ~MainView();


signals:
    // tab widget signals
    void loadPage(const QString &url);
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
    void addWebAction(KAction *action, QWebPage::WebAction webAction);

    KAction *newTabAction() const;
    KAction *closeTabAction() const;
    KAction *recentlyClosedTabsAction() const;
    KAction *nextTabAction() const;
    KAction *previousTabAction() const;

    QWidget *lineEditStack() const;
    QLineEdit *currentLineEdit() const;
    WebView *currentWebView() const;
    WebView *webView(int index) const;
    QLineEdit *lineEdit(int index) const;
    int webViewIndex(WebView *webView) const;

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
    KAction *m_newTabAction;
    KAction *m_closeTabAction;
    KAction *m_nextTabAction;
    KAction *m_previousTabAction;

    KMenu *m_recentlyClosedTabsMenu;
    static const int m_recentlyClosedTabsSize = 10;
    QList<KUrl> m_recentlyClosedTabs;
    QList<WebActionMapper*> m_webActionList;

    QCompleter *m_lineEditCompleter;
    QStackedWidget *m_lineEdits;
    TabBar *m_tabBar;
};

#endif // TABWIDGET_H

