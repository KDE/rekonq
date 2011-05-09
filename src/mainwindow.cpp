/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2011 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2011 by Lionel Chauvin <megabigbug@yahoo.fr>
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


// Self Includes
#include "mainwindow.h"
#include "mainwindow.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "analyzerpanel.h"
#include "application.h"
#include "bookmarkprovider.h"
#include "bookmarkspanel.h"
#include "bookmarkstoolbar.h"
#include "findbar.h"
#include "historypanel.h"
#include "iconmanager.h"
#include "mainview.h"
#include "settingsdialog.h"
#include "stackedurlbar.h"
#include "tabbar.h"
#include "urlbar.h"
#include "webinspectorpanel.h"
#include "webpage.h"
#include "webtab.h"
#include "zoombar.h"
#include "useragentinfo.h"
#include "useragentwidget.h"

// Ui Includes
#include "ui_cleardata.h"

// KDE Includes
#include <KIO/Job>

#include <KAction>
#include <KEditToolBar>
#include <KFileDialog>
#include <KJobUiDelegate>
#include <KMenu>
#include <KMenuBar>
#include <KMessageBox>
#include <KPassivePopup>
#include <KProcess>
#include <KPushButton>
#include <KStandardDirs>
#include <KToggleFullScreenAction>
#include <KProtocolManager>

#include <KParts/Part>
#include <KParts/BrowserExtension>

// Qt Includes
#include <QtCore/QTimer>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <QtGui/QDesktopWidget>
#include <QtGui/QLabel>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QPrintPreviewDialog>
#include <QtGui/QVBoxLayout>

#include <QtWebKit/QWebHistory>

#include <QSignalMapper>


MainWindow::MainWindow()
        : KXmlGuiWindow()
        , m_view(new MainView(this))
        , m_findBar(new FindBar(this))
        , m_zoomBar(new ZoomBar(this))
        , m_historyPanel(0)
        , m_bookmarksPanel(0)
        , m_webInspectorPanel(0)
        , m_analyzerPanel(0)
        , m_historyBackMenu(0)
        , m_historyForwardMenu(0)
        , m_userAgentMenu(new KMenu(this))
        , m_bookmarksBar(0)
        , m_popup(new KPassivePopup(this))
        , m_hidePopupTimer(new QTimer(this))
        , m_toolsMenu(0)
        , m_developerMenu(0)
{
    // creating a centralWidget containing panel, m_view and the hidden findbar
    QWidget *centralWidget = new QWidget;
    centralWidget->setContentsMargins(0, 0, 0, 0);

    // setting layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
    layout->addWidget(m_findBar);
    layout->addWidget(m_zoomBar);
    centralWidget->setLayout(layout);

    // central widget
    setCentralWidget(centralWidget);

    // setting size policies
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // then, setup our actions
    setupActions();

    // setting Panels
    setupPanels();

    // setting up rekonq tools
    setupTools();

    // setting up rekonq toolbar(s)
    setupToolbars();

    // disable help menu, as we'll load it on our own...
    setHelpMenuEnabled(false);
        
    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();

    // no menu bar in rekonq: we have other plans..
    menuBar()->setVisible(false);

    // no more status bar..
    setStatusBar(0);

    // give me some time to do all the other stuffs...
    QTimer::singleShot(100, this, SLOT(postLaunch()));
}


MainWindow::~MainWindow()
{
    m_hidePopupTimer->stop();
    
    rApp->bookmarkProvider()->removeBookmarkBar(m_bookmarksBar);
    rApp->bookmarkProvider()->removeBookmarkPanel(m_bookmarksPanel);
    rApp->removeMainWindow(this);
}


void MainWindow::setupToolbars()
{
    KAction *a;

    // location bar
    a = new KAction(i18n("Location Bar"), this);
    a->setDefaultWidget(m_view->widgetBar());
    actionCollection()->addAction(QL1S("url_bar"), a);

    KToolBar *mainBar = toolBar("mainToolBar");

    mainBar->show();  // this just to fix reopening rekonq after fullscreen close
}


void MainWindow::initBookmarkBar()
{
    KToolBar *XMLGUIBkBar = toolBar("bookmarkToolBar");
    if (!XMLGUIBkBar)
        return;

    if (m_bookmarksBar)
    {
        rApp->bookmarkProvider()->removeBookmarkBar(m_bookmarksBar);
        delete m_bookmarksBar;
    }
    m_bookmarksBar = new BookmarkToolBar(XMLGUIBkBar, this);
    rApp->bookmarkProvider()->registerBookmarkBar(m_bookmarksBar);
}


void MainWindow::configureToolbars()
{
    if (autoSaveSettings())
        saveAutoSaveSettings();

    KEditToolBar dlg(factory(), this);
    // The bookmark bar needs to be refill after the UI changes are finished
    connect(&dlg, SIGNAL(newToolBarConfig()), this, SLOT(initBookmarkBar()));
    dlg.exec();
}


void MainWindow::updateToolsMenu()
{
    if (m_toolsMenu->isEmpty())
    {
        m_toolsMenu->addAction(actionByName(KStandardAction::name(KStandardAction::Open)));
        m_toolsMenu->addAction(actionByName(KStandardAction::name(KStandardAction::SaveAs)));
        m_toolsMenu->addAction(actionByName(KStandardAction::name(KStandardAction::Print)));
        m_toolsMenu->addAction(actionByName(KStandardAction::name(KStandardAction::Find)));

        QAction *action = actionByName(KStandardAction::name(KStandardAction::Zoom));
        action->setCheckable(true);
        connect(m_zoomBar, SIGNAL(visibilityChanged(bool)), action, SLOT(setChecked(bool)));
        m_toolsMenu->addAction(action);

        m_toolsMenu->addAction(actionByName(QL1S("useragent")));

        m_toolsMenu->addSeparator();

        m_toolsMenu->addAction(actionByName(QL1S("private_browsing")));
        m_toolsMenu->addAction(actionByName(QL1S("clear_private_data")));

        m_toolsMenu->addSeparator();

        m_developerMenu = new KActionMenu(KIcon("applications-development-web"), i18n("Development"), this);
        m_developerMenu->addAction(actionByName(QL1S("web_inspector")));
        m_developerMenu->addAction(actionByName(QL1S("page_source")));
        m_developerMenu->addAction(actionByName(QL1S("net_analyzer")));

        m_toolsMenu->addAction(m_developerMenu);
        if (!ReKonfig::showDeveloperTools())
            m_developerMenu->setVisible(false);

        m_toolsMenu->addSeparator();

        action = m_bookmarksBar->toolBar()->toggleViewAction();
        action->setText(i18n("Bookmarks Toolbar"));
        action->setIcon(KIcon("bookmarks-bar"));
        m_toolsMenu->addAction(action);

        m_toolsMenu->addAction(actionByName(QL1S("show_history_panel")));
        m_toolsMenu->addAction(actionByName(QL1S("show_bookmarks_panel")));
        m_toolsMenu->addAction(actionByName(KStandardAction::name(KStandardAction::FullScreen)));

        m_toolsMenu->addSeparator();

        helpMenu()->setIcon(KIcon("help-browser"));
        m_toolsMenu->addAction(helpMenu()->menuAction());
        m_toolsMenu->addAction(actionByName(KStandardAction::name(KStandardAction::Preferences)));
    }

    m_developerMenu->setVisible(ReKonfig::showDeveloperTools());
}


