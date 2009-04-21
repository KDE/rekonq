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


// Self Includes
#include "mainwindow.h"
#include "mainwindow.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "settings.h"
#include "history.h"
#include "cookiejar.h"
#include "networkaccessmanager.h"
#include "bookmarks.h"
#include "webview.h"
#include "mainview.h"
#include "bookmarks.h"
#include "download.h"
#include "findbar.h"
#include "sidepanel.h"

// KDE Includes
#include <KUrl>
#include <KStatusBar>
#include <KMenuBar>
#include <KShortcut>
#include <KStandardAction>
#include <KAction>
#include <KToggleFullScreenAction>
#include <KActionCollection>
#include <KMessageBox>
#include <KFileDialog>
#include <KMenu>
#include <KGlobalSettings>
#include <KPushButton>
#include <KTemporaryFile>


// Qt Includes
#include <QtGui>
#include <QtCore>
#include <QtWebKit>


MainWindow::MainWindow()
        : KXmlGuiWindow()
        , m_view(new MainView(this))
        , m_findBar(new FindBar(this))
        , m_searchBar(new SearchBar(this))
        , m_sidePanel(0)
{
    // accept dnd
    setAcceptDrops(true);

    // updating rekonq configuration
    slotUpdateConfiguration();

    // creating a centralWidget containing panel, m_view and the hidden findbar
    QWidget *centralWidget = new QWidget;
    centralWidget->setContentsMargins(0, 0, 0, 0);

    // setting layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
    layout->addWidget(m_findBar);
    centralWidget->setLayout(layout);

    // central widget
    setCentralWidget(centralWidget);

    // Adding Find Bar
    connect(m_findBar, SIGNAL(searchString(const QString &)), this, SLOT(slotFind(const QString &)));

    // setting size policies
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --------- connect signals and slots
    connect(m_view, SIGNAL(loadUrlPage(const KUrl &)), this, SLOT(loadUrl(const KUrl &)));
    connect(m_view, SIGNAL(setCurrentTitle(const QString &)), this, SLOT(slotUpdateWindowTitle(const QString &)));
    connect(m_view, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));
    connect(m_view, SIGNAL(geometryChangeRequested(const QRect &)), this, SLOT(geometryChangeRequested(const QRect &)));
    connect(m_view, SIGNAL(printRequested(QWebFrame *)), this, SLOT(printRequested(QWebFrame *)));
    connect(m_view, SIGNAL(menuBarVisibilityChangeRequested(bool)), menuBar(), SLOT(setVisible(bool)));
    connect(m_view, SIGNAL(statusBarVisibilityChangeRequested(bool)), statusBar(), SLOT(setVisible(bool)));

    // status bar message
    connect(m_view, SIGNAL(showStatusBarMessage(const QString&)), statusBar(), SLOT(showMessage(const QString&)));
    connect(m_view, SIGNAL(linkHovered(const QString&)), statusBar(), SLOT(showMessage(const QString&)));

    // update toolbar actions
    connect(m_view, SIGNAL(tabsChanged()), this, SLOT(slotUpdateActions()));
    connect(m_view, SIGNAL(currentChanged(int)), this, SLOT(slotUpdateActions()));
    // --------------------------------------
    
    slotUpdateWindowTitle();

    // then, setup our actions
    setupActions();

    // add a status bar
    statusBar()->show();

    // setting up toolbars: this has to be done BEFORE setupGUI!!
    setupToolBars();

    // ----- BOOKMARKS MENU: this has to be done BEFORE setupGUI!!
//     KAction *a = new KActionMenu(i18n("B&ookmarks"), this);
//     actionCollection()->addAction(QLatin1String("bookmarks"), a);
//     KActionMenu *bmMenu = Application::bookmarkProvider()->bookmarkActionMenu();
//     a->setMenu(bmMenu);

    KActionMenu *bmMenu = Application::bookmarkProvider()->bookmarkActionMenu();
    actionCollection()->addAction(QLatin1String("bookmarks"), bmMenu);

    setupSidePanel();
    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();

    // setup history menu: this has to be done AFTER setupGUI!!
    setupHistoryMenu();

    // bookmarks bar: this has to be done AFTER setupGUI!!
