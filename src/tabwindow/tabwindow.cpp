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


// Self Includes
#include "tabwindow.h"
#include "tabwindow.moc"

// Local Includes
#include "application.h"
#include "webpage.h"
#include "webwindow.h"
#include "tabbar.h"

#include "tabhistory.h"

#include "iconmanager.h"

// KDE Includes
#include <KAction>
#include <KApplication>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KUrl>
#include <KToggleFullScreenAction>

#include <KWindowInfo>
#include <KWindowSystem>

// Qt Includes
#include <QDesktopWidget>
#include <QLabel>
#include <QMovie>
#include <QTabBar>
#include <QToolButton>

#include <QWebHistory>
#include <QWebSettings>


TabWindow::TabWindow(bool withTab, bool PrivateBrowsingMode, QWidget *parent)
    : RekonqWindow(parent)
    , _addTabButton(new QToolButton(this))
    , _openedTabsCounter(0)
    , _isPrivateBrowsing(PrivateBrowsingMode)
{
    setContentsMargins(0, 0, 0, 0);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // set mouse tracking for tab previews
    setMouseTracking(true);

    // setting tabbar
    TabBar *tabBar = new TabBar(this);
    setTabBar(tabBar);

    // connecting tabbar signals
    connect(tabBar, SIGNAL(tabCloseRequested(int)), this,   SLOT(closeTab(int)));
    connect(tabBar, SIGNAL(mouseMiddleClick(int)),  this,   SLOT(closeTab(int)));

    connect(tabBar, SIGNAL(newTabRequest()),        this,   SLOT(newCleanTab()));

    connect(tabBar, SIGNAL(cloneTab(int)),          this,   SLOT(cloneTab(int)));
    connect(tabBar, SIGNAL(closeTab(int)),          this,   SLOT(closeTab(int)));
    connect(tabBar, SIGNAL(closeOtherTabs(int)),    this,   SLOT(closeOtherTabs(int)));
    connect(tabBar, SIGNAL(reloadTab(int)),         this,   SLOT(reloadTab(int)));
    connect(tabBar, SIGNAL(detachTab(int)),         this,   SLOT(detachTab(int)));
    connect(tabBar, SIGNAL(restoreClosedTab(int)),  this,   SLOT(restoreClosedTab(int)));

    connect(tabBar, SIGNAL(tabLayoutChanged()),     this,   SLOT(updateNewTabButtonPosition()));

    // new tab button
    KAction* a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    _addTabButton->setDefaultAction(a);
    _addTabButton->setAutoRaise(true);
    _addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(_addTabButton, SIGNAL(triggered(QAction *)), this, SLOT(newCleanTab()));

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    // NOTE: we usually create TabWindow with AT LEAST one tab, but
    // in one important case...
    if (withTab)
    {
        WebWindow *tab = prepareNewTab();
        addTab(tab, i18n("new tab"));
        setCurrentWidget(tab);
    }
}


TabBar *TabWindow::tabBar() const
{
    TabBar *tabBar = qobject_cast<TabBar *>(QTabWidget::tabBar());
    return tabBar;
}


WebWindow *TabWindow::currentWebWindow() const
{
    return webWindow(currentIndex());
}


WebWindow *TabWindow::webWindow(int index) const
{
    WebWindow *tab = qobject_cast<WebWindow *>(this->widget(index));
    if (tab)
    {
        return tab;
    }

    kDebug() << "WebWindow with index " << index << "not found. Returning NULL." ;
    return 0;
}


WebWindow *TabWindow::prepareNewTab(WebPage *page)
{
    WebWindow *tab = new WebWindow(this, page);

    if (_isPrivateBrowsing)
        tab->setPrivateBrowsing(true);

    connect(tab, SIGNAL(titleChanged(QString)), this, SLOT(tabTitleChanged(QString)));

    connect(tab, SIGNAL(loadStarted()), this, SLOT(tabLoadStarted()));
    connect(tab, SIGNAL(loadFinished(bool)), this, SLOT(tabLoadFinished(bool)));

    connect(tab, SIGNAL(pageCreated(WebPage *)), this, SLOT(pageCreated(WebPage *)));

    connect(tab, SIGNAL(setFullScreen(bool)), this, SLOT(setFullScreen(bool)));
    
    return tab;
}


