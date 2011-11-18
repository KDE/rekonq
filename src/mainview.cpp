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
#include "mainview.h"
#include "mainview.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "iconmanager.h"
#include "mainwindow.h"
#include "sessionmanager.h"
#include "stackedurlbar.h"
#include "tabbar.h"
#include "urlbar.h"
#include "webpage.h"
#include "webtab.h"

// KDE Includes
#include <KAction>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>

// Qt Includes
#include <QtGui/QLabel>
#include <QtGui/QMovie>
#include <QtGui/QToolButton>

//Hack:
QString temporaryUglyHackString = "";

MainView::MainView(QWidget *parent)
    : KTabWidget(parent)
    , m_widgetBar(new StackedUrlBar(this))
    , m_addTabButton(0)
    , m_currentTabIndex(0)
{
    // setting tabbar
    TabBar *tabBar = new TabBar(this);
    m_addTabButton = new QToolButton(this);
    setTabBar(tabBar);

    tabBar->show();
    // set mouse tracking for tab previews
    setMouseTracking(true);

    // loading pixmap path
    m_loadingGitPath = KStandardDirs::locate("appdata" , "pics/loading.mng");

    // connecting tabbar signals
    connect(tabBar, SIGNAL(closeTab(int)),          this,   SLOT(closeTab(int)));
    connect(tabBar, SIGNAL(mouseMiddleClick(int)),  this,   SLOT(closeTab(int)));
    connect(tabBar, SIGNAL(newTabRequest()),        this,   SLOT(newTab()));

    connect(tabBar, SIGNAL(cloneTab(int)),          this,   SLOT(cloneTab(int)));
    connect(tabBar, SIGNAL(closeOtherTabs(int)),    this,   SLOT(closeOtherTabs(int)));
    connect(tabBar, SIGNAL(reloadTab(int)),         this,   SLOT(reloadTab(int)));
    connect(tabBar, SIGNAL(reloadAllTabs()),        this,   SLOT(reloadAllTabs()));
    connect(tabBar, SIGNAL(detachTab(int)),         this,   SLOT(detachTab(int)));

    connect(tabBar, SIGNAL(tabCloseRequested(int)), this,   SLOT(closeTab(int)));
    connect(tabBar, SIGNAL(tabMoved(int, int)),     m_widgetBar, SLOT(moveBar(int, int)));

    // current page index changing
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
    connect(this, SIGNAL(currentChanged(int)), rApp->sessionManager(), SLOT(saveSession()));

    QList<TabHistory> list = rApp->sessionManager()->closedSites();
    Q_FOREACH(const TabHistory & tab, list)
    {
        if (tab.url.startsWith(QL1S("about")))
            continue;
        m_recentlyClosedTabs.removeAll(tab);
        m_recentlyClosedTabs.prepend(tab);
    }
}


void MainView::addNewTabButton(QAction *newTabAction)
{
    m_addTabButton->setDefaultAction(newTabAction);

    m_addTabButton->setAutoRaise(true);
    m_addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
}


TabBar *MainView::tabBar() const
{
    TabBar *tabBar = qobject_cast<TabBar *>(KTabWidget::tabBar());
    return tabBar;
}


UrlBar *MainView::currentUrlBar() const
{
    return currentWebTab()->urlBar();
}


WebTab *MainView::currentWebTab() const
{
    int i = currentIndex();
    WebTab *b = webTab(i);
    if (b)
        return b;

    kDebug() << "We failed to find the current tab!!! Let's go sure with the first one...";
    return webTab(0);
}


void MainView::updateTabBar()
{
    if (ReKonfig::alwaysShowTabBar() || count() > 1)
    {
        if (tabBar()->isHidden())
        {
            tabBar()->show();
        }

        // this to ensure tab button visibility also on new window creation
        if (m_addTabButton->isHidden())
        {
            m_addTabButton->show();
        }

    }
    else
    {
        tabBar()->hide();
        m_addTabButton->hide();
        return;
    }

    // update tab button position
    static bool ButtonInCorner = false;

    int tabWidgetWidth = frameSize().width();
    int tabBarWidth = tabBar()->tabSizeHint(0).width() * tabBar()->count();

    if (tabBarWidth + m_addTabButton->width() > tabWidgetWidth)
    {
        if (ButtonInCorner)
            return;
        setCornerWidget(m_addTabButton);
        ButtonInCorner = true;
    }
    else
    {
        if (ButtonInCorner)
        {
            setCornerWidget(0);
            ButtonInCorner = false;
        }

        m_addTabButton->move(tabBarWidth, 0);
    }
}


