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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Local Includes
#include "findbar.h"
#include "searchbar.h"
#include "bookmarks.h"
#include "mainview.h"

// KDE Includes
#include <KXmlGuiWindow>
#include <KIcon>
#include <KAction>
#include <KToolBar>
#include <KMenu>

// Forward Declarations
class KUrl;
class QWebFrame;
class WebView;


/**
 * This class serves as the main window for rekonq.
 * It handles the menus, toolbars, and status bars.
 *
 */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    static KUrl guessUrlFromString(const QString &url);
    MainView *mainView() const;
    WebView *currentTab() const;
    virtual QSize sizeHint() const;

private:
    void setupActions();
    void setupHistoryMenu();
    void setupToolBars();

public slots:
    void slotHome();
    void loadUrl(const KUrl &url);
    void slotUpdateBrowser();

private slots:
    void slotUpdateConfiguration();
    void slotLoadProgress(int);
    void slotUpdateStatusbar(const QString &string);
    void slotUpdateActions();
    void slotUpdateWindowTitle(const QString &title = QString());
    void slotOpenLocation();
    void slotAboutToShowBackMenu();
    void geometryChangeRequested(const QRect &geometry);

    // history related
    void slotOpenActionUrl(QAction *action);
    void slotOpenPrevious();
    void slotOpenNext();

    // File Menu slots
//     void slotFileNew();
    void slotFileOpen();
    void slotFilePrintPreview();
    void slotFilePrint();
    void slotPrivateBrowsing(bool);
    void slotFileSaveAs();
    void printRequested(QWebFrame *frame);

    // Edit Menu slots
    void slotFind(const QString &);
    void slotFindNext();
    void slotFindPrevious();

    // View Menu slots
    void slotViewTextBigger();
    void slotViewTextNormal();
    void slotViewTextSmaller();
    void slotViewPageSource();
    void slotViewFullScreen(bool enable);
    void slotViewFindBar();

    // Tools Menu slots
    void slotToggleInspector(bool enable);

    // Settings Menu slots
    void slotShowMenubar(bool enable);
    void slotPreferences();

private:
    KMenu *m_historyBackMenu;
    KMenu *m_windowMenu;

    KAction *m_stopReloadAction;
    KAction *m_stopAction;
    KAction *m_reloadAction;
    KAction *m_historyBackAction;
    KAction *m_historyForwardAction;

    QString m_lastSearch;
    QString m_homePage;

    MainView *m_view;
    BookmarksProvider *m_bookmarksProvider;
    FindBar *m_findBar;
    SearchBar *m_searchBar;
};

#endif // MAINWINDOW_H