void MainWindow::postLaunch()
{
    setupBookmarksAndToolsShortcuts();

    // setting popup notification
    m_popup->setAutoDelete(false);
    connect(rApp, SIGNAL(focusChanged(QWidget*, QWidget*)), m_popup, SLOT(hide()));
    m_popup->setFrameShape(QFrame::NoFrame);
    m_popup->setLineWidth(0);
    connect(m_hidePopupTimer, SIGNAL(timeout()), m_popup, SLOT(hide()));

    // notification system
    connect(m_view, SIGNAL(showStatusBarMessage(const QString&, Rekonq::Notify)), this, SLOT(notifyMessage(const QString&, Rekonq::Notify)));
    connect(m_view, SIGNAL(linkHovered(const QString&)), this, SLOT(notifyMessage(const QString&)));

    // --------- connect signals and slots
    connect(m_view, SIGNAL(currentTitle(const QString &)), this, SLOT(updateWindowTitle(const QString &)));
    connect(m_view, SIGNAL(printRequested(QWebFrame *)), this, SLOT(printRequested(QWebFrame *)));

    // (shift +) ctrl + tab switching
    connect(this, SIGNAL(ctrlTabPressed()), m_view, SLOT(nextTab()));
    connect(this, SIGNAL(shiftCtrlTabPressed()), m_view, SLOT(previousTab()));

    // wheel history navigation
    connect(m_view, SIGNAL(openPreviousInHistory()), this, SLOT(openPrevious()));
    connect(m_view, SIGNAL(openNextInHistory()), this, SLOT(openNext()));

    // update toolbar actions signals
    connect(m_view, SIGNAL(currentChanged(int)), this, SLOT(updateActions()));

    //Change window icon according to tab icon
    connect(m_view, SIGNAL(currentChanged(int)), this, SLOT(changeWindowIcon(int)));

    // launch it manually. Just the first time...
    updateActions();

    // Find Bar signal
    connect(m_findBar, SIGNAL(searchString(const QString &)), this, SLOT(find(const QString &)));

    // Zoom Bar signal
    connect(m_view, SIGNAL(currentChanged(int)), m_zoomBar, SLOT(updateSlider(int)));
    // Ctrl + wheel handling
    connect(this->currentTab()->view(), SIGNAL(zoomChanged(int)), m_zoomBar, SLOT(setValue(int)));

    // setting up toolbars to NOT have context menu enabled
    setContextMenuPolicy(Qt::DefaultContextMenu);

    // accept d'n'd
    setAcceptDrops(true);

    // Bookmark ToolBar (needs to be setup after the call to setupGUI())
    initBookmarkBar();
}


QSize MainWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.8;
    return size;
}

void MainWindow::changeWindowIcon(int index)
{
    if (ReKonfig::useFavicon())
    {
        KUrl url = mainView()->webTab(index)->url();
        QIcon icon = rApp->iconManager()->iconForUrl(url).pixmap(QSize(32, 32));
        setWindowIcon(icon);
    }
}

