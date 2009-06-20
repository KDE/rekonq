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


#ifndef TABWIDGET_H
#define TABWIDGET_H

// Local Includes
#include "webview.h"
#include "webpage.h"
#include "application.h"

// KDE Includes
#include <KTabWidget>

// Qt Includes
#include <QtGui/QToolButton>

// Forward Declarations
class QUrl;
class QWebFrame;
class QLabel;

class KAction;
class KMenu;
class KUrl;

class StackedUrlBar;
class TabBar;
class UrlBar;


namespace Rekonq
{
    /**
     * @short notifying message status
     * Different message status
     */

    enum Notify
    {
        Success,    ///< url successfully (down)loaded
        Error,      ///< url failed to (down)load
        Download,   ///< downloading url
        Info        ///< information
    };
}


/**
 *  This class represent rekonq Main View. It contains all WebViews and a stack widget
 *  of associated line edits.
 *
 */

class MainView : public KTabWidget
{
    Q_OBJECT

public:
    MainView(QWidget *parent = 0);
    ~MainView();

public:

    UrlBar *urlBar(int index) const;
    UrlBar *currentUrlBar() const;
    WebView *webView(int index) const;

    // inlines
    TabBar *tabBar() const;
    StackedUrlBar *urlBarStack() const;
    WebView *currentWebView() const;
    int webViewIndex(WebView *webView) const;
    KAction *recentlyClosedTabsAction() const;
    void setMakeBackTab(bool b);

    /**
     * show and hide TabBar if user doesn't choose
     * "Always Show TabBar" option
     *
     */
    void showTabBar();
    void clear();


signals:
    // tab widget signals
    void tabsChanged();
    void lastTabClosed();

    // current tab signals
    void setCurrentTitle(const QString &url);
    void showStatusBarMessage(const QString &message);
    void linkHovered(const QString &link);
    void loadProgress(int progress);

    void printRequested(QWebFrame *frame);

public slots:
    /**
     * Core browser slot. This create a new tab with a WebView inside
     * for browsing.
     *
     * @return a pointer to the new WebView
     */
    WebView *newWebView(Rekonq::OpenType type = Rekonq::Default);

    /**
     * Core browser slot. Load an url in a webview
     *
     * @param url The url to load
     */
    void loadUrl(const KUrl &url);

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
    void slotWebBack();
    void slotWebForward();
    void slotWebUndo();
    void slotWebRedo();
    void slotWebCut();
    void slotWebCopy();
    void slotWebPaste();

private slots:
    void slotCurrentChanged(int index);

    void webViewLoadStarted();
    void webViewLoadFinished(bool ok);
    void webViewIconChanged();
    void webViewTitleChanged(const QString &title);
    void webViewUrlChanged(const QUrl &url);

    void windowCloseRequested();

    /**
     * This functions move tab info "from index to index"
     *
     * @param fromIndex the index from which we move
     *
     * @param toIndex the index to which we move
     */
    void moveTab(int fromIndex, int toIndex);

    void postLaunch();

protected:

    virtual void mouseDoubleClickEvent(QMouseEvent *event);


private:

    void addTabButtonPosition();

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

    StackedUrlBar *m_urlBars;
    TabBar *m_tabBar;

    QString m_loadingGitPath;

    bool m_makeBackTab;

    QToolButton *m_addTabButton;
};

#endif