//     KToolBar *bmToolbar = toolBar("bookmarksToolBar");
//     m_bookmarkProvider->provideBmToolbar(bmToolbar);

    // setting up toolbars to NOT have context menu enabled
    setContextMenuPolicy(Qt::DefaultContextMenu);


}


MainWindow::~MainWindow()
{
    delete m_view;
}


QSize MainWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.8;
    return size;
}


void MainWindow::setupToolBars()
{
    KAction *a;

    // location bar
    a = new KAction(i18n("Location Bar"), this);
    a->setShortcut(KShortcut(Qt::Key_F6));
    a->setDefaultWidget(m_view->lineEditStack());
    actionCollection()->addAction(QLatin1String("url_bar"), a);

    // search bar
    a = new KAction(i18n("Search Bar"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_K));
    a->setDefaultWidget(m_searchBar);
    connect(m_searchBar, SIGNAL(search(const KUrl&)), this, SLOT(loadUrl(const KUrl&)));
    actionCollection()->addAction(QLatin1String("search_bar"), a);

    // bookmarks bar
    KAction *bookmarkBarAction = Application::bookmarkProvider()->bookmarkToolBarAction();
    a = actionCollection()->addAction(QLatin1String("bookmarks_bar"), bookmarkBarAction);
}


void MainWindow::setupActions()
{
    KAction *a;

    // Standard Actions
    KStandardAction::open(this, SLOT(slotFileOpen()), actionCollection());
    KStandardAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
    KStandardAction::printPreview(this, SLOT(slotFilePrintPreview()), actionCollection());
    KStandardAction::print(this, SLOT(slotFilePrint()), actionCollection());
    KStandardAction::quit(this , SLOT(close()), actionCollection());
    KStandardAction::find(this, SLOT(slotViewFindBar()) , actionCollection());
    KStandardAction::findNext(this, SLOT(slotFindNext()) , actionCollection());
    KStandardAction::findPrev(this, SLOT(slotFindPrevious()) , actionCollection());
    KStandardAction::fullScreen(this, SLOT(slotViewFullScreen(bool)), this, actionCollection());
    KStandardAction::home(this, SLOT(slotHome()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotPreferences()), actionCollection());
    KStandardAction::showMenubar(this, SLOT(slotShowMenubar(bool)), actionCollection());

    // WEB Actions (NO KStandardActions..)
    KStandardAction::redisplay(m_view, SLOT(slotWebReload()), actionCollection());
    KStandardAction::back(m_view, SLOT(slotWebBack()), actionCollection());
    KStandardAction::forward(m_view, SLOT(slotWebForward()), actionCollection());
    KStandardAction::undo(m_view, SLOT(slotWebUndo()), actionCollection());
    KStandardAction::redo(m_view, SLOT(slotWebRedo()), actionCollection());
    KStandardAction::cut(m_view, SLOT(slotWebCut()), actionCollection());
    KStandardAction::copy(m_view, SLOT(slotWebCopy()), actionCollection());
    KStandardAction::paste(m_view, SLOT(slotWebPaste()), actionCollection());

    a = new KAction(KIcon("process-stop"), i18n("&Stop"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_Period));
    actionCollection()->addAction(QLatin1String("stop"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(slotWebStop()));

    // stop reload Action
    m_stopReloadAction = new KAction(KIcon("view-refresh"), i18n("reload"), this);
    actionCollection()->addAction(QLatin1String("stop_reload") , m_stopReloadAction);
    m_stopReloadAction->setShortcutConfigurable(false);

    // ============== Custom Actions
    a = new KAction(KIcon("document-open-remote"), i18n("Open Location"), this);
    actionCollection()->addAction(QLatin1String("open_location"), a);
    connect(a, SIGNAL(triggered(bool)) , this, SLOT(slotOpenLocation()));

    a = new KAction(KIcon("zoom-in"), i18n("&Enlarge Font"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_Plus));
    actionCollection()->addAction(QLatin1String("bigger_font"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(slotViewTextBigger()));

    a = new KAction(KIcon("zoom-original"),  i18n("&Normal Font"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_0));
    actionCollection()->addAction(QLatin1String("normal_font"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(slotViewTextNormal()));

    a = new KAction(KIcon("zoom-out"),  i18n("&Shrink Font"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_Minus));
    actionCollection()->addAction(QLatin1String("smaller_font"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(slotViewTextSmaller()));

    a = new KAction(i18n("Page S&ource"), this);
    actionCollection()->addAction(QLatin1String("page_source"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(slotViewPageSource()));

    // ================ Tools (WebKit) Actions
    a = new KAction(KIcon("tools-report-bug"), i18n("Web &Inspector"), this);
    a->setCheckable(true);
    actionCollection()->addAction(QLatin1String("web_inspector"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(slotToggleInspector(bool)));

    a = new KAction(KIcon("view-media-artist"), i18n("Private &Browsing"), this);
    a->setCheckable(true);
    actionCollection()->addAction(QLatin1String("private_browsing"), a);
    connect(a, SIGNAL(triggered(bool)) , this, SLOT(slotPrivateBrowsing(bool)));

    // ================ history related actions
    m_historyBackAction = new KAction(KIcon("go-previous"), i18n("Back"), this);
    m_historyBackMenu = new KMenu(this);
    m_historyBackAction->setMenu(m_historyBackMenu);
    connect(m_historyBackAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenPrevious()));
    connect(m_historyBackMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowBackMenu()));
    connect(m_historyBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotOpenActionUrl(QAction *)));
    actionCollection()->addAction(QLatin1String("history_back"), m_historyBackAction);

    m_historyForwardAction = new KAction(KIcon("go-next"), i18n("Forward"), this);
    connect(m_historyForwardAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenNext()));
    actionCollection()->addAction(QLatin1String("history_forward"), m_historyForwardAction);

    // =================== Tab Actions
    a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    QList<QKeySequence> newTabShortcutList;
    newTabShortcutList << QKeySequence(QKeySequence::New);
    newTabShortcutList << QKeySequence(QKeySequence::AddTab);
    a->setShortcut(KShortcut(newTabShortcutList));
    actionCollection()->addAction(QLatin1String("new_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(newWebView()));

    a = new KAction(KIcon("tab-close"), i18n("&Close Tab"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_W));
    actionCollection()->addAction(QLatin1String("close_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(slotCloseTab()));

    a = new KAction(i18n("Show Next Tab"), this);
    a->setShortcuts(QApplication::isRightToLeft() ? KStandardShortcut::tabPrev() : KStandardShortcut::tabNext());
    actionCollection()->addAction(QLatin1String("show_next_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(nextTab()));

    a = new KAction(i18n("Show Previous Tab"), this);
    a->setShortcuts(QApplication::isRightToLeft() ? KStandardShortcut::tabNext() : KStandardShortcut::tabPrev());
    actionCollection()->addAction(QLatin1String("show_prev_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(previousTab()));

    // clear Location Bar action (Henry de Valance WISH)
    a = new KAction(KIcon("edit-clear-locationbar-rtl"), i18n("Clear Location Bar"), this);
    a->setShortcut(Qt::CTRL+Qt::Key_L);
    actionCollection()->addAction(QLatin1String("clear_location"),a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(slotClearLocationBar()));
    a->setWhatsThis(i18n( "<html>Clear Location bar<br /><br />"
                           "Clears the contents of the location bar.</html>" ));

}


void MainWindow::setupSidePanel()
{
    // Setup history side panel
    m_sidePanel = new SidePanel(i18n("History"), this);
    connect(m_sidePanel, SIGNAL(openUrl(const KUrl&)), this, SLOT(loadUrl(const KUrl&)));
    connect(m_sidePanel, SIGNAL(destroyed()), Application::instance(), SLOT(slotSaveConfiguration()));
    
    addDockWidget(Qt::LeftDockWidgetArea, m_sidePanel);
    
    // setup side panel actions
    KAction* a = new KAction(this);
    a->setText(i18n("History"));
    a->setCheckable(true);
    a->setChecked(ReKonfig::showSideBar());
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
    actionCollection()->addAction(QLatin1String("show_history_panel"), a);
    
    // connect to toogle action
    connect(a, SIGNAL(triggered(bool)), m_sidePanel->toggleViewAction(), SLOT(trigger()));
}


void MainWindow::setupHistoryMenu()
{
    HistoryMenu *historyMenu = new HistoryMenu(this);
    connect(historyMenu, SIGNAL(openUrl(const KUrl&)), m_view, SLOT(loadUrlInCurrentTab(const KUrl&)));
    connect(historyMenu, SIGNAL(hovered(const QString&)), this, SLOT(slotUpdateStatusbar(const QString&)));
    historyMenu->setTitle(i18n("&History"));

    // setting history menu position
    menuBar()->insertMenu(actionCollection()->action("bookmarks"), historyMenu);

    // setting initial actions
    QList<QAction*> historyActions;
    historyActions.append(actionCollection()->action("history_back"));
    historyActions.append(actionCollection()->action("history_forward"));
    historyActions.append(m_view->recentlyClosedTabsAction());
    historyMenu->setInitialActions(historyActions);
}


void MainWindow::slotUpdateConfiguration()
{
    // ============== General ==================
    m_homePage = ReKonfig::homePage();
    mainView()->showTabBar();

    // =========== Fonts ==============
    QWebSettings *defaultSettings = QWebSettings::globalSettings();

    int fnSize = ReKonfig::fontSize();

    QFont standardFont = ReKonfig::standardFont();
    defaultSettings->setFontFamily(QWebSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, fnSize);

    QFont fixedFont = ReKonfig::fixedFont();
    defaultSettings->setFontFamily(QWebSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFixedFontSize, fnSize);

    // ================ WebKit ============================
    defaultSettings->setAttribute(QWebSettings::AutoLoadImages, ReKonfig::autoLoadImages());
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, ReKonfig::javascriptEnabled());
    defaultSettings->setAttribute(QWebSettings::JavaEnabled, ReKonfig::javaEnabled());
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, ReKonfig::pluginsEnabled());
    defaultSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, ReKonfig::javascriptCanOpenWindows());
    defaultSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, ReKonfig::javascriptCanAccessClipboard());
    defaultSettings->setAttribute(QWebSettings::LinksIncludedInFocusChain, ReKonfig::linksIncludedInFocusChain());
    defaultSettings->setAttribute(QWebSettings::ZoomTextOnly, ReKonfig::zoomTextOnly());
    defaultSettings->setAttribute(QWebSettings::PrintElementBackgrounds, ReKonfig::printElementBackgrounds());
    defaultSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, ReKonfig::offlineStorageDatabaseEnabled());
    defaultSettings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, ReKonfig::offlineWebApplicationCacheEnabled());
    defaultSettings->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, ReKonfig::localStorageDatabaseEnabled());

    // ====== load Settings on main classes
    Application::networkAccessManager()->loadSettings();
    Application::cookieJar()->loadSettings();
    Application::historyManager()->loadSettings();
}


void MainWindow::slotUpdateBrowser()
{
    slotUpdateConfiguration();
    mainView()->slotReloadAllTabs();
}


KUrl MainWindow::guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema)
    {
        QUrl qurl(urlStr, QUrl::TolerantMode);
        KUrl url(qurl);
        if (url.isValid())
        {
            return url;
        }
    }

    // Might be a file.
    if (QFile::exists(urlStr))
    {
        QFileInfo info(urlStr);
        return KUrl::fromPath(info.absoluteFilePath());
    }

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema)
    {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if (dotIndex != -1)
        {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl qurl(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            KUrl url(qurl);
            if (url.isValid())
            {
                return url;
            }
        }
    }

    // Fall back to QUrl's own tolerant parser.
    QUrl qurl = QUrl(string, QUrl::TolerantMode);
    KUrl url(qurl);

    // finally for cases where the user just types in a hostname add http
    if (qurl.scheme().isEmpty())
    {
        qurl = QUrl(QLatin1String("http://") + string, QUrl::TolerantMode);
        url = KUrl(qurl);
    }
    return url;
}


void MainWindow::loadUrl(const KUrl &url)
{
    if (!currentTab() || url.isEmpty())
        return;

    m_view->currentLineEdit()->setText(url.prettyUrl());
    m_view->loadUrlInCurrentTab(url);
}


void MainWindow::slotOpenLocation()
{
    m_view->currentLineEdit()->selectAll();
    m_view->currentLineEdit()->setFocus();
}


void MainWindow::slotFileSaveAs()
{
    KUrl srcUrl = currentTab()->url();
    Application::downloadManager()->newDownload(srcUrl);
}


void MainWindow::slotPreferences()
{
    // an instance the dialog could be already created and could be cached,
    // in which case you want to display the cached dialog
    if (SettingsDialog::showDialog("rekonfig"))
        return;

    // we didn't find an instance of this dialog, so lets create it
    SettingsDialog *s = new SettingsDialog(this);

    // keep us informed when the user changes settings
    connect(s, SIGNAL(settingsChanged(const QString&)), this, SLOT(slotUpdateBrowser()));

    s->exec();
}


void MainWindow::slotUpdateStatusbar(const QString &string)
{
    statusBar()->showMessage(string, 2000);
}


void MainWindow::slotUpdateActions()
{
    m_historyBackAction->setEnabled(currentTab()->history()->canGoBack());
    m_historyForwardAction->setEnabled(currentTab()->history()->canGoForward());
}


void MainWindow::slotUpdateWindowTitle(const QString &title)
{
    if (title.isEmpty())
    {
        setWindowTitle("rekonq");
    }
    else
    {
        setWindowTitle(title + " - rekonq");
    }
}


void MainWindow::slotFileOpen()
{
    QString filePath = KFileDialog::getOpenFileName(KUrl(),
                       i18n("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"),
                       this,
                       i18n("Open Web Resource")
                                                   );

    if (filePath.isEmpty())
        return;

    loadUrl(guessUrlFromString(filePath));
}


// TODO: Port to KDE
void MainWindow::slotFilePrintPreview()
{
    if (!currentTab())
        return;
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
    connect(dialog, SIGNAL(paintRequested(QPrinter *)), currentTab(), SLOT(print(QPrinter *)));
    dialog->exec();
}


void MainWindow::slotFilePrint()
{
    if (!currentTab())
        return;
    printRequested(currentTab()->page()->mainFrame());
}


// TODO: Port to KDE
void MainWindow::printRequested(QWebFrame *frame)
{
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(i18n("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;
    frame->print(&printer);
}


void MainWindow::slotPrivateBrowsing(bool enable)
{
    QWebSettings *settings = QWebSettings::globalSettings();
    if (enable)
    {
        QString title = i18n("Are you sure you want to turn on private browsing?");
        QString text = "<b>" + title + i18n("</b><br><br>When private browsing in turned on,"
                                            " webpages are not added to the history,"
                                            " new cookies are not stored, current cookies can't be accessed," \
                                            " site icons wont be stored, session wont be saved, " \
                                            " and searches are not addded to the pop-up menu in the Google search box." \
                                            "  Until you close the window, you can still click the Back and Forward buttons" \
                                            " to return to the webpages you have opened.");

        int  button = KMessageBox::questionYesNo(this, text, title);
        if (button == KMessageBox::Ok)
        {
            settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        }
        else
        {
            actionCollection()->action("private_browsing")->setChecked(false);
        }
    }
    else
    {
        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, false);

        MainWindow* win = Application::instance()->mainWindow();
        win->m_lastSearch = QString::null;
        win->mainView()->clear();
    }
}

void MainWindow::slotFind(const QString & search)
{
    if (!currentTab())
        return;
    m_lastSearch = search;
    slotFindNext();
}


void MainWindow::slotViewFindBar()
{
    m_findBar->showFindBar();
}


void MainWindow::slotFindNext()
{
    if (!currentTab() && m_lastSearch.isEmpty())
        return;

    QWebPage::FindFlags options;
    if(m_findBar->matchCase())
    {
        options = QWebPage::FindCaseSensitively | QWebPage::FindWrapsAroundDocument;
    }
    else
    {
        options = QWebPage::FindWrapsAroundDocument;
    }

    if (!currentTab()->findText(m_lastSearch, options))
    {
        slotUpdateStatusbar(QString(m_lastSearch) + i18n(" not found."));
    }
}


void MainWindow::slotFindPrevious()
{
    if (!currentTab() && m_lastSearch.isEmpty())
        return;

    QWebPage::FindFlags options;
    if(m_findBar->matchCase())
    {
        options = QWebPage::FindCaseSensitively | QWebPage::FindBackward | QWebPage::FindWrapsAroundDocument;
    }
    else
    {
        options = QWebPage::FindBackward | QWebPage::FindWrapsAroundDocument;
    }

    if (!currentTab()->findText(m_lastSearch, options))
    {
        slotUpdateStatusbar(QString(m_lastSearch) + i18n(" not found."));
    }
}


void MainWindow::slotViewTextBigger()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() + 0.1);
}


