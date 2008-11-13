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


#ifndef BROWSERMAINWINDOW_H
#define BROWSERMAINWINDOW_H

// Local Includes
#include "findwidget.h"

// KDE Includes
#include <KMainWindow>
#include <KIcon>
#include <KToolBar>
#include <KAction>
#include <KMenu>
#include <KBookmarkMenu>

// Qt Includes
#include <QUrl>


class AutoSaver;
class QWebFrame;
class TabWidget;
class ToolbarSearch;
class WebView;

/*!
    The MainWindow of the Browser Application.

    Handles the tab widget and all the actions
 */
class BrowserMainWindow : public KMainWindow {
    Q_OBJECT

public:
    BrowserMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~BrowserMainWindow();
    QSize sizeHint() const;

public:
    static QUrl guessUrlFromString(const QString &url);
    TabWidget *tabWidget() const;
    WebView *currentTab() const;
    QByteArray saveState(bool withTabs = true) const;
    bool restoreState(const QByteArray &state);

public slots:
    void loadPage(const QString &url);
    void slotHome();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void save();

    void slotLoadProgress(int);
    void slotUpdateStatusbar(const QString &string);
    void slotUpdateWindowTitle(const QString &title = QString());

    void loadUrl(const QUrl &url);
    void slotPreferences();

    void slotFileNew();
    void slotFileOpen();
    void slotFilePrintPreview();
    void slotFilePrint();
    void slotPrivateBrowsing();
    void slotFileSaveAs();
    void slotEditFind();
    void slotEditFindNext();
    void slotEditFindPrevious();
/*    void slotShowBookmarksDialog();
    void slotAddBookmark();*/
    void slotViewTextBigger();
    void slotViewTextNormal();
    void slotViewTextSmaller();
    void slotViewStatusbar();
    void slotViewPageSource();
    void slotViewFullScreen(bool enable);

    void slotWebSearch();
    void slotToggleInspector(bool enable);
    void slotDownloadManager();
    void slotSelectLineEdit();

    void slotAboutToShowBackMenu();
    void slotAboutToShowForwardMenu();
    void slotAboutToShowWindowMenu();
    void slotOpenActionUrl(QAction *action);
    void slotShowWindow();
    void slotSwapFocus();
    void slotViewFindWidget();

    void printRequested(QWebFrame *frame);
    void geometryChangeRequested(const QRect &geometry);

private:
    void loadDefaultState();
    void setupMenu();
    void setupToolBar();
    void updateStatusbarActionText(bool visible);

private:

    KToolBar *m_navigationBar;
    ToolbarSearch *m_toolbarSearch;
    TabWidget *m_tabWidget;
    AutoSaver *m_autoSaver;

    QAction *m_historyBack;
    KMenu *m_historyBackMenu;
    QAction *m_historyForward;
    KMenu *m_historyForwardMenu;
    KMenu *m_windowMenu;

    KAction *m_stop;
    QAction *m_reload;
    KAction *m_stopReload;
    KAction *m_goHome;
    KAction *m_viewStatusbar;
    QAction *m_restoreLastSession;
    KAction *m_addBookmark;

    KIcon m_reloadIcon;
    KIcon m_stopIcon;

    FindWidget *m_findWidg;
    QString m_lastSearch;

    KBookmarkMenu* m_bookmarkMenu;
};

#endif // BROWSERMAINWINDOW_H

