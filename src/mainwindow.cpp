/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2012 by Andrea Diamantini <adjam7 at gmail dot com>
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
#include "adblockmanager.h"
#include "analyzerpanel.h"
#include "application.h"
#include "bookmarkmanager.h"
#include "bookmarkspanel.h"
#include "bookmarkstoolbar.h"
#include "downloadmanager.h"
#include "findbar.h"
#include "historypanel.h"
#include "iconmanager.h"
#include "mainview.h"
#include "rekonqmenu.h"
#include "sessionmanager.h"
#include "settingsdialog.h"
#include "stackedurlbar.h"
#include "syncmanager.h"
#include "tabbar.h"
#include "urlbar.h"
#include "webinspectorpanel.h"
#include "webpage.h"
#include "webtab.h"
#include "zoombar.h"
#include "useragentmanager.h"

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
#include <KProcess>
#include <KPushButton>
#include <KProtocolManager>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KToggleFullScreenAction>
#include <KXMLGUIFactory>
#include <kdeprintdialog.h>

#include <KParts/Part>
#include <KParts/BrowserExtension>
#include <KMimeTypeTrader>

// Qt Includes
#include <QtCore/QTimer>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <QtGui/QDesktopWidget>
#include <QtGui/QLabel>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QVBoxLayout>

#include <QtWebKit/QWebHistory>

#include <QSignalMapper>
#include <QTextDocument>


MainWindow::MainWindow()
    : KXmlGuiWindow()
    , m_view(new MainView(this))
    , m_findBar(new FindBar(this))
    , m_zoomBar(new ZoomBar(this))
    , m_historyPanel(0)
    , m_bookmarksPanel(0)
    , m_webInspectorPanel(0)
    , m_analyzerPanel(0)
    , m_loadStopReloadAction(0)
    , m_historyBackMenu(0)
    , m_historyForwardMenu(0)
    , m_tabListMenu(0)
    , m_bookmarksBar(0)
    , m_popup(new QLabel(this))
    , m_hidePopupTimer(new QTimer(this))
    , m_rekonqMenu(0)
{
    // Setting attributes (just to be sure...)
    setAttribute(Qt::WA_DeleteOnClose, true);

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

    // BEFORE setupGUI, AFTER setupActions!!
    m_view->addNewTabButton(actionByName("new_tab"));

    // setting Panels
    setupPanels();

    // setting up rekonq tools
    setupTools();

    // setting up rekonq toolbar(s)
    setupToolbars();

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

    setupBookmarksAndToolsShortcuts();

    // setting popup notification
    connect(rApp, SIGNAL(focusChanged(QWidget*,QWidget*)), m_popup, SLOT(hide()));
    m_popup->setAutoFillBackground(true);
    m_popup->setMargin(4);
    m_popup->raise();
    m_popup->hide();
    connect(m_hidePopupTimer, SIGNAL(timeout()), m_popup, SLOT(hide()));

    // notification system
    connect(m_view, SIGNAL(showStatusBarMessage(QString,Rekonq::Notify)), this, SLOT(notifyMessage(QString,Rekonq::Notify)));
    connect(m_view, SIGNAL(linkHovered(QString)), this, SLOT(notifyMessage(QString)));
    connect(rApp->downloadManager(), SIGNAL(notifyDownload(QString,Rekonq::Notify)),
            this, SLOT(notifyMessage(QString,Rekonq::Notify)));

    // connect signals and slots
    connect(m_view, SIGNAL(currentTitle(QString)), this, SLOT(updateWindowTitle(QString)));
    connect(m_view, SIGNAL(printRequested(QWebFrame*)), this, SLOT(printRequested(QWebFrame*)));
    connect(m_view, SIGNAL(closeWindow()), this, SLOT(close()));

    // (shift +) ctrl + tab switching
    connect(this, SIGNAL(ctrlTabPressed()), m_view, SLOT(nextTab()));
    connect(this, SIGNAL(shiftCtrlTabPressed()), m_view, SLOT(previousTab()));

    // wheel history navigation
    connect(m_view, SIGNAL(openPreviousInHistory()), this, SLOT(openPrevious()));
    connect(m_view, SIGNAL(openNextInHistory()), this, SLOT(openNext()));

    // update actions
    connect(m_view, SIGNAL(currentChanged(int)), this, SLOT(updateHistoryActions()));
    connect(m_view, SIGNAL(currentTabStateChanged()), this, SLOT(updateTabActions()));

    // Change window icon according to tab icon
    connect(m_view, SIGNAL(currentChanged(int)), this, SLOT(changeWindowIcon(int)));

    // Find Bar signal
    connect(m_findBar, SIGNAL(searchString(QString)), this, SLOT(find(QString)));

    // Zoom Bar signal
    connect(m_view, SIGNAL(currentChanged(int)), m_zoomBar, SLOT(updateSlider(int)));

    // Save session on window closing
    connect(this, SIGNAL(windowClosing()), rApp->sessionManager(), SLOT(saveSession()));

    // setting up toolbars to NOT have context menu enabled
    setContextMenuPolicy(Qt::DefaultContextMenu);

    // accept d'n'd
    setAcceptDrops(true);

    // Things that need to be setup after the call to setupGUI() and after ctor call
    QTimer::singleShot(1, this, SLOT(postLaunch()));
}