void MainWindow::slotViewTextNormal()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(1.0);
}


void MainWindow::slotViewTextSmaller()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() - 0.1);
}


void MainWindow::slotViewFullScreen(bool makeFullScreen)
{
    if (makeFullScreen == true)
    {
        menuBar()->hide();
        toolBar("mainToolBar")->hide();
        toolBar("locationToolBar")->hide();
    }
    else
    {
        menuBar()->show();
        toolBar("mainToolBar")->show();
        toolBar("locationToolBar")->show();
    }
    KToggleFullScreenAction::setFullScreen(this, makeFullScreen);
}


void MainWindow::slotViewPageSource()
{
    if (!currentTab())
        return;

    KUrl url(currentTab()->url());
    bool isTempFile = false;
    if (!url.isLocalFile())
    {
        KTemporaryFile sourceFile;

        /// TODO: autochoose tempfile suffix
        sourceFile.setSuffix(QString(".html"));
        sourceFile.setAutoRemove(false);
        
        if (sourceFile.open())
        {
            QDataStream stream(&sourceFile);
            stream << currentTab()->page()->mainFrame()->toHtml().toUtf8();
            
            url = KUrl();
            url.setPath(sourceFile.fileName());
            isTempFile = true;
        }
    }
    KRun::runUrl(url, QLatin1String("text/plain"), this, isTempFile);
}