void TabWindow::loadUrl(const KUrl &url, Rekonq::OpenType type, TabHistory *history)
{
    WebWindow *tab = 0;
    switch (type)
    {
    case Rekonq::NewTab:
    case Rekonq::NewBackGroundTab:
        tab = prepareNewTab();
        addTab(tab, i18n("new tab"));
        break;

    case Rekonq::NewFocusedTab:
        tab = prepareNewTab();
        addTab(tab, i18n("new tab"));
        setCurrentWidget(tab);
        break;

    case Rekonq::NewWindow:
    case Rekonq::NewPrivateWindow:
        rApp->loadUrl(url, type);
        return;

    case Rekonq::CurrentTab:
    default:
        tab = currentWebWindow();
        break;
    };

    tab->load(url);

    if (history)
    {
        history->applyHistory(tab->page()->history());
    }
}


void TabWindow::newCleanTab()
{
    loadUrl(QUrl("about:home"), Rekonq::NewFocusedTab);
}


void TabWindow::pageCreated(WebPage *page)
{
    WebWindow *tab = prepareNewTab(page);

    // Now, the dirty jobs...
    _openedTabsCounter++;
    insertTab(currentIndex() + _openedTabsCounter, tab, i18n("new tab"));
}


void TabWindow::currentChanged(int newIndex)
{
    _openedTabsCounter = 0;

    tabBar()->setTabHighlighted(newIndex, false);

    // update window title & icon
    WebWindow* tab = webWindow(newIndex);
    if (!tab)
        return;

    QString t = tab->title();
    (t.isEmpty())
    ? setWindowTitle(QL1S("rekonq"))
    : setWindowTitle(t + QL1S(" - rekonq"));
}


void TabWindow::updateNewTabButtonPosition()
{
    int tabWidgetWidth = frameSize().width();
    int tabBarWidth = tabBar()->sizeHint().width();

    if (tabBarWidth + _addTabButton->width() > tabWidgetWidth)
    {
        setCornerWidget(_addTabButton);
    }
    else
    {
        setCornerWidget(0);
        _addTabButton->move(tabBarWidth, 0);
    }

    _addTabButton->show();
}


void TabWindow::tabTitleChanged(const QString &title)
{
    WebWindow *tab = qobject_cast<WebWindow *>(sender());
    if (!tab)
        return;

    bool emptyTitle = title.isEmpty();

    QString tabTitle = emptyTitle ? tab->url().url() : title;
    tabTitle.replace('&', "&&");

    int index = indexOf(tab);
    if (-1 != index && !tabBar()->tabData(index).toBool())
    {
        setTabText(index, tabTitle);
    }

    if (currentIndex() != index)
    {
        if (!emptyTitle)
            tabBar()->setTabHighlighted(index, true);
    }
    else
    {
        emptyTitle
        ? setWindowTitle(QL1S("rekonq"))
        : setWindowTitle(tabTitle + QL1S(" - rekonq"));
    }
}


void TabWindow::tabLoadStarted()
{
    WebWindow *tab = qobject_cast<WebWindow *>(sender());
    if (!tab)
        return;

    int index = indexOf(tab);
    if (index != -1)
    {
        QLabel *label = qobject_cast<QLabel* >(tabBar()->tabButton(index, QTabBar::LeftSide));
        if (!label)
        {
            label = new QLabel(this);
        }

        if (!label->movie())
        {
            static QString loadingGitPath = KStandardDirs::locate("appdata" , "pics/loading.mng");

            QMovie *movie = new QMovie(loadingGitPath, QByteArray(), label);
            movie->setSpeed(50);
            label->setMovie(movie);
            movie->start();
        }

        tabBar()->setTabButton(index, QTabBar::LeftSide, 0);
        tabBar()->setTabButton(index, QTabBar::LeftSide, label);

        if (!tabBar()->tabData(index).toBool())
            tabBar()->setTabText(index, i18n("Loading..."));
    }
}


void TabWindow::tabLoadFinished(bool ok)
{
    Q_UNUSED(ok);

    WebWindow *tab = qobject_cast<WebWindow *>(sender());
    if (!tab)
        return;

    int index = indexOf(tab);

    if (-1 == index)
        return;

    QLabel *label = qobject_cast<QLabel* >(tabBar()->tabButton(index, QTabBar::LeftSide));

    QMovie *movie = label->movie();
    movie->stop();
    delete movie;

    label->setMovie(0);

    KIcon ic = IconManager::self()->iconForUrl(tab->url());
    label->setPixmap(ic.pixmap(16, 16));

    if (!tabBar()->tabData(index).toBool())
        setTabText(index, tab->title());
}


