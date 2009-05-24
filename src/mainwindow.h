/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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
#include "searchbar.h"
#include "bookmarks.h"
#include "mainview.h"
#include "webview.h"

// KDE Includes
#include <KXmlGuiWindow>
#include <KToolBar>

// Forward Declarations
class QWebFrame;

class KUrl;
class KAction;
class KActionMenu;
class KIcon;
class KMenu;

class FindBar;
class HistoryMenu;
class SidePanel;
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
    QAction *actionByName(const QString name);
    virtual QSize sizeHint() const;

private:
    void setupActions();
    void setupHistoryMenu();
    void setupToolBars();
    void setupSidePanel();
    SidePanel *sidePanel();

public slots:
    void slotHome();
    void loadUrl(const KUrl &url);
    void slotUpdateBrowser();

protected:
    bool queryClose();

private slots:
    void postLaunch();
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
    MainView *m_view;
    FindBar *m_findBar;
    SearchBar *m_searchBar;
    SidePanel *m_sidePanel;

    KMenu *m_historyBackMenu;
    KMenu *m_windowMenu;
    KActionMenu *m_historyActionMenu;

    KAction *m_stopReloadAction;
    KAction *m_stopAction;
    KAction *m_reloadAction;
    KAction *m_historyBackAction;
    KAction *m_historyForwardAction;

    QString m_lastSearch;
    QString m_homePage;
};

#endif // MAINWINDOW_H