void MainWindow::slotHome()
{
    loadUrl(KUrl(m_homePage));
}


void MainWindow::slotToggleInspector(bool enable)
{
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, enable);
    if (enable)
    {
        int result = KMessageBox::questionYesNo(this,
                        i18n("The web inspector will only work correctly for pages that were loaded after enabling.\n"
                                "Do you want to reload all pages?"),
                        i18n("Web Inspector")
                     );

        if (result == KMessageBox::Yes)
        {
            m_view->slotReloadAllTabs();
        }
    }
}


MainView *MainWindow::mainView() const
{
    return m_view;
}



WebView *MainWindow::currentTab() const
{
    return m_view->currentWebView();
}


void MainWindow::slotLoadProgress(int progress)
{
    QAction *stop = actionCollection()->action("stop");
    QAction *reload = actionCollection()->action("view_redisplay");
    if (progress < 100 && progress > 0)
    {
        disconnect(m_stopReloadAction, SIGNAL(triggered(bool)), reload , SIGNAL(triggered(bool)));
        m_stopReloadAction->setIcon(KIcon("process-stop"));
        m_stopReloadAction->setToolTip(i18n("Stop loading the current page"));
        m_stopReloadAction->setText(i18n("Stop"));
        connect(m_stopReloadAction, SIGNAL(triggered(bool)), stop, SIGNAL(triggered(bool)));
    }
    else
    {
        disconnect(m_stopReloadAction, SIGNAL(triggered(bool)), stop , SIGNAL(triggered(bool)));
        m_stopReloadAction->setIcon(KIcon("view-refresh"));
        m_stopReloadAction->setToolTip(i18n("Reload the current page"));
        m_stopReloadAction->setText(i18n("Reload"));
        connect(m_stopReloadAction, SIGNAL(triggered(bool)), reload, SIGNAL(triggered(bool)));

    }
}