void MainView::webReload()
{
    reloadTab(currentIndex());
}


void MainView::webStop()
{
    WebTab *tabToStop = currentWebTab();
    QAction *action = tabToStop->view()->page()->action(QWebPage::Stop);
    action->trigger();
}


// When index is -1 index chooses the current tab
void MainView::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    WebTab *reloadingTab = webTab(index);
    if (reloadingTab->view()->url().scheme() != QL1S("about"))
    {
        QAction *action = reloadingTab->view()->page()->action(QWebPage::Reload);
        action->trigger();
    }
    else
    {
        reloadingTab->view()->setUrl(reloadingTab->page()->loadingUrl());
    }
}


// this slot is called on tab switching
void MainView::currentChanged(int index)
{
    // retrieve the webview related to the index
    WebTab *tab = this->webTab(index);
    if (!tab)
        return;

    // retrieve the old webview (that where we move from)
    WebTab *oldTab = this->webTab(m_currentTabIndex);

    // set current index
    m_currentTabIndex = index;

    if (oldTab)
    {
        // disconnecting webpage from mainview
        disconnect(oldTab->page(), SIGNAL(statusBarMessage(const QString&)),
                   this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldTab->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
                   this, SIGNAL(linkHovered(const QString&)));
    }

    connect(tab->page(), SIGNAL(statusBarMessage(const QString&)),
            this, SIGNAL(showStatusBarMessage(const QString&)));
    connect(tab->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
            this, SIGNAL(linkHovered(const QString&)));

    emit currentTitle(tab->view()->title());
    m_widgetBar->setCurrentIndex(index);

    // clean up "status bar"
    emit showStatusBarMessage(QString());

    // notify UI to eventually switch stop/reload button
    emit browserTabLoading(tab->isPageLoading());

    // set focus to the current webview
    if (tab->url().scheme() == QL1S("about"))
        m_widgetBar->currentWidget()->setFocus();
    else
        tab->view()->setFocus();

    tabBar()->resetTabHighlighted(index);
}


WebTab *MainView::webTab(int index) const
{
    WebTab *tab = qobject_cast<WebTab *>(this->widget(index));
    if (tab)
    {
        return tab;
    }

    kDebug() << "WebTab with index " << index << "not found. Returning NULL." ;
    return 0;
}


WebTab *MainView::newWebTab(bool focused)
{
    WebTab *tab = new WebTab(this);

    // connecting webview with mainview
    connect(tab->view(), SIGNAL(loadStarted()), this, SLOT(webViewLoadStarted()));
    connect(tab->view(), SIGNAL(loadFinished(bool)), this, SLOT(webViewLoadFinished(bool)));
    connect(tab, SIGNAL(titleChanged(const QString &)), this, SLOT(webViewTitleChanged(const QString &)));
    connect(tab->view(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(tab->view(), SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(tab->view(), SIGNAL(openPreviousInHistory()), this, SIGNAL(openPreviousInHistory()));
    connect(tab->view(), SIGNAL(openNextInHistory()), this, SIGNAL(openNextInHistory()));

    // connecting webPage signals with mainview
    connect(tab->page(), SIGNAL(windowCloseRequested()), this, SLOT(windowCloseRequested()));
    connect(tab->page(), SIGNAL(printRequested(QWebFrame *)), this, SIGNAL(printRequested(QWebFrame *)));

    if (ReKonfig::openTabsNearCurrent())
    {
        insertTab(currentIndex() + 1, tab, i18n("(Untitled)"));
        temporaryUglyHackString = tabText(currentIndex() + 1);
        m_widgetBar->insertWidget(currentIndex() + 1, tab->urlBar());
    }
    else
    {
        addTab(tab, i18n("(Untitled)"));
        temporaryUglyHackString = tabText(count() - 1);
        m_widgetBar->addWidget(tab->urlBar());
    }
    updateTabBar();

    if (focused)
    {
        setCurrentWidget(tab);
    }

    return tab;
}


void MainView::newTab()
{
    WebView *w = newWebTab()->view();

    currentUrlBar()->setFocus();

    switch (ReKonfig::newTabsBehaviour())
    {
    case 0: // new tab page
        w->load(KUrl("about:home"));
        break;
    case 1: // blank page
        currentUrlBar()->clear();
        break;
    case 2: // homepage
        w->load(KUrl(ReKonfig::homePage()));
        break;
    default:
        break;
    }
}


void MainView::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i)
    {
        reloadTab(i);
    }
}


void MainView::windowCloseRequested()
{
    WebPage *page = qobject_cast<WebPage *>(sender());
    WebView *view = qobject_cast<WebView *>(page->view());
    int index = indexOf(view->parentWidget());

    if (index >= 0)
    {
        if (count() == 1)
        {
            MainWindow *w = qobject_cast<MainWindow *>(parent());
            w->close();
        }
        else
        {
            closeTab(index);
        }
        return;
    }
}


void MainView::closeOtherTabs(int index)
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


void MainView::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    KUrl url = webTab(index)->url();

    rApp->loadUrl(url, Rekonq::NewTab);
}