MainWindow::~MainWindow()
{
    m_hidePopupTimer->stop();

    rApp->bookmarkManager()->removeBookmarkBar(m_bookmarksBar);
    rApp->bookmarkManager()->removeBookmarkPanel(m_bookmarksPanel);
    rApp->removeMainWindow(this);
}


void MainWindow::setupToolbars()
{
    KAction *a;

    // location bar
    a = new KAction(i18n("Location Bar"), this);
    a->setDefaultWidget(m_view->widgetBar());
    actionCollection()->addAction(QL1S("url_bar"), a);
}


void MainWindow::postLaunch()
{
    // this just to fix reopening rekonq after fullscreen close
    KToolBar *mainBar = toolBar("mainToolBar");
    mainBar->show();  
    
    // Bookmarks Bar
    KToolBar *XMLGUIBkBar = toolBar("bookmarkToolBar");
    if (!XMLGUIBkBar)
        return;

    if (m_bookmarksBar)
    {
        rApp->bookmarkManager()->removeBookmarkBar(m_bookmarksBar);
        delete m_bookmarksBar;
    }
    m_bookmarksBar = new BookmarkToolBar(XMLGUIBkBar, this);
    rApp->bookmarkManager()->registerBookmarkBar(m_bookmarksBar);

    QAction *a;

    // Bookmarks bar action
    a = actionByName(QL1S("show_bookmarks_toolbar"));
    a->setChecked(XMLGUIBkBar->isVisible());
    connect(XMLGUIBkBar, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

    // History panel action
    a = actionByName(QL1S("show_history_panel"));
    a->setChecked(m_historyPanel->isVisible());

    // Bookmarks panel action
    a = actionByName(QL1S("show_bookmarks_panel"));
    a->setChecked(m_bookmarksPanel->isVisible());
}


void MainWindow::toggleBookmarkBarVisible(bool visible)
{
    if (m_bookmarksBar)
        m_bookmarksBar->toolBar()->setVisible(visible);
}


void MainWindow::configureToolbars()
{
    if (autoSaveSettings())
        saveAutoSaveSettings();

    KEditToolBar dlg(factory(), this);
    // The bookmark bar needs to be refill after the UI changes are finished
    connect(&dlg, SIGNAL(newToolBarConfig()), this, SLOT(postLaunch()));
    dlg.exec();
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
    KStandardAction::quit(rApp, SLOT(queryQuit()), actionCollection());

    a = KStandardAction::find(m_findBar, SLOT(show()), actionCollection());
    KShortcut findShortcut = KStandardShortcut::find();
    a->setShortcut(findShortcut);

    KStandardAction::findNext(this, SLOT(findNext()) , actionCollection());
    KStandardAction::findPrev(this, SLOT(findPrevious()) , actionCollection());

    a = KStandardAction::fullScreen(this, SLOT(viewFullScreen(bool)), this, actionCollection());
    KShortcut fullScreenShortcut = KStandardShortcut::fullScreen();
    fullScreenShortcut.setAlternate(Qt::Key_F11);
    a->setShortcut(fullScreenShortcut);

    a = actionCollection()->addAction(KStandardAction::Home);
    connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(homePage()));
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

    // load stop reload Action
    m_loadStopReloadAction = new KAction(this);
    actionCollection()->addAction(QL1S("load_stop_reload") , m_loadStopReloadAction);
    m_loadStopReloadAction->setShortcutConfigurable(false);

    // Open location action
    a = new KAction(i18n("Open Location"), this);
    KShortcut openLocationShortcut(Qt::CTRL + Qt::Key_L);
    openLocationShortcut.setAlternate(Qt::ALT + Qt::Key_D);
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

    //================================Download========================================
    a = new KAction(KIcon("download"), i18n("Downloads"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_J));
    actionCollection()->addAction(QL1S("openDownloadsPage"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(openDownloadsPage()));

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
    connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(openPrevious(Qt::MouseButtons,Qt::KeyboardModifiers)));

    m_historyBackMenu = new KMenu(this);
    a->setMenu(m_historyBackMenu);
    connect(m_historyBackMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowBackMenu()));
    connect(m_historyBackMenu, SIGNAL(triggered(QAction*)), this, SLOT(openActionUrl(QAction*)));

    a = actionCollection()->addAction(KStandardAction::Forward);
    connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(openNext(Qt::MouseButtons,Qt::KeyboardModifiers)));

    m_historyForwardMenu = new KMenu(this);
    a->setMenu(m_historyForwardMenu);
    connect(m_historyForwardMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowForwardMenu()));
    connect(m_historyForwardMenu, SIGNAL(triggered(QAction*)), this, SLOT(openActionUrl(QAction*)));

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
    a->setData(0);  // last tab has always index = 0
    a->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_T));
    actionCollection()->addAction(QL1S("open_last_closed_tab"), a);
    connect(a, SIGNAL(triggered(bool)), m_view, SLOT(openClosedTab()));

    // shortcuts for quickly switching to a tab
    QSignalMapper *tabSignalMapper = new QSignalMapper(this);
    for (int i = 1; i <= 9; i++)
    {
        a = new KAction(i18n("Switch to Tab %1", i), this);
        a->setShortcut(KShortcut(QString("Alt+%1").arg(i)));
        actionCollection()->addAction(QL1S(QString("switch_tab_" + QString::number(i)).toAscii()), a);
        connect(a, SIGNAL(triggered(bool)), tabSignalMapper, SLOT(map()));
        tabSignalMapper->setMapping(a, i);
    }
    connect(tabSignalMapper, SIGNAL(mapped(int)), m_view, SLOT(switchToTab(int)));

    // shortcuts for loading favorite pages
    QSignalMapper *favoritesSignalMapper = new QSignalMapper(this);
    for (int i = 1; i <= 9; ++i)
    {
        a = new KAction(i18n("Switch to Favorite Page %1", i), this);
        a->setShortcut(KShortcut(QString("Ctrl+%1").arg(i)));
        actionCollection()->addAction(QL1S(QString("switch_favorite_" + QString::number(i)).toAscii()), a);
        connect(a, SIGNAL(triggered(bool)), favoritesSignalMapper, SLOT(map()));
        favoritesSignalMapper->setMapping(a, i);
    }
    connect(favoritesSignalMapper, SIGNAL(mapped(int)), m_view, SLOT(loadFavorite(int)));

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
    KActionMenu *bmMenu = rApp->bookmarkManager()->bookmarkActionMenu(this);
    bmMenu->setIcon(KIcon("bookmarks"));
    bmMenu->setDelayed(false);
    bmMenu->setShortcutConfigurable(true);
    bmMenu->setShortcut(KShortcut(Qt::ALT + Qt::Key_B));
    actionCollection()->addAction(QL1S("bookmarksActionMenu"), bmMenu);

    // Bookmark Toolbar
    a = new KAction(KIcon("bookmarks-bar"), i18n("Bookmarks Toolbar"), this);
    a->setCheckable(true);
    actionCollection()->addAction(QL1S("show_bookmarks_toolbar"), a);
    connect(a, SIGNAL(toggled(bool)), this, SLOT(toggleBookmarkBarVisible(bool)));

    // User Agent
    a = new KAction(KIcon("preferences-web-browser-identification"), i18n("Browser Identification"), this);
    actionCollection()->addAction(QL1S("useragent"), a);
    KMenu *uaMenu = new KMenu(this);
    a->setMenu(uaMenu);
    connect(uaMenu, SIGNAL(aboutToShow()), this, SLOT(populateUserAgentMenu()));

    // Editable Page
    a = new KAction(KIcon("document-edit"), i18n("Set Editable"), this);
    a->setCheckable(true);
    actionCollection()->addAction(QL1S("set_editable"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(setEditable(bool)));

    // Adblock
    a = new KAction(KIcon("preferences-web-browser-adblock"), i18n("Ad Block"), this);
    actionCollection()->addAction(QL1S("adblock"), a);
    connect(a, SIGNAL(triggered(bool)), rApp->adblockManager(), SLOT(showSettings()));

    // Web Applications
    a = new KAction(KIcon("applications-internet"), i18n("Create application shortcut"), this);
    actionCollection()->addAction(QL1S("webapp_shortcut"), a);
    connect(a, SIGNAL(triggered(bool)), rApp, SLOT(createWebAppShortcut()));

    // Sync action
    a = new KAction(KIcon("tools-wizard"), i18n("Sync"), this); // FIXME sync icon!!
    actionCollection()->addAction(QL1S("sync"), a);
    connect(a, SIGNAL(triggered(bool)), rApp->syncManager(), SLOT(showSettings()));
}


