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

#include "bookmarkmanager.h"
#include "iconmanager.h"

#include "webpage.h"
#include "webtab.h"

#include "bookmarkstoolbar.h"
#include "rekonqmenu.h"
#include "urlbar.h"

#include "websnap.h"

#include <KUrl>
#include <KToolBar>

#include <QLabel>
#include <QStyle>
#include <QTextDocument>
#include <QTimer>
#include <QWebFrame>
#include <QWebView>
#include <QWebHistory>
#include <QVBoxLayout>


WebWindow::WebWindow(QWidget *parent, WebPage *pg)
    : QWidget(parent)
    , _tab(new WebTab(this))
    , _bar(new UrlBar(_tab))
    , _mainToolBar(new KToolBar(this, false, false))
    , _bookmarksBar(0)
    , m_loadStopReloadAction(0)
    , m_rekonqMenu(0)
    , m_popup(new QLabel(this))
    , m_hidePopupTimer(new QTimer(this))
    , _ac(new KActionCollection(this))
{
    if (pg)
    {
        _tab->view()->setPage(pg);
        pg->setParent(_tab->view());
    }
    
    // then, setup our actions
    setupActions();

    // setting up rekonq tools
    setupTools();

    // main toolbar
    _mainToolBar->addAction(actionByName(QL1S("go_back")));
    _mainToolBar->addAction(actionByName(QL1S("go_forward")));
    _mainToolBar->addAction(actionByName(QL1S("url_bar")));
    _mainToolBar->addAction(actionByName(QL1S("load_stop_reload")));
    _mainToolBar->addAction(actionByName(QL1S("rekonq_tools")));

    // bookmarks toolbar
    if (_bookmarksBar)
    {
        BookmarkManager::self()->removeBookmarkBar(_bookmarksBar);
        delete _bookmarksBar;
    }
    KToolBar *XMLGUIBkBar = new KToolBar(this);
    _bookmarksBar = new BookmarkToolBar(XMLGUIBkBar, this);
    BookmarkManager::self()->registerBookmarkBar(_bookmarksBar);

    // layout
    QVBoxLayout *l = new QVBoxLayout(this);
    l->addWidget(_mainToolBar);
    l->addWidget(XMLGUIBkBar);
    l->addWidget(_tab);
    l->setContentsMargins(0, 0, 0, 0);

    setContentsMargins(0, 0, 0, 0);

    // things changed signals
    connect(_tab->view(), SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));

    // check view signals
    connect(_tab->view(), SIGNAL(loadStarted()), this, SLOT(webLoadStarted()));
    connect(_tab->view(), SIGNAL(loadFinished(bool)), this, SLOT(webLoadFinished(bool)));
    connect(_tab->view(), SIGNAL(loadProgress(int)), this, SLOT(webLoadProgress(int)));

    // page signals
    connect(page(), SIGNAL(pageCreated(WebPage *)), this, SIGNAL(pageCreated(WebPage *)));

    // message popup
    m_popup->setAutoFillBackground(true);
    m_popup->setMargin(4);
    m_popup->raise();
    m_popup->hide();
    connect(m_hidePopupTimer, SIGNAL(timeout()), m_popup, SLOT(hide()));
    connect(_tab->page(), SIGNAL(linkHovered(QString,QString,QString)), this, SLOT(notifyMessage(QString)));
}


void WebWindow::setupActions()
{
    KAction *a;
    
    // ========================= History related actions ==============================
    a = actionCollection()->addAction(KStandardAction::Back);
    connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)),
            this, SLOT(openPrevious(Qt::MouseButtons, Qt::KeyboardModifiers)));

    m_historyBackMenu = new KMenu(this);
    a->setMenu(m_historyBackMenu);
    connect(m_historyBackMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowBackMenu()));
    connect(m_historyBackMenu, SIGNAL(triggered(QAction*)), this, SLOT(openActionUrl(QAction*)));

    a = actionCollection()->addAction(KStandardAction::Forward);
    connect(a, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)),
            this, SLOT(openNext(Qt::MouseButtons, Qt::KeyboardModifiers)));

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

    m_loadStopReloadAction->setIcon(KIcon("go-jump-locationbar"));
    m_loadStopReloadAction->setToolTip(i18n("Go"));
    m_loadStopReloadAction->setText(i18n("Go"));

    updateHistoryActions();
}


void WebWindow::setupTools()
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

    m_loadStopReloadAction->setIcon(KIcon("view-refresh"));
    m_loadStopReloadAction->setToolTip(i18n("Reload the current page"));
    m_loadStopReloadAction->setText(i18n("Reload"));
    connect(m_loadStopReloadAction, SIGNAL(triggered(bool)), _tab->view(), SLOT(reload()));

    updateHistoryActions();
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
// FIXME        rApp->loadUrl(item->url(), Rekonq::NewTab);
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
// FIXME        rApp->loadUrl(item->url(), Rekonq::NewTab);
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
    const bool actionBarsVisible = false; //m_findBar->isVisible() || m_zoomBar->isVisible();

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
    return _tab->view()->title();
}


QIcon WebWindow::icon() const
{
    return _tab->view()->icon();
}


UrlBar *WebWindow::urlBar()
{
    return _bar;
}


WebTab *WebWindow::view()
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