// When index is -1 index chooses the current tab
void MainView::closeTab(int index, bool del)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    if (count() == 1)
    {
        if (ReKonfig::lastTabClosesWindow())
        {
            // closing window...
            MainWindow *w = qobject_cast<MainWindow *>(parent());
            w->close();
            return;
        }

        // open default homePage if just one tab is opened
        WebView *w = currentWebTab()->view();

        if (currentWebTab()->url().protocol() == QL1S("about"))
            return;

        switch (ReKonfig::newTabsBehaviour())
        {
        case 0: // new tab page
        case 1: // blank page
            w->load(KUrl("about:home"));
            currentUrlBar()->setFocus();
            break;
        case 2: // homepage
            w->load(KUrl(ReKonfig::homePage()));
            break;
        default:
            break;
        }
        return;
    }

    WebTab *tabToClose = webTab(index);
    if (!tabToClose)
        return;

    if (tabToClose->view()->isModified())
    {
        int risp = KMessageBox::warningContinueCancel(this,
                   i18n("This tab contains changes that have not been submitted.\n"
                        "Closing the tab will discard these changes.\n"
                        "Do you really want to close this tab?\n"),
                   i18n("Closing Modified Tab"), KGuiItem(i18n("Close &Tab"), "tab-close"), KStandardGuiItem::cancel());
        if (risp != KMessageBox::Continue)
            return;
    }

    if (!tabToClose->url().isEmpty()
            && !QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled)
       )
    {
        const int recentlyClosedTabsLimit = 10;
        TabHistory history(tabToClose->view()->history());

        m_recentlyClosedTabs.removeAll(history);
        if (m_recentlyClosedTabs.count() == recentlyClosedTabsLimit)
            m_recentlyClosedTabs.removeLast();
        m_recentlyClosedTabs.prepend(history);
    }

    removeTab(index);
    updateTabBar();        // UI operation: do it ASAP!!

    m_widgetBar->removeWidget(tabToClose->urlBar());
    m_widgetBar->setCurrentIndex(m_currentTabIndex);

    if (del)
    {
        tabToClose->deleteLater();
    }
}

void MainView::webViewLoadStarted()
{
    WebView *view = qobject_cast<WebView *>(sender());
    int index = indexOf(view->parentWidget());
    if (-1 != index)
    {
        QLabel *label = animatedLoading(index, true);
        if (label->movie())
        {
            label->movie()->start();
        }
    }

    if (index != currentIndex())
        return;

    emit browserTabLoading(true);
    emit showStatusBarMessage(i18n("Loading..."), Rekonq::Info);

    if (view == currentWebTab()->view() && view->url().scheme() != QL1S("about"))
    {
        view->setFocus();
    }
}


void MainView::webViewLoadFinished(bool ok)
{
    WebView *view = qobject_cast<WebView *>(sender());
    int index = -1;
    if (view)
        index = indexOf(view->parentWidget());

    if (-1 != index)
    {
        QLabel *label = animatedLoading(index, true);
        QMovie *movie = label->movie();
        if (movie)
            movie->stop();
    }

    webViewIconChanged();
    emit browserTabLoading(false);

    // don't display messages for background tabs
    if (index != currentIndex())
    {
        return;
    }

    if (ok)
        emit showStatusBarMessage(i18n("Done"), Rekonq::Info);
//     else
//         emit showStatusBarMessage(i18n("Failed to load"), Rekonq::Error);
}


void MainView::webViewIconChanged()
{
    WebView *view = qobject_cast<WebView *>(sender());
    WebTab *tab = qobject_cast<WebTab *>(view->parent());
    int index = indexOf(tab);

    if (-1 != index)
    {
        KIcon icon = rApp->iconManager()->iconForUrl(tab->url());
        QLabel *label = animatedLoading(index, false);
        QMovie *movie = label->movie();
        delete movie;
        label->setMovie(0);
        label->setPixmap(icon.pixmap(16, 16));
    }
}