void MainWindow::setupTools()
{
    KActionMenu *toolsAction = new KActionMenu(KIcon("configure"), i18n("&Tools"), this);
    toolsAction->setDelayed(false);
    toolsAction->setShortcutConfigurable(true);
    toolsAction->setShortcut(KShortcut(Qt::ALT + Qt::Key_T));
    m_rekonqMenu = new RekonqMenu(this);
    toolsAction->setMenu(m_rekonqMenu); // dummy menu to have the dropdown arrow

    // adding rekonq_tools to rekonq actionCollection
    actionCollection()->addAction(QL1S("rekonq_tools"), toolsAction);
}


void MainWindow::setupPanels()
{
    KAction* a;

    // STEP 1
    // Setup history panel
    const QString historyTitle = i18n("History Panel");
    m_historyPanel = new HistoryPanel(historyTitle, this);
    connect(m_historyPanel, SIGNAL(openUrl(KUrl,Rekonq::OpenType)),
            rApp, SLOT(loadUrl(KUrl,Rekonq::OpenType)));
    connect(m_historyPanel, SIGNAL(itemHovered(QString)), this, SLOT(notifyMessage(QString)));
    connect(m_historyPanel, SIGNAL(destroyed()), rApp, SLOT(saveConfiguration()));

    addDockWidget(Qt::LeftDockWidgetArea, m_historyPanel);

    // setup history panel action
    a = new KAction(KIcon("view-history"), historyTitle, this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
    actionCollection()->addAction(QL1S("show_history_panel"), a);
    a->setCheckable(true);
    connect(a, SIGNAL(triggered(bool)), m_historyPanel, SLOT(setVisible(bool)));
    connect(m_historyPanel, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

    // STEP 2
    // Setup bookmarks panel
    const QString bookmarksTitle = i18n("Bookmarks Panel");
    m_bookmarksPanel = new BookmarksPanel(bookmarksTitle, this);
    connect(m_bookmarksPanel, SIGNAL(openUrl(KUrl,Rekonq::OpenType)),
            rApp, SLOT(loadUrl(KUrl,Rekonq::OpenType)));
    connect(m_bookmarksPanel, SIGNAL(itemHovered(QString)), this, SLOT(notifyMessage(QString)));
    connect(m_bookmarksPanel, SIGNAL(destroyed()), rApp, SLOT(saveConfiguration()));

    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarksPanel);

    rApp->bookmarkManager()->registerBookmarkPanel(m_bookmarksPanel);

    // setup bookmarks panel action
    a = new KAction(KIcon("bookmarks-organize"), bookmarksTitle, this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_B));
    actionCollection()->addAction(QL1S("show_bookmarks_panel"), a);
    a->setCheckable(true);
    connect(a, SIGNAL(triggered(bool)), m_bookmarksPanel, SLOT(setVisible(bool)));
    connect(m_bookmarksPanel, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

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


void MainWindow::finalizeGUI(KXMLGUIClient* client)
{
    KXmlGuiWindow::finalizeGUI(client);

    // update rekonqMenu when GUI has changed
    KMenu *m = qobject_cast<KMenu*>(factory()->container("rekonqMenu", this));
    if (m)
        m_rekonqMenu->addActions(m->actions());
    else
        kDebug() << " ====================== "
                 << "Could not get the rekonqMenu menu. Maybe the rekonqui.rc file wasn't found."
                 << "Was rekonq installed correctly?"
                 << " ====================== ";
}

void MainWindow::readProperties(const KConfigGroup& config)
{
    Q_UNUSED(config)

    Application::instance()->sessionManager()->restoreMainWindow(this);
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

    if (currentTab()->page()->isOnRekonqPage())
    {
        KParts::ReadOnlyPart *p = currentTab()->part();
        if (p)
        {
            // if this is a KParts document then the w->url() will be empty and the srcUrl
            // must be set to the document url
            srcUrl = p->url();
        }
    }

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

    const KUrl destUrl = KFileDialog::getSaveUrl(name, QString(), this);
    if (destUrl.isEmpty())
        return;

    if (w->page()->isContentEditable())
    {
        QString code = w->page()->mainFrame()->toHtml();
        QFile file(destUrl.url());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);
        out << code;

        return;
    }

    KIO::Job *job = KIO::file_copy(srcUrl, destUrl, -1, KIO::Overwrite);
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
    connect(s, SIGNAL(settingsChanged(QString)), rApp, SLOT(updateConfiguration()));

    s->exec();
    delete s;
}


void MainWindow::updateHistoryActions()
{
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
    printer.setDocName(printFrame->title());
    QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, this);

    if (printDialog) //check if the Dialog was created
    {
        if (printDialog->exec())
            printFrame->print(&printer);

        delete printDialog;
    }
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


void MainWindow::openDownloadsPage()
{
    rApp->loadUrl(KUrl("about:downloads"), Rekonq::NewFocusedTab);
}


void MainWindow::setWidgetsVisible(bool makeVisible)
{
    // state flags
    static bool bookmarksToolBarFlag;
    static bool historyPanelFlag;
    static bool bookmarksPanelFlag;

    KToolBar *mainBar = toolBar("mainToolBar");
    KToolBar *bookBar = toolBar("bookmarkToolBar");

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
    WebTab * w = currentTab();

    if (!w)
        return;

    KUrl url = w->url();
    QString code = w->page()->mainFrame()->toHtml();

    KTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    if (!tmpFile.open())
        return;

    QTextStream out(&tmpFile);
    out << code;
    tmpFile.close();
    KUrl tmpUrl(tmpFile.fileName());

    KParts::ReadOnlyPart *pa = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(QL1S("text/plain"), w, this, QString());
    if (pa)
    {
        WebTab *srcTab = m_view->newWebTab(true);
        srcTab->page()->setIsOnRekonqPage(true);
        srcTab->setPart(pa, tmpUrl);
        srcTab->urlBar()->setQUrl(url.pathOrUrl());
        m_view->setTabText(m_view->currentIndex(), i18n("Source of: ") + url.prettyUrl());
        updateHistoryActions();
    }
    else
        KRun::runUrl(tmpUrl, QL1S("text/plain"), this, false);
}


void MainWindow::homePage()
{
    KUrl homeUrl = ReKonfig::useNewTabPage()
                   ? KUrl(QL1S("about:home"))
                   : KUrl(ReKonfig::homePage());

    currentTab()->view()->load(homeUrl);
}


WebTab *MainWindow::currentTab() const
{
    return m_view->currentWebTab();
}


void MainWindow::updateTabActions()
{
    m_loadStopReloadAction->disconnect();

    if (m_view->currentUrlBar()->hasFocus())
    {
        m_loadStopReloadAction->disconnect();

        m_loadStopReloadAction->setIcon(KIcon("go-jump-locationbar"));
        m_loadStopReloadAction->setToolTip(i18n("Go"));
        m_loadStopReloadAction->setText(i18n("Go"));

        connect(m_loadStopReloadAction, SIGNAL(triggered(bool)), m_view->currentUrlBar(), SLOT(loadDigitedUrl()));
        return;
    }

    QAction *stop = actionCollection()->action(QL1S("stop"));
    QAction *reload = actionCollection()->action(QL1S("view_redisplay"));

    if (currentTab()->isPageLoading())
    {
        m_loadStopReloadAction->setIcon(KIcon("process-stop"));
        m_loadStopReloadAction->setToolTip(i18n("Stop loading the current page"));
        m_loadStopReloadAction->setText(i18n("Stop"));
        connect(m_loadStopReloadAction, SIGNAL(triggered(bool)), stop, SIGNAL(triggered(bool)));
        stop->setEnabled(true);
    }
    else
    {
        m_loadStopReloadAction->setIcon(KIcon("view-refresh"));
        m_loadStopReloadAction->setToolTip(i18n("Reload the current page"));
        m_loadStopReloadAction->setText(i18n("Reload"));
        connect(m_loadStopReloadAction, SIGNAL(triggered(bool)), reload, SIGNAL(triggered(bool)));
        stop->setEnabled(false);

        updateHistoryActions();
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

    updateHistoryActions();
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

    updateHistoryActions();
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // ctrl + tab action
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Tab))
    {
        emit ctrlTabPressed();
        event->accept();
        return;
    }

    // shift + ctrl + tab action
    if ((event->modifiers() == Qt::ControlModifier + Qt::ShiftModifier) && (event->key() == Qt::Key_Backtab))
    {
        emit shiftCtrlTabPressed();
        event->accept();
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
    // deleting popus if empty msgs
    if (msg.isEmpty())
    {
        m_hidePopupTimer->start(250);
        return;
    }

    m_hidePopupTimer->stop();

    switch (status)
    {
    case Rekonq::Info:
        m_hidePopupTimer->start(500);
        break;
    case Rekonq::Url:
    case Rekonq::Success:
    case Rekonq::Error:
    case Rekonq::Download:
        m_hidePopupTimer->start(3000);
        break;
    default:
        break;
    }

    QString msgToShow = Qt::escape(msg);

    // useful values
    WebTab *tab = m_view->currentWebTab();

    // fix crash on window close
    if (!tab || !tab->page())
        return;

    const int margin = 4;
    const int halfWidth = width() / 2;

    // Set Popup size
    QFontMetrics fm = m_popup->fontMetrics();
    QSize labelSize(fm.width(msgToShow) + 2 * margin, fm.height() + 2 * margin);

    if (labelSize.width() > halfWidth)
        labelSize.setWidth(halfWidth);

    m_popup->setFixedSize(labelSize);
    m_popup->setText(fm.elidedText(msgToShow, Qt::ElideMiddle, labelSize.width() - 2 * margin));

    // NOTE: while currentFrame should NEVER be null
    // we are checking here its existence cause of bug:264187
    if (!tab->page()->currentFrame())
        return;

    const bool horizontalScrollbarIsVisible = tab->page()->currentFrame()->scrollBarMaximum(Qt::Horizontal);
    const bool verticalScrollbarIsVisible = tab->page()->currentFrame()->scrollBarMaximum(Qt::Vertical);
    const bool actionBarsVisible = m_findBar->isVisible() || m_zoomBar->isVisible();

    const int scrollbarExtent = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int hScrollbarSize = horizontalScrollbarIsVisible ? scrollbarExtent : 0;
    const int vScrollbarSize = verticalScrollbarIsVisible ? scrollbarExtent : 0;

    const QPoint mousePos = mapFromGlobal(QCursor::pos());
    const QPoint bottomPoint = m_view->mapTo(this, m_view->geometry().bottomLeft());
    // +1 because bottom() returns top() + height() - 1 , see QRect doku
    int y = bottomPoint.y() + 1 - m_popup->height() - hScrollbarSize;
    int x = QRect(QPoint(0, y), labelSize).contains(mousePos) || actionBarsVisible
            ? width() - labelSize.width() - vScrollbarSize
            : 0;

    m_popup->move(x, y);
    m_popup->show();
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
            rApp->downloadManager()->clearDownloadsHistory();
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
            Q_FOREACH(const QString & str, fileList)
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


void MainWindow::enableNetworkAnalysis(bool b)
{
    currentTab()->page()->enableNetworkAnalyzer(b);
    m_analyzerPanel->toggle(b);
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

        // HACK: set button widget in rekonq menu
        m_rekonqMenu->setButtonWidget(toolsButton);
    }
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


void MainWindow::setEditable(bool on)
{
    currentTab()->page()->setContentEditable(on);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    emit windowClosing();
    kDebug() << "CLOSING WINDOW...";
    KXmlGuiWindow::closeEvent(event);
}


void MainWindow::populateUserAgentMenu()
{
    KMenu *uaMenu = static_cast<KMenu *>(QObject::sender());
    rApp->userAgentManager()->populateUAMenuForTabUrl(uaMenu, currentTab());
}