void MainWindow::setupActions()
{
    // this let shortcuts work..
    actionCollection()->addAssociatedWidget(this);

    KAction *a;

    // new window action
    a = new KAction(KIcon("window-new"), i18n("&New Window"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_N));
    actionCollection()->addAction(QL1S("new_window"), a);
    connect(a, SIGNAL(triggered(bool)), rApp, SLOT(newWindow()));

    // Standard Actions
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    KStandardAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStandardAction::print(this, SLOT(printRequested()), actionCollection());
    KStandardAction::quit(rApp, SLOT(quit()), actionCollection());

    a = KStandardAction::find(m_findBar, SLOT(show()), actionCollection());
    KShortcut findShortcut = KStandardShortcut::find();
    findShortcut.setAlternate(Qt::Key_Slash);
    a->setShortcut(findShortcut);

    KStandardAction::findNext(this, SLOT(findNext()) , actionCollection());
    KStandardAction::findPrev(this, SLOT(findPrevious()) , actionCollection());

    a = KStandardAction::fullScreen(this, SLOT(viewFullScreen(bool)), this, actionCollection());
    KShortcut fullScreenShortcut = KStandardShortcut::fullScreen();
    fullScreenShortcut.setAlternate(Qt::Key_F11);
    a->setShortcut(fullScreenShortcut);

    a = actionCollection()->addAction(KStandardAction::Home);
    connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(homePage(Qt::MouseButtons, Qt::KeyboardModifiers)));
    KStandardAction::preferences(this, SLOT(preferences()), actionCollection());

    a = KStandardAction::redisplay(m_view, SLOT(webReload()), actionCollection());
    a->setText(i18n("Reload"));
    KShortcut reloadShortcut = KStandardShortcut::reload();
    reloadShortcut.setAlternate(Qt::CTRL + Qt::Key_R);
    a->setShortcut(reloadShortcut);

    a = new KAction(KIcon("process-stop"), i18n("&Stop"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_Period));
    actionCollection()->addAction(QL1S("stop"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(webStop()));

    // stop reload Action
    m_stopReloadAction = new KAction(this);
    actionCollection()->addAction(QL1S("stop_reload") , m_stopReloadAction);
    m_stopReloadAction->setShortcutConfigurable(false);
    connect(m_view, SIGNAL(browserTabLoading(bool)), this, SLOT(browserLoading(bool)));

    a = new KAction(i18n("Open Location"), this);
    KShortcut openLocationShortcut(Qt::CTRL + Qt::Key_L);
    openLocationShortcut.setAlternate(Qt::Key_F6);
    a->setShortcut(openLocationShortcut);
    actionCollection()->addAction(QL1S("open_location"), a);
    connect(a, SIGNAL(triggered(bool)) , this, SLOT(openLocation()));

    // set zoom bar actions
    m_zoomBar->setupActions(this);

    // tab list
    m_tabListMenu = new KMenu();
    m_tabListMenu->addAction("hack"); // necessary to show the menu on the right side the first time
    connect(m_tabListMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowTabListMenu()));
    connect(m_tabListMenu, SIGNAL(triggered(QAction*)), this, SLOT(openActionTab(QAction*)));
    KActionMenu *tabAction = new KActionMenu(i18n("Tab List"), this);
    tabAction->setMenu(m_tabListMenu);
    tabAction->setIcon(KIcon("document-multiple"));
    tabAction->setDelayed(false);
    actionCollection()->addAction(QL1S("tab_list"), tabAction);

    // =============================== Tools Actions =================================
    a = new KAction(i18n("View Page S&ource"), this);
    a->setIcon(KIcon("application-xhtml+xml"));
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
    actionCollection()->addAction(QL1S("page_source"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(viewPageSource()));

    a = rApp->privateBrowsingAction();
    a->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_P);
    actionCollection()->addAction(QL1S("private_browsing"), a);

    a = new KAction(KIcon("edit-clear"), i18n("Clear Private Data..."), this);
    a->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Delete);
    actionCollection()->addAction(QL1S("clear_private_data"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(clearPrivateData()));

    // ========================= History related actions ==============================
    a = actionCollection()->addAction(KStandardAction::Back);
    connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(openPrevious(Qt::MouseButtons, Qt::KeyboardModifiers)));

    m_historyBackMenu = new KMenu(this);
    a->setMenu(m_historyBackMenu);
    connect(m_historyBackMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowBackMenu()));
    connect(m_historyBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(openActionUrl(QAction *)));

    a = actionCollection()->addAction(KStandardAction::Forward);
    connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)), this, SLOT(openNext(Qt::MouseButtons, Qt::KeyboardModifiers)));

    m_historyForwardMenu = new KMenu(this);
    a->setMenu(m_historyForwardMenu);
    connect(m_historyForwardMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowForwardMenu()));
    connect(m_historyForwardMenu, SIGNAL(triggered(QAction *)), this, SLOT(openActionUrl(QAction*)));

    // ============================== General Tab Actions ====================================
    a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_T));
    actionCollection()->addAction(QL1S("new_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(newTab()));

    a = new KAction(KIcon("view-refresh"), i18n("Reload All Tabs"), this);
    actionCollection()->addAction(QL1S("reload_all_tabs"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(reloadAllTabs()));

    a = new KAction(i18n("Show Next Tab"), this);
    a->setShortcuts(QApplication::isRightToLeft() ? KStandardShortcut::tabPrev() : KStandardShortcut::tabNext());
    actionCollection()->addAction(QL1S("show_next_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(nextTab()));

    a = new KAction(i18n("Show Previous Tab"), this);
    a->setShortcuts(QApplication::isRightToLeft() ? KStandardShortcut::tabNext() : KStandardShortcut::tabPrev());
    actionCollection()->addAction(QL1S("show_prev_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(previousTab()));

    a = new KAction(KIcon("tab-new"), i18n("Open Last Closed Tab"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_T));
    actionCollection()->addAction(QL1S("open_last_closed_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(openLastClosedTab()));

    // Closed Tabs Menu
    KActionMenu *closedTabsMenu = new KActionMenu(KIcon("tab-new"), i18n("Closed Tabs"), this);
    closedTabsMenu->setDelayed(false);
    actionCollection()->addAction(QL1S("closed_tab_menu"), closedTabsMenu);

    QSignalMapper *tabSignalMapper = new QSignalMapper(this);
    // shortcuts for quickly switching to a tab
    for (int i = 1; i <= 9; i++)
    {
        a = new KAction(i18n("Switch to Tab %1", i), this);
        a->setShortcut(KShortcut(QString("Alt+%1").arg(i)));
        actionCollection()->addAction(QL1S(("switch_tab_" + QString::number(i)).toAscii()), a);
        connect(a, SIGNAL(triggered(bool)), tabSignalMapper, SLOT(map()));
        tabSignalMapper->setMapping(a, i);
    }
    connect(tabSignalMapper, SIGNAL(mapped(const int)), m_view, SLOT(switchToTab(const int)));

    // shortcuts for loading favorite pages
    QSignalMapper *favoritesSignalMapper = new QSignalMapper(this);
    for (int i = 1; i <= 9; ++i)
    {
        a = new KAction(i18n("Switch to Favorite Page %1", i), this);
        a->setShortcut(KShortcut(QString("Ctrl+%1").arg(i)));
        actionCollection()->addAction(QL1S(("switch_favorite_" + QString::number(i)).toAscii()), a);
        connect(a, SIGNAL(triggered(bool)), favoritesSignalMapper, SLOT(map()));
        favoritesSignalMapper->setMapping(a, i);
    }
    connect(favoritesSignalMapper, SIGNAL(mapped(const int)), m_view, SLOT(loadFavorite(const int)));

    // ============================== Indexed Tab Actions ====================================
    a = new KAction(KIcon("tab-close"), i18n("&Close Tab"), this);
    a->setShortcuts(KStandardShortcut::close());
    actionCollection()->addAction(QL1S("close_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view->tabBar(), SLOT(closeTab()));

    a = new KAction(KIcon("tab-duplicate"), i18n("Clone Tab"), this);
    actionCollection()->addAction(QL1S("clone_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view->tabBar(), SLOT(cloneTab()));

    a = new KAction(KIcon("tab-close-other"), i18n("Close &Other Tabs"), this);
    actionCollection()->addAction(QL1S("close_other_tabs"), a);
    connect(a, SIGNAL(triggered(bool)), m_view->tabBar(), SLOT(closeOtherTabs()));

    a = new KAction(KIcon("view-refresh"), i18n("Reload Tab"), this);
    actionCollection()->addAction(QL1S("reload_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view->tabBar(), SLOT(reloadTab()));

    a = new KAction(KIcon("tab-detach"), i18n("Detach Tab"), this);
    actionCollection()->addAction(QL1S("detach_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view->tabBar(), SLOT(detachTab()));

    // Bookmark Menu
    KActionMenu *bmMenu = rApp->bookmarkProvider()->bookmarkActionMenu(this);
    bmMenu->setIcon(KIcon("bookmarks"));
    bmMenu->setDelayed(false);
    bmMenu->setShortcutConfigurable(true);
    bmMenu->setShortcut(KShortcut(Qt::ALT + Qt::Key_B));
    actionCollection()->addAction(QL1S("bookmarksActionMenu"), bmMenu);

    // --- User Agent
    a = new KAction(KIcon("preferences-web-browser-identification"), i18n("Browser Identification"), this);
    actionCollection()->addAction(QL1S("useragent"), a);
    a->setMenu(m_userAgentMenu);
    connect(m_userAgentMenu, SIGNAL(aboutToShow()), this, SLOT(populateUserAgentMenu()));

    a = new KAction(KIcon("preferences-web-browser-identification"), i18n("Browser Identification"), this);
    actionCollection()->addAction(QL1S("UserAgentSettings"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(showUserAgentSettings()));
}


void MainWindow::setupTools()
{
    KActionMenu *toolsAction = new KActionMenu(KIcon("configure"), i18n("&Tools"), this);
    toolsAction->setDelayed(false);
    toolsAction->setShortcutConfigurable(true);
    toolsAction->setShortcut(KShortcut(Qt::ALT + Qt::Key_T));
    m_toolsMenu = new KMenu(this);
    toolsAction->setMenu(m_toolsMenu); // dummy menu to have the dropdown arrow
    connect(m_toolsMenu, SIGNAL(aboutToShow()), this, SLOT(updateToolsMenu()));

    // adding rekonq_tools to rekonq actionCollection
    actionCollection()->addAction(QL1S("rekonq_tools"), toolsAction);

    // Actions are added after the call to setupGUI() to ensure the help menu works
}


void MainWindow::setupPanels()
{
    KAction* a;

    // STEP 1
    // Setup history panel
    m_historyPanel = new HistoryPanel(i18n("History Panel"), this);
    connect(m_historyPanel, SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType &)), rApp, SLOT(loadUrl(const KUrl&, const Rekonq::OpenType &)));
    connect(m_historyPanel, SIGNAL(itemHovered(QString)), this, SLOT(notifyMessage(QString)));
    connect(m_historyPanel, SIGNAL(destroyed()), rApp, SLOT(saveConfiguration()));

    addDockWidget(Qt::LeftDockWidgetArea, m_historyPanel);

    // setup history panel action
    a = (KAction *) m_historyPanel->toggleViewAction();
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
    a->setIcon(KIcon("view-history"));
    actionCollection()->addAction(QL1S("show_history_panel"), a);

    // STEP 2
    // Setup bookmarks panel
    m_bookmarksPanel = new BookmarksPanel(i18n("Bookmarks Panel"), this);
    connect(m_bookmarksPanel, SIGNAL(openUrl(const KUrl&, const Rekonq::OpenType &)), rApp, SLOT(loadUrl(const KUrl&, const Rekonq::OpenType &)));
    connect(m_bookmarksPanel, SIGNAL(itemHovered(QString)), this, SLOT(notifyMessage(QString)));
    connect(m_bookmarksPanel, SIGNAL(destroyed()), rApp, SLOT(saveConfiguration()));

    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarksPanel);

    rApp->bookmarkProvider()->registerBookmarkPanel(m_bookmarksPanel);

    // setup bookmarks panel action
    a = (KAction *) m_bookmarksPanel->toggleViewAction();
    a->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_B));
    a->setIcon(KIcon("bookmarks-organize"));
    actionCollection()->addAction(QL1S("show_bookmarks_panel"), a);

    // STEP 3
    // Setup webinspector panel
    m_webInspectorPanel = new WebInspectorPanel(i18n("Web Inspector"), this);
    connect(mainView(), SIGNAL(currentChanged(int)), m_webInspectorPanel, SLOT(changeCurrentPage()));

    a = new KAction(KIcon("tools-report-bug"), i18n("Web &Inspector"), this);
    a->setCheckable(true);
    actionCollection()->addAction(QL1S("web_inspector"), a);
    connect(a, SIGNAL(triggered(bool)), m_webInspectorPanel, SLOT(toggle(bool)));

    addDockWidget(Qt::BottomDockWidgetArea, m_webInspectorPanel);
    m_webInspectorPanel->hide();

    // STEP 4
    // Setup Network analyzer panel
    m_analyzerPanel = new NetworkAnalyzerPanel(i18n("Network Analyzer"), this);
    connect(mainView(), SIGNAL(currentChanged(int)), m_analyzerPanel, SLOT(changeCurrentPage()));

    a = new KAction(KIcon("document-edit-decrypt-verify"), i18n("Network Analyzer"), this);
    a->setCheckable(true);
    actionCollection()->addAction(QL1S("net_analyzer"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(enableNetworkAnalysis(bool)));

    addDockWidget(Qt::BottomDockWidgetArea, m_analyzerPanel);
    m_analyzerPanel->hide();
}

void MainWindow::openLocation()
{
    if (isFullScreen())
    {
        setWidgetsVisible(true);
    }
    m_view->currentUrlBar()->selectAll();
    m_view->currentUrlBar()->setFocus();
}


void MainWindow::fileSaveAs()
{
    WebTab *w = currentTab();
    KUrl srcUrl = w->url();

    // First, try with suggested file name...
    QString name = w->page()->suggestedFileName();

    // Second, with KUrl fileName...
    if (name.isEmpty())
    {
        name = srcUrl.fileName();
    }

    // Last chance...
    if (name.isEmpty())
    {
        name = srcUrl.host() + QString(".html");
    }

    const QString destUrl = KFileDialog::getSaveFileName(name, QString(), this);
    if (destUrl.isEmpty())
        return;

    KIO::Job *job = KIO::file_copy(srcUrl, KUrl(destUrl), -1, KIO::Overwrite);
    job->addMetaData("MaxCacheSize", "0");  // Don't store in http cache.
    job->addMetaData("cache", "cache");     // Use entry from cache if available.
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
}


void MainWindow::preferences()
{
    // an instance the dialog could be already created and could be cached,
    // in which case you want to display the cached dialog
    if (SettingsDialog::showDialog("rekonfig"))
        return;

    // we didn't find an instance of this dialog, so lets create it
    QPointer<SettingsDialog> s = new SettingsDialog(this);

    // keep us informed when the user changes settings
    connect(s, SIGNAL(settingsChanged(const QString&)), rApp, SLOT(updateConfiguration()));

    s->exec();
    delete s;
}


void MainWindow::updateActions()
{
    kDebug() << "updating actions..";
    bool rekonqPage = currentTab()->page()->isOnRekonqPage();

    QAction *historyBackAction = actionByName(KStandardAction::name(KStandardAction::Back));
    if (rekonqPage && currentTab()->view()->history()->count() > 0)
        historyBackAction->setEnabled(true);
    else
        historyBackAction->setEnabled(currentTab()->view()->history()->canGoBack());

    QAction *historyForwardAction = actionByName(KStandardAction::name(KStandardAction::Forward));
    historyForwardAction->setEnabled(currentTab()->view()->history()->canGoForward());
}


void MainWindow::updateWindowTitle(const QString &title)
{
    QWebSettings *settings = QWebSettings::globalSettings();
    if (title.isEmpty())
    {
        if (settings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        {
            setWindowTitle(i18nc("Window title when private browsing is activated", "rekonq (Private Browsing)"));
        }
        else
        {
            setWindowTitle("rekonq");
        }
    }
    else
    {
        if (settings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        {
            setWindowTitle(i18nc("window title, %1 = title of the active website", "%1 – rekonq (Private Browsing)", title));
        }
        else
        {
            setWindowTitle(i18nc("window title, %1 = title of the active website", "%1 – rekonq", title));
        }
    }
}


void MainWindow::fileOpen()
{
    QString filePath = KFileDialog::getOpenFileName(KUrl(),
                       i18n("*.html *.htm *.svg *.png *.gif *.svgz|Web Resources (*.html *.htm *.svg *.png *.gif *.svgz)\n"
                            "*.*|All files (*.*)"),
                       this,
                       i18n("Open Web Resource"));

    if (filePath.isEmpty())
        return;

    rApp->loadUrl(filePath);
}


void MainWindow::printRequested(QWebFrame *frame)
{
    if (!currentTab())
        return;

    if (currentTab()->page()->isOnRekonqPage())
    {
        // trigger print part action instead of ours..
        KParts::ReadOnlyPart *p = currentTab()->part();
        if (p)
        {
            KParts::BrowserExtension *ext = p->browserExtension();
            if (ext)
            {
                KParts::BrowserExtension::ActionSlotMap *actionSlotMap = KParts::BrowserExtension::actionSlotMapPtr();

                connect(this, SIGNAL(triggerPartPrint()), ext, actionSlotMap->value("print"));
                emit triggerPartPrint();

                return;
            }
        }
    }

    QWebFrame *printFrame = 0;
    if (frame == 0)
    {
        printFrame = currentTab()->page()->mainFrame();
    }
    else
    {
        printFrame = frame;
    }

    QPrinter printer;
    QPrintPreviewDialog previewdlg(&printer, this);

    connect(&previewdlg, SIGNAL(paintRequested(QPrinter *)), printFrame, SLOT(print(QPrinter *)));

    previewdlg.exec();
}


void MainWindow::find(const QString & search)
{
    if (!currentTab())
        return;
    m_lastSearch = search;

    updateHighlight();
    findNext();
}


void MainWindow::matchCaseUpdate()
{
    if (!currentTab())
        return;

    currentTab()->view()->findText(m_lastSearch, QWebPage::FindBackward);
    findNext();
    updateHighlight();
}


void MainWindow::findNext()
{
    if (!currentTab())
        return;

    if (currentTab()->page()->isOnRekonqPage())
    {
        // trigger part find action
        KParts::ReadOnlyPart *p = currentTab()->part();
        if (p)
        {
            connect(this, SIGNAL(triggerPartFind()), p, SLOT(slotFind()));
            emit triggerPartFind();
            return;
        }
    }

    if (m_findBar->isHidden())
    {
        QPoint previous_position = currentTab()->view()->page()->currentFrame()->scrollPosition();
        currentTab()->view()->page()->focusNextPrevChild(true);
        currentTab()->view()->page()->currentFrame()->setScrollPosition(previous_position);
        return;
    }

    QWebPage::FindFlags options = QWebPage::FindWrapsAroundDocument;
    if (m_findBar->matchCase())
        options |= QWebPage::FindCaseSensitively;

    bool found = currentTab()->view()->findText(m_lastSearch, options);
    m_findBar->notifyMatch(found);

    if (!found)
    {
        QPoint previous_position = currentTab()->view()->page()->currentFrame()->scrollPosition();
        currentTab()->view()->page()->focusNextPrevChild(true);
        currentTab()->view()->page()->currentFrame()->setScrollPosition(previous_position);
    }
}


void MainWindow::findPrevious()
{
    if (!currentTab())
        return;

    QWebPage::FindFlags options = QWebPage::FindBackward | QWebPage::FindWrapsAroundDocument;
    if (m_findBar->matchCase())
        options |= QWebPage::FindCaseSensitively;

    bool found = currentTab()->view()->findText(m_lastSearch, options);
    m_findBar->notifyMatch(found);
}


void MainWindow::updateHighlight()
{
    if (!currentTab())
        return;

    QWebPage::FindFlags options = QWebPage::HighlightAllOccurrences;

    currentTab()->view()->findText("", options); //Clear an existing highlight

    if (m_findBar->highlightAllState() && !m_findBar->isHidden())
    {
        if (m_findBar->matchCase())
            options |= QWebPage::FindCaseSensitively;

        currentTab()->view()->findText(m_lastSearch, options);
    }
}

void MainWindow::findSelectedText()
{
    // FindBar::setVisible() gets the selected text by itself
    m_findBar->show();
}


void MainWindow::viewFullScreen(bool makeFullScreen)
{
    setWidgetsVisible(!makeFullScreen);
    KToggleFullScreenAction::setFullScreen(this, makeFullScreen);
}


void MainWindow::setWidgetsVisible(bool makeVisible)
{
    // state flags
    static bool bookmarksToolBarFlag;
    static bool historyPanelFlag;
    static bool bookmarksPanelFlag;

    KToolBar *mainBar = toolBar("mainToolBar");
    KToolBar *bookBar = toolBar("bookmarksToolBar");

    if (!makeVisible)
    {
        // save current state, if in windowed mode
        if (!isFullScreen())
        {
            bookmarksToolBarFlag = bookBar->isHidden();
            historyPanelFlag = m_historyPanel->isHidden();
            bookmarksPanelFlag = m_bookmarksPanel->isHidden();
        }

        bookBar->hide();
        m_view->tabBar()->hide();
        m_historyPanel->hide();
        m_bookmarksPanel->hide();

        // hide main toolbar
        mainBar->hide();
    }
    else
    {
        // show main toolbar
        mainBar->show();
        m_view->tabBar()->show();

        // restore state of windowed mode
        if (!bookmarksToolBarFlag)
            bookBar->show();
        if (!historyPanelFlag)
            m_historyPanel->show();
        if (!bookmarksPanelFlag)
            m_bookmarksPanel->show();
    }
}


QString MainWindow::selectedText() const
{
    if (!currentTab())
        return QString();

    return currentTab()->view()->selectedText();
}


void MainWindow::viewPageSource()
{
    if (!currentTab())
        return;

    KUrl url = currentTab()->url();
    KRun::runUrl(url, QL1S("text/plain"), this, false);
}


void MainWindow::homePage(Qt::MouseButtons mouseButtons, Qt::KeyboardModifiers keyboardModifiers)
{
    KUrl homeUrl = ReKonfig::useNewTabPage()
                   ? KUrl(QL1S("about:home"))
                   : KUrl(ReKonfig::homePage());

    if (mouseButtons == Qt::MidButton || keyboardModifiers == Qt::ControlModifier)
        rApp->loadUrl(homeUrl, Rekonq::NewTab);
    else
        currentTab()->view()->load(homeUrl);
}


WebTab *MainWindow::currentTab() const
{
    return m_view->currentWebTab();
}


void MainWindow::browserLoading(bool v)
{
    QAction *stop = actionCollection()->action(QL1S("stop"));
    QAction *reload = actionCollection()->action(QL1S("view_redisplay"));
    if (v)
    {
        disconnect(m_stopReloadAction, SIGNAL(triggered(bool)), reload , SIGNAL(triggered(bool)));
        m_stopReloadAction->setIcon(KIcon("process-stop"));
        m_stopReloadAction->setToolTip(i18n("Stop loading the current page"));
        m_stopReloadAction->setText(i18n("Stop"));
        connect(m_stopReloadAction, SIGNAL(triggered(bool)), stop, SIGNAL(triggered(bool)));
        stop->setEnabled(true);
    }
    else
    {
        disconnect(m_stopReloadAction, SIGNAL(triggered(bool)), stop , SIGNAL(triggered(bool)));
        m_stopReloadAction->setIcon(KIcon("view-refresh"));
        m_stopReloadAction->setToolTip(i18n("Reload the current page"));
        m_stopReloadAction->setText(i18n("Reload"));
        connect(m_stopReloadAction, SIGNAL(triggered(bool)), reload, SIGNAL(triggered(bool)));
        stop->setEnabled(false);

        updateActions();
    }
}


void MainWindow::openPrevious(Qt::MouseButtons mouseButtons, Qt::KeyboardModifiers keyboardModifiers)
{
    QWebHistory *history = currentTab()->view()->history();
    QWebHistoryItem *item = 0;

    if (currentTab()->page()->isOnRekonqPage())
    {
        item = new QWebHistoryItem(history->currentItem());
    }
    else
    {
        if (history->canGoBack())
        {
            item = new QWebHistoryItem(history->backItem());
        }
    }

    if (!item)
        return;

    if (mouseButtons == Qt::MidButton || keyboardModifiers == Qt::ControlModifier)
    {
        rApp->loadUrl(item->url(), Rekonq::NewTab);
    }
    else
    {
        history->goToItem(*item);
    }

    updateActions();
}


void MainWindow::openNext(Qt::MouseButtons mouseButtons, Qt::KeyboardModifiers keyboardModifiers)
{
    QWebHistory *history = currentTab()->view()->history();
    QWebHistoryItem *item = 0;

    if (currentTab()->view()->page()->isOnRekonqPage())
    {
        item = new QWebHistoryItem(history->currentItem());
    }
    else
    {
        if (history->canGoForward())
        {
            item = new QWebHistoryItem(history->forwardItem());
        }
    }

    if (!item)
        return;

    if (mouseButtons == Qt::MidButton || keyboardModifiers == Qt::ControlModifier)
    {
        rApp->loadUrl(item->url(), Rekonq::NewTab);
    }
    else
    {
        history->goToItem(*item);
    }

    updateActions();
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // ctrl + tab action
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Tab))
    {
        emit ctrlTabPressed();
        return;
    }

    // shift + ctrl + tab action
    if ((event->modifiers() == Qt::ControlModifier + Qt::ShiftModifier) && (event->key() == Qt::Key_Backtab))
    {
        emit shiftCtrlTabPressed();
        return;
    }

    KMainWindow::keyPressEvent(event);
}


bool MainWindow::event(QEvent *event)
{
    // Avoid a conflict with window-global actions
    if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress)
    {
        QKeyEvent *kev = static_cast<QKeyEvent *>(event);
        if (kev->key() == Qt::Key_Escape)
        {
            // if zoombar is visible, hide it
            if (m_zoomBar->isVisible())
            {
                m_zoomBar->hide();
                event->accept();
                currentTab()->setFocus();
                return true;
            }

            // if findbar is visible, hide it
            if (m_findBar->isVisible())
            {
                m_findBar->hide();
                event->accept();
                currentTab()->setFocus();
                return true;
            }
        }
    }
    return KMainWindow::event(event);
}


void MainWindow::notifyMessage(const QString &msg, Rekonq::Notify status)
{
    if (this != QApplication::activeWindow())
    {
        return;
    }

    // deleting popus if empty msgs
    if (msg.isEmpty())
    {
        m_hidePopupTimer->start(250);
        return;
    }

    m_hidePopupTimer->stop();


    switch (status)
    {
    case Rekonq::Url:
        break;
    case Rekonq::Info:
        m_hidePopupTimer->start(500);
        break;
    case Rekonq::Success:
        break;
    case Rekonq::Error:
        break;
    case Rekonq::Download:
        break;
    default:
        break;
    }

    int margin = 4;

    // setting popup size
    QLabel *label = new QLabel(msg);
    m_popup->setView(label);
    QSize labelSize(label->fontMetrics().width(msg) + 2*margin, label->fontMetrics().height() + 2*margin);
    if (labelSize.width() > width())
    {
        labelSize.setWidth(width());
        label->setText(label->fontMetrics().elidedText(msg, Qt::ElideMiddle, width()));
    }
    m_popup->setFixedSize(labelSize);
    m_popup->layout()->setAlignment(Qt::AlignTop);
    m_popup->layout()->setMargin(margin);

    // useful values
    WebTab *tab = m_view->currentWebTab();

    // fix crash on window close
    if (!tab || !tab->page())
        return;

    bool horizontalScrollbarIsVisible = tab->page()->currentFrame()->scrollBarMaximum(Qt::Horizontal);
    bool verticalScrollbarIsVisible = tab->page()->currentFrame()->scrollBarMaximum(Qt::Vertical);

    //TODO: detect QStyle sizeHint, instead of fixed 17
    int hScrollbarSize = horizontalScrollbarIsVisible ? 17 : 0;
    int vScrollbarSize = verticalScrollbarIsVisible ? 17 : 0;
    
    QPoint webViewOrigin = tab->view()->mapToGlobal(QPoint(0, 0));
    QPoint mousePos = tab->mapToGlobal(tab->view()->mousePos());

    // setting popup in bottom-left position
    int x = mapToGlobal(QPoint(0, 0)).x();
    int y = webViewOrigin.y() + tab->page()->viewportSize().height() - labelSize.height() - hScrollbarSize;

    if ( QRect(x, y, labelSize.width() , labelSize.height() ).contains(mousePos) )
    {
        // settings popup on the right
        x = mapToGlobal(QPoint(- labelSize.width() + tab->page()->viewportSize().width(), 0)).x() - vScrollbarSize;
    }

    if (QRect(x , y , labelSize.width() , labelSize.height()).contains(mousePos))
    {
        // setting popup above the mouse
        y -= labelSize.height();
    }
    
    m_popup->show(QPoint(x, y));
}


void MainWindow::clearPrivateData()
{
    QPointer<KDialog> dialog = new KDialog(this);
    dialog->setCaption(i18nc("@title:window", "Clear Private Data"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);

    dialog->button(KDialog::Ok)->setIcon(KIcon("edit-clear"));
    dialog->button(KDialog::Ok)->setText(i18n("Clear"));

    Ui::ClearDataWidget clearWidget;
    QWidget widget;
    clearWidget.setupUi(&widget);
    clearWidget.clearHistory->setChecked(ReKonfig::clearHistory());
    clearWidget.clearDownloads->setChecked(ReKonfig::clearDownloads());
    clearWidget.clearCookies->setChecked(ReKonfig::clearCookies());
    clearWidget.clearCachedPages->setChecked(ReKonfig::clearCachedPages());
    clearWidget.clearWebIcons->setChecked(ReKonfig::clearWebIcons());
    clearWidget.homePageThumbs->setChecked(ReKonfig::clearHomePageThumbs());

    dialog->setMainWidget(&widget);
    dialog->exec();

    if (dialog->result() == QDialog::Accepted)
    {
        //Save current state
        ReKonfig::setClearHistory(clearWidget.clearHistory->isChecked());
        ReKonfig::setClearDownloads(clearWidget.clearDownloads->isChecked());
        ReKonfig::setClearCookies(clearWidget.clearDownloads->isChecked());
        ReKonfig::setClearCachedPages(clearWidget.clearCachedPages->isChecked());
        ReKonfig::setClearWebIcons(clearWidget.clearWebIcons->isChecked());
        ReKonfig::setClearHomePageThumbs(clearWidget.homePageThumbs->isChecked());

        if (clearWidget.clearHistory->isChecked())
        {
            rApp->historyManager()->clear();
        }

        if (clearWidget.clearDownloads->isChecked())
        {
            rApp->clearDownloadsHistory();
        }

        if (clearWidget.clearCookies->isChecked())
        {
            QDBusInterface kcookiejar("org.kde.kded", "/modules/kcookiejar", "org.kde.KCookieServer");
            QDBusReply<void> reply = kcookiejar.call("deleteAllCookies");
        }

        if (clearWidget.clearCachedPages->isChecked())
        {
            KProcess::startDetached(KStandardDirs::findExe("kio_http_cache_cleaner"),
                                    QStringList(QL1S("--clear-all")));
        }

        if (clearWidget.clearWebIcons->isChecked())
        {
            rApp->iconManager()->clearIconCache();
        }

        if (clearWidget.homePageThumbs->isChecked())
        {
            QString path = KStandardDirs::locateLocal("cache", QString("thumbs/rekonq"), true);
            path.remove("rekonq");
            QDir cacheDir(path);
            QStringList fileList = cacheDir.entryList();
            foreach(const QString &str, fileList)
            {
                QFile file(path + str);
                file.remove();
            }
        }
    }

    dialog->deleteLater();
}

void MainWindow::aboutToShowBackMenu()
{
    m_historyBackMenu->clear();
    if (!currentTab())
        return;
    QWebHistory *history = currentTab()->view()->history();
    int pivot = history->currentItemIndex();
    int offset = 0;
    const int maxItemNumber = 8;  // no more than 8 elements in the Back History Menu!
    QList<QWebHistoryItem> historyList = history->backItems(maxItemNumber);
    int listCount = historyList.count();
    if (pivot >= maxItemNumber)
        offset = pivot - maxItemNumber;

    if (currentTab()->view()->page()->isOnRekonqPage())
    {
        QWebHistoryItem item = history->currentItem();
        KAction *action = new KAction(this);
        action->setData(listCount + offset++);
        KIcon icon = rApp->iconManager()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyBackMenu->addAction(action);
    }

    for (int i = listCount - 1; i >= 0; --i)
    {
        QWebHistoryItem item = historyList.at(i);
        KAction *action = new KAction(this);
        action->setData(i + offset);
        KIcon icon = rApp->iconManager()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyBackMenu->addAction(action);
    }
}


void MainWindow::aboutToShowForwardMenu()
{
    m_historyForwardMenu->clear();

    if (!currentTab())
        return;

    QWebHistory *history = currentTab()->view()->history();
    const int pivot = history->currentItemIndex();
    int offset = 0;
    const int maxItemNumber = 8;  // no more than 8 elements in the Forward History Menu!
    QList<QWebHistoryItem> historyList = history->forwardItems(maxItemNumber);
    int listCount = historyList.count();

    if (pivot >= maxItemNumber)
        offset = pivot - maxItemNumber;

    if (currentTab()->view()->page()->isOnRekonqPage())
    {
        QWebHistoryItem item = history->currentItem();
        KAction *action = new KAction(this);
        action->setData(listCount + offset++);
        KIcon icon = rApp->iconManager()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyForwardMenu->addAction(action);
    }

    for (int i = 1; i <= listCount; i++)
    {
        QWebHistoryItem item = historyList.at(i - 1);
        KAction *action = new KAction(this);
        action->setData(pivot + i + offset);
        KIcon icon = rApp->iconManager()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyForwardMenu->addAction(action);
    }
}


void MainWindow::aboutToShowTabListMenu()
{
    m_tabListMenu->clear();
    for (int i = 0; i < m_view->count(); ++i)
    {
        KAction *action = new KAction(m_view->tabText(i), this);
        action->setIcon(rApp->iconManager()->iconForUrl(m_view->webTab(i)->url()).pixmap(16, 16));
        action->setData(i);
        if (mainView()->tabBar()->currentIndex() == i)
        {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }
        m_tabListMenu->addAction(action);
    }
    m_tabListMenu->adjustSize();
}


void MainWindow::openActionUrl(QAction *action)
{
    int index = action->data().toInt();

    QWebHistory *history = currentTab()->view()->history();
    if (!history->itemAt(index).isValid())
    {
        kDebug() << "Invalid Index!: " << index;
        return;
    }

    history->goToItem(history->itemAt(index));
}

void MainWindow::openActionTab(QAction* action)
{
    int index = action->data().toInt();
    if (index < 0 || index >= m_view->count())
    {
        kDebug() << "Invalid Index!: " << index;
        return;
    }
    m_view->setCurrentIndex(index);
}


void MainWindow::setUserAgent()
{
    QAction *sender = static_cast<QAction *>(QObject::sender());

    QString info;
    QString desc = sender->text();
    int uaIndex = sender->data().toInt();

    KUrl url = currentTab()->url();
    UserAgentInfo uaInfo;
    kDebug() << "SETTING USER AGENT";
    uaInfo.setUserAgentForHost(uaIndex, url.host());
    currentTab()->page()->triggerAction(QWebPage::Reload);
}


void MainWindow::populateUserAgentMenu()
{
    kDebug() << "populating user agent menu...";
    bool defaultUA = true;
    KUrl url = currentTab()->url();

    QAction *a, *defaultAction;

    m_userAgentMenu->clear();

    defaultAction = new QAction(i18nc("Default rekonq user agent", "Default"), this);
    defaultAction->setData(-1);
    defaultAction->setCheckable(true);
    connect(defaultAction, SIGNAL(triggered(bool)), this, SLOT(setUserAgent()));

    m_userAgentMenu->addAction(defaultAction);
    m_userAgentMenu->addSeparator();

    UserAgentInfo uaInfo;
    QStringList UAlist = uaInfo.availableUserAgents();
    int uaIndex = uaInfo.uaIndexForHost(currentTab()->url().host());

    for (int i = 0; i < UAlist.count(); ++i)
    {
        QString uaDesc = UAlist.at(i);

        a = new QAction(uaDesc, this);
        a->setData(i);
        a->setCheckable(true);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(setUserAgent()));

        if (i == uaIndex)
        {
            a->setChecked(true);
            defaultUA = false;
        }
        m_userAgentMenu->addAction(a);
    }
    defaultAction->setChecked(defaultUA);

    m_userAgentMenu->addSeparator();
    m_userAgentMenu->addAction(actionByName("UserAgentSettings"));
}


void MainWindow::enableNetworkAnalysis(bool b)
{
    currentTab()->page()->enableNetworkAnalyzer(b);
    m_analyzerPanel->toggle(b);
}


bool MainWindow::queryClose()
{
    // this should fux bug 240432
    if (rApp->sessionSaving())
        return true;

    // smooth private browsing mode
    if (QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return true;

    if (rApp->mainWindowList().count() > 1)
    {
        int answer = KMessageBox::questionYesNoCancel(
                        this,
                        i18n("Wanna close the window or the whole app?"),
                        i18n("Application/Window closing..."),
                        KGuiItem(i18n("C&lose Current Window"), KIcon("window-close")),
                        KStandardGuiItem::quit(),
                        KStandardGuiItem::cancel(),
                        "confirmClosingMultipleWindows"
                     );

        switch (answer)
        {
        case KMessageBox::Yes:
            return true;

        case KMessageBox::No:
            rApp->quit();
            return true;

        default:
            return false;
        }
    }
    return true;
}


void MainWindow::saveNewToolbarConfig()
{
    KXmlGuiWindow::saveNewToolbarConfig();
    setupBookmarksAndToolsShortcuts();
}


void MainWindow::setupBookmarksAndToolsShortcuts()
{
    KToolBar *mainBar = toolBar("mainToolBar");

    QToolButton *bookmarksButton = qobject_cast<QToolButton*>(mainBar->widgetForAction(actionByName(QL1S("bookmarksActionMenu"))));
    if (bookmarksButton)
    {
        connect(actionByName(QL1S("bookmarksActionMenu")), SIGNAL(triggered()), bookmarksButton, SLOT(showMenu()));
    }

    QToolButton *toolsButton = qobject_cast<QToolButton*>(mainBar->widgetForAction(actionByName(QL1S("rekonq_tools"))));
    if (toolsButton)
    {
        connect(actionByName(QL1S("rekonq_tools")), SIGNAL(triggered()), toolsButton, SLOT(showMenu()));
    }
}


void MainWindow::showUserAgentSettings()
{
    QPointer<KDialog> dialog = new KDialog(this);
    dialog->setCaption(i18nc("@title:window", "User Agent Settings"));
    dialog->setButtons(KDialog::Ok);

    UserAgentWidget widget;
    dialog->setMainWidget(&widget);
    dialog->exec();

    dialog->deleteLater();
}


void MainWindow::moveEvent(QMoveEvent *event)
{
    if (m_hidePopupTimer)
        m_hidePopupTimer->stop();
    if (m_popup)
        m_popup->hide();

    KMainWindow::moveEvent(event);
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_hidePopupTimer)
        m_hidePopupTimer->stop();
    if (m_popup)
        m_popup->hide();

    KMainWindow::resizeEvent(event);
}