void MainView::webViewTitleChanged(const QString &title)
{
    QString viewTitle = title.isEmpty() ? i18n("(Untitled)") : title;
    QString tabTitle = viewTitle;
    tabTitle.replace('&', "&&");

    WebTab *tab = qobject_cast<WebTab *>(sender());
    int index = indexOf(tab);
    if (-1 != index)
    {
        setTabText(index, tabTitle);
    }
    if (currentIndex() == index)
    {
        emit currentTitle(viewTitle);
    }
    else
    {
        if (tabTitle != temporaryUglyHackString)
            tabBar()->setTabHighlighted(index);
    }
    rApp->historyManager()->updateHistoryEntry(tab->url(), tabTitle);
    if (ReKonfig::hoveringTabOption() == 1)
        tabBar()->setTabToolTip(index, tabTitle.remove('&'));
}


void MainView::webViewUrlChanged(const QUrl &url)
{
    WebView *view = qobject_cast<WebView *>(sender());
    int index = indexOf(view->parentWidget());
    if (-1 != index)
    {
        tabBar()->setTabData(index, url);
    }
    if (ReKonfig::hoveringTabOption() == 2)
        tabBar()->setTabToolTip(index, webTab(index)->url().toMimeDataString());
}


void MainView::nextTab()
{
    int next = currentIndex() + 1;
    if (next == count())
        next = 0;
    setCurrentIndex(next);
}


void MainView::previousTab()
{
    int next = currentIndex() - 1;
    if (next < 0)
        next = count() - 1;
    setCurrentIndex(next);
}


void MainView::openLastClosedTab()
{
    if (m_recentlyClosedTabs.isEmpty())
        return;

    TabHistory history = m_recentlyClosedTabs.takeFirst();
    WebView *view = rApp->mainWindow()->mainView()->newWebTab()->view();

    history.applyHistory(view->history());
}


void MainView::openClosedTab()
{
    KAction *action = qobject_cast<KAction *>(sender());
    if (action)
    {
        WebView *view = rApp->mainWindow()->mainView()->newWebTab()->view();
        TabHistory history;
        Q_FOREACH(const TabHistory & item, m_recentlyClosedTabs)
        {
            if (item.history == action->data().toByteArray())
            {
                history = item;
                break;
            }
        }
        history.applyHistory(view->history());

        m_recentlyClosedTabs.removeAll(history);
    }
}


void MainView::switchToTab(const int index)
{
    if (index <= 0 || index > count())
        return;
    setCurrentIndex(index - 1);
}


void MainView::loadFavorite(const int index)
{
    QStringList urls = ReKonfig::previewUrls();
    if (index < 0 || index > urls.length())
        return;
    KUrl url = KUrl(urls.at(index - 1));
    rApp->loadUrl(url);
    currentWebTab()->setFocus();
}


QLabel *MainView::animatedLoading(int index, bool addMovie)
{
    if (index == -1)
        return 0;

    QLabel *label = qobject_cast<QLabel* >(tabBar()->tabButton(index, QTabBar::LeftSide));
    if (!label)
    {
        label = new QLabel(this);
    }
    if (addMovie && !label->movie())
    {
        QMovie *movie = new QMovie(m_loadingGitPath, QByteArray(), label);
        movie->setSpeed(50);
        label->setMovie(movie);
        movie->start();
    }
    tabBar()->setTabButton(index, QTabBar::LeftSide, 0);
    tabBar()->setTabButton(index, QTabBar::LeftSide, label);
    return label;
}


void MainView::resizeEvent(QResizeEvent *event)
{
    updateTabBar();
    KTabWidget::resizeEvent(event);
}


void MainView::detachTab(int index, MainWindow *toWindow)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    WebTab *tab = webTab(index);
    KUrl u = tab->url();
    if (u.scheme() == QL1S("about"))
    {
        closeTab(index);
        rApp->loadUrl(u, Rekonq::NewWindow);
    }
    else
    {
        QString label = tab->view()->title();
        UrlBar *bar = tab->urlBar();
        closeTab(index, false);

        MainWindow *w;
        if (toWindow == NULL)
            w = rApp->newMainWindow(false);
        else
            w = toWindow;
        w->mainView()->addTab(tab, rApp->iconManager()->iconForUrl(u), label);
        w->mainView()->widgetBar()->insertWidget(0, bar);
        w->mainView()->updateTabBar();
    }
}