void TabWindow::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    QUrl u = webWindow(index)->url();
    QWebHistory* history = webWindow(index)->page()->history();
    TabHistory tHistory(history);

    loadUrl(u, Rekonq::NewTab, &tHistory);
}


void TabWindow::closeTab(int index, bool del)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    WebWindow *tabToClose = webWindow(index);
    if (!tabToClose)
        return;

    // what to do if there is just one tab...
    if (count() == 1)
    {
        kDebug() << "CANNOT CLOSE WINDOW FROM HERE...";
        QUrl u = QUrl::fromUserInput("/DATI/WEBPAGES/HomePage/index.htm");
        currentWebWindow()->load(u);
        return;
    }

    if (!tabToClose->url().isEmpty()
            && tabToClose->url().scheme() != QL1S("about")
            && !tabToClose->page()->settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled)
       )
    {
        const int recentlyClosedTabsLimit = 8;
        TabHistory history(tabToClose->page()->history());
        history.title = tabToClose->title();
        history.url = tabToClose->url().url();

        m_recentlyClosedTabs.removeAll(history);
        if (m_recentlyClosedTabs.count() == recentlyClosedTabsLimit)
            m_recentlyClosedTabs.removeLast();
        m_recentlyClosedTabs.prepend(history);
    }

    removeTab(index);

    if (del)
    {
        tabToClose->deleteLater();
    }
}


void TabWindow::closeOtherTabs(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    for (int i = count() - 1; i > index; --i)
    {
        closeTab(i);
    }

    for (int i = index - 1; i >= 0; --i)
    {
        closeTab(i);
    }
}


void TabWindow::detachTab(int index, TabWindow *toWindow)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    WebWindow *tab = webWindow(index);
    KUrl u = tab->url();
    if (u.scheme() == QL1S("about"))
    {
        closeTab(index);
        loadUrl(u, Rekonq::NewWindow);
        return;
    }
    // else

    closeTab(index, false);

    TabWindow *w = 0;
    w = (toWindow == 0)
        ? new TabWindow(false)
        : toWindow;

    w->addTab(tab, tab->title());
    w->setCurrentWidget(tab);

    // disconnect signals from old tabwindow
    // WARNING: Code copied from prepareNewTab method.
    // Any new changes there should be applied here...
    disconnect(tab, SIGNAL(titleChanged(QString)), this, SLOT(tabTitleChanged(QString)));
    disconnect(tab, SIGNAL(loadStarted()), this, SLOT(tabLoadStarted()));
    disconnect(tab, SIGNAL(loadFinished(bool)), this, SLOT(tabLoadFinished(bool)));
    disconnect(tab, SIGNAL(pageCreated(WebPage *)), this, SLOT(pageCreated(WebPage *)));

    // reconnect signals to new tabwindow
    // WARNING: Code copied from prepareNewTab method.
    // Any new changes there should be applied here...
    connect(tab, SIGNAL(titleChanged(QString)), w, SLOT(tabTitleChanged(QString)));
    connect(tab, SIGNAL(loadStarted()), w, SLOT(tabLoadStarted()));
    connect(tab, SIGNAL(loadFinished(bool)), w, SLOT(tabLoadFinished(bool)));
    connect(tab, SIGNAL(pageCreated(WebPage *)), w, SLOT(pageCreated(WebPage *)));

    w->show();
}


void TabWindow::reloadTab(int index)
{
    // When index is -1 index chooses the current tab
    if (index < 0)
        index = currentIndex();

    if (index < 0 || index >= count())
        return;

    WebWindow *reloadingTab = webWindow(index);
    QAction *action = reloadingTab->page()->action(QWebPage::Reload);
    action->trigger();
}


void TabWindow::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i)
    {
        reloadTab(i);
    }
}


void TabWindow::restoreClosedTab(int i)
{
    if (m_recentlyClosedTabs.isEmpty())
        return;

    TabHistory history = m_recentlyClosedTabs.takeAt(i);

    QUrl u = QUrl(history.url);

    loadUrl(u, Rekonq::NewTab, &history);

    // just to get sure...
    m_recentlyClosedTabs.removeAll(history);
}


void TabWindow::setFullScreen(bool makeFullScreen)
{
    tabBar()->setVisible(!makeFullScreen);
    _addTabButton->setVisible(!makeFullScreen);
    KToggleFullScreenAction::setFullScreen(this, makeFullScreen);
}


bool TabWindow::isPrivateBrowsingWindowMode()
{
    return _isPrivateBrowsing;
}
