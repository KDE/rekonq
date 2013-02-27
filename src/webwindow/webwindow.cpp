/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
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


#include "webwindow.h"
#include "webwindow.moc"

// Auto Includes
#include "rekonq.h"

#include "application.h"
#include "tabwindow.h"

#include "adblockmanager.h"
#include "bookmarkmanager.h"
#include "iconmanager.h"
#include "syncmanager.h"
#include "useragentmanager.h"

#include "webpage.h"
#include "webtab.h"

#include "bookmarkstoolbar.h"
#include "findbar.h"
#include "rekonqfactory.h"
#include "rekonqmenu.h"
#include "settingsdialog.h"
#include "urlbar.h"

#include "websnap.h"

// KDE Includes
#include <KIO/Job>
#include <KEditToolBar>
#include <KFileDialog>
#include <KJobUiDelegate>
#include <KMimeTypeTrader>
#include <KTemporaryFile>
#include <KUrl>
#include <KToolBar>
#include <KToggleFullScreenAction>
#include <KShortcutsDialog>
#include <KRun>

// Qt Includes
#include <QLabel>
#include <QStyle>
#include <QTextDocument>
#include <QTimer>
#include <QWebFrame>
#include <QWebView>
#include <QWebHistory>
#include <QVBoxLayout>


WebWindow::WebWindow(QWidget *parent, bool isPrivateBrowsing, WebPage *pg)
    : QWidget(parent)
    , _tab(new WebTab(this, isPrivateBrowsing))
    , _bar(new UrlBar(_tab))
    , m_findBar(new FindBar(this))
    , m_loadStopReloadAction(0)
    , m_rekonqMenu(0)
    , m_popup(new QLabel(this))
    , m_hidePopupTimer(new QTimer(this))
    , _ac(new KActionCollection(this))
{
    if (pg)
    {
        pg->setParent(_tab->view());
        _tab->view()->setPage(pg);
    }

    // then, setup our actions
    setupActions();

    // setting up rekonq tools
    setupTools();

    // layout
    QVBoxLayout *l = new QVBoxLayout(this);

    // main toolbar
    _mainToolBar = qobject_cast<KToolBar *>(RekonqFactory::createWidget(QL1S("mainToolBar"), this));
    l->addWidget(_mainToolBar.data());

    if (ReKonfig::showBookmarksToolbar())
    {
        _bookmarksBar = qobject_cast<BookmarkToolBar *>(RekonqFactory::createWidget(QL1S("bookmarkToolBar"), this));
        BookmarkManager::self()->registerBookmarkBar(_bookmarksBar.data());

        l->addWidget(_bookmarksBar.data());
    }

    l->addWidget(_tab);
    l->addWidget(m_findBar);
    l->setContentsMargins(0, 0, 0, 0);

    setContentsMargins(0, 0, 0, 0);

    // bookmarks toolbar
    connect(rApp, SIGNAL(toggleBookmarksToolbar(bool)), this, SLOT(toggleBookmarksToolbar(bool)));

    // things changed signals
    connect(_tab, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(_tab, SIGNAL(iconChanged()), this, SIGNAL(iconChanged()));

    // check view signals
    connect(_tab, SIGNAL(loadStarted()), this, SLOT(webLoadStarted()));
    connect(_tab, SIGNAL(loadFinished(bool)), this, SLOT(webLoadFinished(bool)));
    connect(_tab, SIGNAL(loadProgress(int)), this, SLOT(webLoadProgress(int)));

    connect(_bar, SIGNAL(focusIn()), this, SLOT(urlbarFocused()));

    // page signals
    connect(page(), SIGNAL(pageCreated(WebPage*)), this, SIGNAL(pageCreated(WebPage*)));

    // message popup
    m_popup->setAutoFillBackground(true);
    m_popup->setMargin(4);
    m_popup->raise();
    m_popup->hide();
    connect(m_hidePopupTimer, SIGNAL(timeout()), m_popup, SLOT(hide()));
    connect(_tab->page(), SIGNAL(linkHovered(QString,QString,QString)), this, SLOT(notifyMessage(QString)));
    connect(_tab, SIGNAL(infoToShow(QString)), this, SLOT(notifyMessage(QString)));

    updateHistoryActions();

    if (parent && parent->isFullScreen())
        setWidgetsHidden(true);
}


WebWindow::~WebWindow()
{
    m_hidePopupTimer->stop();

    if (!_bookmarksBar.isNull())
    {
        BookmarkManager::self()->removeBookmarkBar(_bookmarksBar.data());
        _bookmarksBar.clear();
    }
}


void WebWindow::setupActions()
{
    // this let shortcuts work..
    actionCollection()->addAssociatedWidget(this);

    KAction *a;

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

    // urlbar
    a = new KAction(i18n("Location Bar"), this);
    a->setDefaultWidget(_bar);
    actionCollection()->addAction(QL1S("url_bar"), a);

    // load stop reload Action
    m_loadStopReloadAction = new KAction(this);
    actionCollection()->addAction(QL1S("load_stop_reload") , m_loadStopReloadAction);
    m_loadStopReloadAction->setShortcutConfigurable(false);
    urlbarFocused();

    // new window action
    a = new KAction(KIcon("window-new"), i18n("&New Window"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_N));
    actionCollection()->addAction(QL1S("new_window"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(openNewWindow()));

    // Standard Actions
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    KStandardAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStandardAction::print(_tab, SLOT(printFrame()), actionCollection());
    KStandardAction::preferences(this, SLOT(preferences()), actionCollection());
    KStandardAction::keyBindings(this, SLOT(keyBindings()), actionCollection());
    KStandardAction::quit(rApp, SLOT(queryQuit()), actionCollection());

    // Configure Main Toolbar
    a = new KAction(KIcon("configure-toolbars"), i18n("Configure Main ToolBar"), this);
    actionCollection()->addAction(QL1S("configure_main_toolbar"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(showToolbarEditor()));

    // Bookmark Toolbar
    a = new KAction(KIcon("bookmark-toolbar"), i18n("Bookmarks Toolbar"), this);
    a->setCheckable(true);
    a->setChecked(ReKonfig::showBookmarksToolbar());
    actionCollection()->addAction(QL1S("show_bookmarks_toolbar"), a);
    connect(a, SIGNAL(toggled(bool)), this, SLOT(toggleBookmarksToolbar(bool)));

    // Open Home page
    a = actionCollection()->addAction(KStandardAction::Home);
    connect(a, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(openHomePage(Qt::MouseButtons,Qt::KeyboardModifiers)));

    // Open Downloads page
    a = new KAction(KIcon("download"), i18n("Downloads page"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_J));
    actionCollection()->addAction(QL1S("open_downloads_page"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(openDownloadsPage()));

    // Open History page
    a = new KAction(KIcon("view-history"), i18n("History page"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
    actionCollection()->addAction(QL1S("open_history_page"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(openHistoryPage()));

    // Open Bookmarks page
    a = new KAction(KIcon("bookmarks"), i18n("Bookmarks page"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_B));
    actionCollection()->addAction(QL1S("open_bookmarks_page"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(openBookmarksPage()));

    // find action
    a = KStandardAction::find(m_findBar, SLOT(show()), actionCollection());
    KShortcut findShortcut = KStandardShortcut::find();
    a->setShortcut(findShortcut);

    KStandardAction::findNext(m_findBar, SLOT(findNext()), actionCollection());
    KStandardAction::findPrev(m_findBar, SLOT(findPrevious()), actionCollection());

    a = KStandardAction::redisplay(_tab->view(), SLOT(reload()), actionCollection());
    a->setText(i18n("Reload"));
    KShortcut reloadShortcut = KStandardShortcut::reload();
    reloadShortcut.setAlternate(Qt::CTRL + Qt::Key_R);
    a->setShortcut(reloadShortcut);

    a = new KAction(KIcon("process-stop"), i18n("&Stop"), this);
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_Period));
    actionCollection()->addAction(QL1S("stop"), a);
    connect(a, SIGNAL(triggered(bool)), _tab->view(), SLOT(stop()));

    // Open location action
    a = new KAction(i18n("Open Location"), this);
    KShortcut openLocationShortcut(Qt::CTRL + Qt::Key_L);
    openLocationShortcut.setAlternate(Qt::ALT + Qt::Key_D);
    a->setShortcut(openLocationShortcut);
    actionCollection()->addAction(QL1S("open_location"), a);
    connect(a, SIGNAL(triggered(bool)) , this, SLOT(openLocation()));

    // ===== Tools Actions =================================
    a = new KAction(i18n("View Page S&ource"), this);
    a->setIcon(KIcon("application-xhtml+xml"));
    a->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
    actionCollection()->addAction(QL1S("page_source"), a);
    connect(a, SIGNAL(triggered(bool)), this, SLOT(viewPageSource()));

    a = new KAction(KIcon("view-media-artist"), i18n("New Private Window"), this);
    a->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    connect(a, SIGNAL(triggered(bool)), rApp, SLOT(newPrivateBrowsingWindow()));
    actionCollection()->addAction(QL1S("private_browsing"), a);

    a = new KAction(KIcon("edit-clear"), i18n("Clear Private Data..."), this);
    a->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Delete);
    actionCollection()->addAction(QL1S("clear_private_data"), a);
    connect(a, SIGNAL(triggered(bool)), rApp, SLOT(clearPrivateData()));

    // Zoom ==============
    KStandardAction::zoomIn(_tab, SLOT(zoomIn()), actionCollection());
    KStandardAction::zoomOut(_tab, SLOT(zoomOut()), actionCollection());
    a = KStandardAction::zoom(_tab, SLOT(zoomDefault()), actionCollection());
    a->setShortcut(KShortcut(Qt::CTRL | Qt::Key_0));

    // Bookmark ==========
    a = KStandardAction::addBookmark(_bar, SLOT(manageBookmarks()), actionCollection());
    KShortcut bkShortcut(Qt::CTRL + Qt::Key_D);
    a->setShortcut(bkShortcut);

    // Edit bookmarks
    a = KStandardAction::editBookmarks(BookmarkManager::self(), SLOT(slotEditBookmarks()), actionCollection());
    
    // Bookmark Menu
    KActionMenu *bmMenu = BookmarkManager::self()->bookmarkActionMenu(this);
    bmMenu->setIcon(KIcon("bookmarks"));
    bmMenu->setDelayed(false);
    bmMenu->setShortcutConfigurable(true);
    bmMenu->setShortcut(KShortcut(Qt::ALT + Qt::Key_B));
    actionCollection()->addAction(QL1S("bookmarksActionMenu"), bmMenu);

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
    connect(a, SIGNAL(triggered(bool)), AdBlockManager::self(), SLOT(showSettings()));

    // Web Applications
    a = new KAction(KIcon("applications-internet"), i18n("Create application shortcut"), this);
    actionCollection()->addAction(QL1S("webapp_shortcut"), a);
    connect(a, SIGNAL(triggered(bool)), rApp, SLOT(createWebAppShortcut()));

    // Sync action
    a = new KAction(KIcon("tools-wizard"), i18n("Sync"), this); // FIXME sync icon!!
    actionCollection()->addAction(QL1S("sync"), a);
    connect(a, SIGNAL(triggered(bool)), SyncManager::self(), SLOT(showSettings()));

    // web inspector
    a = new KAction(KIcon("layer-visible-on"), i18n("&Inspect"), this);
    a->setCheckable(true);
    actionCollection()->addAction(QL1S("web_inspector"), a);
    connect(a, SIGNAL(triggered(bool)), _tab, SLOT(toggleInspector(bool)));

    _ac->readSettings();
}


void WebWindow::setupTools()
{
    KActionMenu *toolsAction = new KActionMenu(KIcon("configure"), i18n("&Tools"), this);
    toolsAction->setDelayed(false);
    toolsAction->setShortcutConfigurable(true);
    toolsAction->setShortcut(KShortcut(Qt::ALT + Qt::Key_T));
    m_rekonqMenu = qobject_cast<RekonqMenu *>(RekonqFactory::createWidget(QL1S("rekonqMenu"), this));
    toolsAction->setMenu(m_rekonqMenu); // dummy menu to have the dropdown arrow

    // adding rekonq_tools to rekonq actionCollection
    actionCollection()->addAction(QL1S("rekonq_tools"), toolsAction);
}

// ---------------------------------------------------------------------------------------------------

KActionCollection *WebWindow::actionCollection() const
{
    return _ac;
}


QAction *WebWindow::actionByName(const QString &name)
{
    return actionCollection()->action(name);
}


void WebWindow::load(const QUrl &url)
{
    _tab->view()->load(url);
}


WebPage *WebWindow::page()
{
    return _tab->page();
}


void WebWindow::webLoadProgress(int p)
{
    _progress = p;
    emit loadProgress(p);
}


void WebWindow::webLoadStarted()
{
    emit loadStarted();

    m_loadStopReloadAction->setIcon(KIcon("process-stop"));
    m_loadStopReloadAction->setToolTip(i18n("Stop loading the current page"));
    m_loadStopReloadAction->setText(i18n("Stop"));
    connect(m_loadStopReloadAction, SIGNAL(triggered(bool)), _tab->view(), SLOT(stop()));

    updateHistoryActions();
}


void WebWindow::webLoadFinished(bool b)
{
    emit loadFinished(b);

    if (_bar->hasFocus())
    {
        urlbarFocused();
    }
    else
    {
        m_loadStopReloadAction->setIcon(KIcon("view-refresh"));
        m_loadStopReloadAction->setToolTip(i18n("Reload the current page"));
        m_loadStopReloadAction->setText(i18n("Reload"));
        connect(m_loadStopReloadAction, SIGNAL(triggered(bool)), _tab->view(), SLOT(reload()));
    }

    updateHistoryActions();
}


void WebWindow::urlbarFocused()
{
    m_loadStopReloadAction->setIcon(KIcon("go-jump-locationbar"));
    m_loadStopReloadAction->setToolTip(i18n("Go"));
    m_loadStopReloadAction->setText(i18n("Go"));
    connect(m_loadStopReloadAction, SIGNAL(triggered(bool)), _bar, SLOT(loadTypedUrl()));
}


void WebWindow::aboutToShowBackMenu()
{
    m_historyBackMenu->clear();

    QWebHistory *history = _tab->view()->history();
    int pivot = history->currentItemIndex();
    int offset = 0;
    const int maxItemNumber = 8;  // no more than 8 elements in the Back History Menu!
    QList<QWebHistoryItem> historyList = history->backItems(maxItemNumber);
    int listCount = historyList.count();
    if (pivot >= maxItemNumber)
        offset = pivot - maxItemNumber;

    if (_tab->page()->isOnRekonqPage())
    {
        QWebHistoryItem item = history->currentItem();
        KAction *action = new KAction(this);
        action->setData(listCount + offset++);
        KIcon icon = IconManager::self()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyBackMenu->addAction(action);
    }

    for (int i = listCount - 1; i >= 0; --i)
    {
        QWebHistoryItem item = historyList.at(i);
        KAction *action = new KAction(this);
        action->setData(i + offset);
        KIcon icon = IconManager::self()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyBackMenu->addAction(action);
    }
}


void WebWindow::aboutToShowForwardMenu()
{
    m_historyForwardMenu->clear();

    QWebHistory *history = _tab->view()->history();
    const int pivot = history->currentItemIndex();
    int offset = 0;
    const int maxItemNumber = 8;  // no more than 8 elements in the Forward History Menu!
    QList<QWebHistoryItem> historyList = history->forwardItems(maxItemNumber);
    int listCount = historyList.count();

    if (pivot >= maxItemNumber)
        offset = pivot - maxItemNumber;

    if (_tab->page()->isOnRekonqPage())
    {
        QWebHistoryItem item = history->currentItem();
        KAction *action = new KAction(this);
        action->setData(listCount + offset++);
        KIcon icon = IconManager::self()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyForwardMenu->addAction(action);
    }

    for (int i = 1; i <= listCount; i++)
    {
        QWebHistoryItem item = historyList.at(i - 1);
        KAction *action = new KAction(this);
        action->setData(pivot + i + offset);
        KIcon icon = IconManager::self()->iconForUrl(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyForwardMenu->addAction(action);
    }
}


void WebWindow::openActionUrl(QAction *action)
{
    int index = action->data().toInt();

    QWebHistory *history = _tab->view()->history();
    if (!history->itemAt(index).isValid())
    {
        kDebug() << "Invalid Index!: " << index;
        return;
    }

    history->goToItem(history->itemAt(index));
}


void WebWindow::openPrevious(Qt::MouseButtons mouseButtons, Qt::KeyboardModifiers keyboardModifiers)
{
    QWebHistory *history = _tab->view()->history();
    QWebHistoryItem *item = 0;

    if (_tab->page()->isOnRekonqPage())
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


void WebWindow::openNext(Qt::MouseButtons mouseButtons, Qt::KeyboardModifiers keyboardModifiers)
{
    QWebHistory *history = _tab->view()->history();
    QWebHistoryItem *item = 0;

    if (_tab->page()->isOnRekonqPage())
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


void WebWindow::updateHistoryActions()
{
    QWebHistory *history = _tab->view()->history();

    bool rekonqPage = _tab->page()->isOnRekonqPage();

    QAction *historyBackAction = actionByName(KStandardAction::name(KStandardAction::Back));
    if (rekonqPage && history->count() > 0)
        historyBackAction->setEnabled(true);
    else
        historyBackAction->setEnabled(history->canGoBack());

    QAction *historyForwardAction = actionByName(KStandardAction::name(KStandardAction::Forward));
    historyForwardAction->setEnabled(history->canGoForward());
}


void WebWindow::notifyMessage(const QString &msg)
{
    // deleting popus if empty msgs
    if (msg.isEmpty())
    {
        m_hidePopupTimer->start(250);
        return;
    }

    m_hidePopupTimer->stop();
    m_hidePopupTimer->start(3000);

    QString msgToShow = Qt::escape(msg);

    // fix crash on window close
    if (!_tab || !_tab->page())
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
    if (!_tab->page()->currentFrame())
        return;

    const bool horizontalScrollbarIsVisible = _tab->page()->currentFrame()->scrollBarMaximum(Qt::Horizontal);
    const bool verticalScrollbarIsVisible = _tab->page()->currentFrame()->scrollBarMaximum(Qt::Vertical);
    const bool actionBarsVisible = m_findBar->isVisible(); // NOTE: actually we have just the finbar down there...

    const int scrollbarExtent = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int hScrollbarSize = horizontalScrollbarIsVisible ? scrollbarExtent : 0;
    const int vScrollbarSize = verticalScrollbarIsVisible ? scrollbarExtent : 0;

    const QPoint mousePos = mapFromGlobal(QCursor::pos());
    const QPoint bottomPoint = mapTo(this, geometry().bottomLeft());

    int y = bottomPoint.y() + 1 - m_popup->height() - hScrollbarSize;   // +1 because bottom() returns top() + height() - 1, see QRect doku
    int x = QRect(QPoint(0, y), labelSize).contains(mousePos) || actionBarsVisible
            ? width() - labelSize.width() - vScrollbarSize
            : 0;

    m_popup->move(x, y);
    m_popup->show();
}


KUrl WebWindow::url() const
{
    return _tab->url();
}


QString WebWindow::title() const
{
    QString t = _tab->title();
    if (t.isEmpty())
    {
        if (url().isLocalFile())
            return url().fileName();
        else
            return url().prettyUrl();
    }

    return t;
}


UrlBar *WebWindow::urlBar()
{
    return _bar;
}


WebTab *WebWindow::tabView()
{
    return _tab;
}


QPixmap WebWindow::tabPreview(int width, int height)
{
    return WebSnap::renderPagePreview(*page(), width, height);
}


bool WebWindow::isLoading()
{
    return _progress != 0 && _progress != 100;
}


void WebWindow::fileOpen()
{
    QString filePath = KFileDialog::getOpenFileName(KUrl(),
                       i18n("*.html *.htm *.svg *.png *.gif *.svgz|Web Resources (*.html *.htm *.svg *.png *.gif *.svgz)\n"
                            "*.*|All files (*.*)"),
                       this,
                       i18n("Open Web Resource"));

    if (filePath.isEmpty())
        return;

    load(KUrl(filePath));
}


void WebWindow::fileSaveAs()
{
    KUrl srcUrl = url();

    if (page()->isOnRekonqPage())
    {
        KParts::ReadOnlyPart *p = _tab->part();
        if (p)
        {
            // if this is a KParts document then the w->url() will be empty and the srcUrl
            // must be set to the document url
            srcUrl = p->url();
        }
    }

    // First, try with suggested file name...
    QString name = page()->suggestedFileName();

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

    if (page()->isContentEditable())
    {
        QString code = page()->mainFrame()->toHtml();
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


void WebWindow::openLocation()
{
    _bar->selectAll();
    _bar->setFocus();
}


void WebWindow::viewPageSource()
{
    QString code = _tab->page()->mainFrame()->toHtml();

    KTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    if (!tmpFile.open())
        return;

    QTextStream out(&tmpFile);
    out << code;
    tmpFile.close();
    KUrl tmpUrl(tmpFile.fileName());

    KRun::runUrl(tmpUrl, QL1S("text/plain"), this, false);

//     FIXME: Implement "view-source" scheme
//     KParts::ReadOnlyPart *pa = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(QL1S("text/plain"), _tab, this, QString());
//     if (pa)
//     {
//         WebTab *srcTab = m_view->newWebTab(true);
//         srcTab->page()->setIsOnRekonqPage(true);
//         srcTab->setPart(pa, tmpUrl);
//         srcTab->urlBar()->setQUrl(url.pathOrUrl());
//         m_view->setTabText(m_view->currentIndex(), i18n("Source of: ") + url.prettyUrl());
//         updateHistoryActions();
//     }

}


void WebWindow::setWidgetsHidden(bool hide)
{
    // state flags
    static bool bookmarksToolBarFlag = false;

    if (hide)
    {
        // save current state, if in windowed mode
        if (!_bookmarksBar.isNull())
        {
            bookmarksToolBarFlag = true;
            _bookmarksBar.data()->hide();
        }

        // hide main toolbar
        _mainToolBar.data()->hide();
    }
    else
    {
        // show main toolbar
        _mainToolBar.data()->show();

        // restore state of windowed mode
        if (!_bookmarksBar.isNull() && bookmarksToolBarFlag)
            _bookmarksBar.data()->show();
    }
}


void WebWindow::populateUserAgentMenu()
{
    KMenu *uaMenu = static_cast<KMenu *>(QObject::sender());
    if (!uaMenu)
    {
        kDebug() << "oops... NO user agent menu";
        return;
    }

    UserAgentManager::self()->populateUAMenuForTabUrl(uaMenu, this);
}


void WebWindow::setEditable(bool on)
{
    page()->setContentEditable(on);
}


void WebWindow::preferences()
{
    // an instance the dialog could be already created and could be cached,
    // in which case you want to display the cached dialog
    if (SettingsDialog::showDialog("rekonfig"))
        return;

    // we didn't find an instance of this dialog, so lets create it
    QPointer<SettingsDialog> s = new SettingsDialog(this);

    // keep us informed when the user changes settings
    connect(s, SIGNAL(settingsChanged(QString)), rApp, SLOT(updateConfiguration()));
    connect(s, SIGNAL(finished(int)), s, SLOT(deleteLater()));

    s->show();
}


void WebWindow::toggleBookmarksToolbar(bool b)
{
    bool actual = !_bookmarksBar.isNull();
    if (actual == b)
        return;

    if (b)
    {
        _bookmarksBar = qobject_cast<BookmarkToolBar *>(RekonqFactory::createWidget(QL1S("bookmarkToolBar"), this));
        BookmarkManager::self()->registerBookmarkBar(_bookmarksBar.data());

        qobject_cast<QVBoxLayout *>(layout())->insertWidget(1, _bookmarksBar.data());
    }
    else
    {
        qobject_cast<QVBoxLayout *>(layout())->removeWidget(_bookmarksBar.data());

        BookmarkManager::self()->removeBookmarkBar(_bookmarksBar.data());
        delete _bookmarksBar.data();
        _bookmarksBar.clear();
    }

    ReKonfig::setShowBookmarksToolbar(b);
    QAction *a = actionByName(QL1S("show_bookmarks_toolbar"));
    a->setChecked(b);
    rApp->bookmarksToolbarToggled(b);
}


void WebWindow::showCrashMessageBar()
{
    _tab->showCrashMessageBar();
}


void WebWindow::openNewWindow()
{
    rApp->loadUrl(QUrl("about:home"), Rekonq::NewWindow);
}


void WebWindow::checkFocus()
{
    if (url().scheme() == QL1S("about"))
        _bar->setFocus();
    else
        _tab->view()->setFocus();
}


void WebWindow::openDownloadsPage()
{
    rApp->loadUrl(QUrl("about:downloads"), Rekonq::NewFocusedTab);
}


void WebWindow::openHistoryPage()
{
    rApp->loadUrl(QUrl("about:history"), Rekonq::NewFocusedTab);
}


void WebWindow::openBookmarksPage()
{
    rApp->loadUrl(QUrl("about:bookmarks"), Rekonq::NewFocusedTab);
}


void WebWindow::openHomePage(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    KUrl homeUrl = ReKonfig::useNewTabPage()
                   ? KUrl(QL1S("about:home"))
                   : KUrl(ReKonfig::homePage());

    if (buttons == Qt::MidButton || modifiers == Qt::ControlModifier)
        rApp->loadUrl(homeUrl, Rekonq::NewTab);
    else
        load(homeUrl);
}


void WebWindow::keyPressEvent(QKeyEvent *kev)
{
    if (kev->key() == Qt::Key_Escape)
    {
        // if findbar is visible, hide it
        if (m_findBar->isVisible())
        {
            m_findBar->hide();
            kev->accept();
            checkFocus();
            return;
        }
    }

    return QWidget::keyPressEvent(kev);
}


void WebWindow::keyBindings()
{
    QPointer<KShortcutsDialog> dialog = new KShortcutsDialog(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);

    dialog->addCollection(actionCollection(), i18n("web window"));
    TabWindow *tw = rApp->tabWindow();
    if (tw)
    {
        dialog->addCollection(tw->actionCollection(), i18n("tab window"));
    }

    dialog->configure();
    dialog->deleteLater();
}



void WebWindow::findSelectedText()
{
    // Just done everything! Just show findbar... ;)
    m_findBar->show();
}


// Main Toolbar methods ----------


void WebWindow::setupMainToolBar()
{
    RekonqFactory::updateWidget(_mainToolBar.data(), QL1S("mainToolBar"));
}


void WebWindow::showToolbarEditor()
{
    QPointer<KEditToolBar> ed = new KEditToolBar(actionCollection(), this);
    ed->setResourceFile( "rekonqui.rc" );
    connect(ed, SIGNAL(newToolBarConfig()),this, SLOT(setupMainToolBar()));

    ed->exec();
    ed->deleteLater();
}