void MainWindow::slotAboutToShowBackMenu()
{
    m_historyBackMenu->clear();
    if (!currentTab())
        return;
    QWebHistory *history = currentTab()->history();
    int historyCount = history->count();
    for (int i = history->backItems(historyCount).count() - 1; i >= 0; --i)
    {
        QWebHistoryItem item = history->backItems(history->count()).at(i);
        KAction *action = new KAction(this);
        action->setData(-1*(historyCount - i - 1));
        QIcon icon = Application::instance()->icon(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyBackMenu->addAction(action);
    }
}


void MainWindow::slotOpenActionUrl(QAction *action)
{
    int offset = action->data().toInt();
    QWebHistory *history = currentTab()->history();
    if (offset < 0)
    {
        history->goToItem(history->backItems(-1*offset).first()); // back
    }
    else
    {
        if (offset > 0)
        {
            history->goToItem(history->forwardItems(history->count() - offset + 1).back()); // forward
        }
    }
}


void MainWindow::slotOpenPrevious()
{
    QWebHistory *history = currentTab()->history();
    if (history->canGoBack())
        history->goToItem(history->backItem());
}


void MainWindow::slotOpenNext()
{
    QWebHistory *history = currentTab()->history();
    if (history->canGoForward())
        history->goToItem(history->forwardItem());
}


void MainWindow::geometryChangeRequested(const QRect &geometry)
{
    setGeometry(geometry);
}


void MainWindow::slotShowMenubar(bool enable)
{
    if(enable)
        menuBar()->show();
    else
        menuBar()->hide();
}


bool MainWindow::queryClose()
{
    if (m_view->count() > 1)
    {

        int answer = KMessageBox::questionYesNoCancel( 
                        this,
                        i18n("Are you sure you want to close the window?\n" "You have %1 tab(s) open" , m_view->count()),
                        i18n("Are you sure you want to close the window?"),
                        KStandardGuiItem::quit(),
                        KGuiItem(i18n("C&lose Current Tab"), KIcon("tab-close")),
                        KStandardGuiItem::cancel(),
                        "confirmClosingMultipleTabs"
                     );

        switch (answer) 
        {
            case KMessageBox::Yes:
                // Quit
                return true;
                break;
            case KMessageBox::No:
                // Close only the current tab
                m_view->slotCloseTab();
            default:
                return false;
        }
    }

    return true;
}


void MainWindow::slotClearLocationBar()
{
    QLineEdit *lineEdit = m_view->currentLineEdit();
    lineEdit->clear();
    lineEdit->setFocus();
}


QAction *MainWindow::actionByName(const QString name)
{
    QAction *ret = actionCollection()->action(name);

    if (ret)
        return ret;

    /* else */
    kWarning() << "Action named: " << name << " not found, returning empty action.";

    return new QAction(this);  // return empty object instead of NULL pointer
}

