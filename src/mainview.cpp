/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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


MainView::MainView(MainWindow *parent)
        : KTabWidget(parent)
        , m_widgetBar(new StackedUrlBar(this))
        , m_addTabButton(0)
        , m_currentTabIndex(0)
        , m_parentWindow(parent)
{
    // setting tabbar
    TabBar *tabBar = new TabBar(this);
    m_addTabButton = new QToolButton(this);
    setTabBar(tabBar);

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

    QTimer::singleShot(0, this, SLOT(postLaunch()));
}


void MainView::postLaunch()
{
    QStringList list = Application::sessionManager()->closedSites();
    Q_FOREACH(const QString &line, list)
    {
        if(line.startsWith( QL1S("about") ))
            break;
        QString title = line;
        QString url = title;
        HistoryItem item(url, QDateTime(), title);
        m_recentlyClosedTabs.removeAll(item);
        m_recentlyClosedTabs.prepend(item);
    }

    // Session Manager
    connect(this, SIGNAL(tabsChanged()), Application::sessionManager(), SLOT(saveSession()));
    connect(this, SIGNAL(currentChanged(int)), Application::sessionManager(), SLOT(saveSession()));

    m_addTabButton->setDefaultAction(m_parentWindow->actionByName("new_tab"));

    m_addTabButton->setAutoRaise(true);
    m_addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
}


void MainView::updateTabButtonPosition()
{
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
            m_addTabButton->show();
            ButtonInCorner = false;
        }

        // detecting X position
        int newPosX = tabBarWidth;
        int tabWidthHint = tabBar()->tabSizeHint(0).width();
        if (tabWidthHint < sizeHint().width() / 4)
            newPosX = tabWidgetWidth - m_addTabButton->width();

        m_addTabButton->move(newPosX, 0);
    }
}


TabBar *MainView::tabBar() const
{
    TabBar *tabBar = qobject_cast<TabBar *>(KTabWidget::tabBar());
    return tabBar;
}


UrlBar *MainView::currentUrlBar() const
{
    return webTab(currentIndex())->urlBar();
}


WebTab *MainView::currentWebTab() const
{
    return webTab(currentIndex());
}


void MainView::updateTabBar()
{
    if (ReKonfig::alwaysShowTabBar() || tabBar()->count() > 1)
    {
        if (tabBar()->isHidden())
        {
            tabBar()->show();
            m_addTabButton->show();
        }
    }
    else
    {
        tabBar()->hide();
        m_addTabButton->hide();
    }

    updateTabButtonPosition();
}


void MainView::webReload()
{
    WebTab *webTab = currentWebTab();
    QAction *action = webTab->view()->page()->action(QWebPage::Reload);
    action->trigger();
}


void MainView::webStop()
{
    WebTab *webTab = currentWebTab();
    QAction *action = webTab->view()->page()->action(QWebPage::Stop);
    action->trigger();
}


// When index is -1 index chooses the current tab
void MainView::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    webTab(index)->view()->reload();
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

    if ( ReKonfig::openTabsNearCurrent() )
    {
        insertTab(currentIndex() + 1, tab, i18n("(Untitled)"));
        m_widgetBar->insertWidget(currentIndex() + 1, tab->urlBar());
    }
    else
    {
        addTab(tab, i18n("(Untitled)"));
        m_widgetBar->addWidget(tab->urlBar());
    }
    updateTabBar();

    if (focused)
    {
        setCurrentWidget(tab);
    }

    emit tabsChanged();

    return tab;
}


void MainView::newTab()
{
    WebView *w = newWebTab()->view();

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
    currentUrlBar()->setFocus();
}


void MainView::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i)
    {
        webTab(i)->view()->reload();
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
            m_parentWindow->close();
        }
        else
        {
            closeTab(index);
        }
        return;
    }
    kDebug() << "Invalid tab index" << "line:" << __LINE__;
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

    updateTabBar();
}


void MainView::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    KUrl url = webTab(index)->url();

    Application::instance()->loadUrl(url, Rekonq::NewTab);

    updateTabBar();
}


// When index is -1 index chooses the current tab
void MainView::closeTab(int index, bool del)
{
    // open default homePage if just one tab is opened
    if (count() == 1)
    {
        WebView *w = currentWebTab()->view();

        if( currentWebTab()->url().protocol() == QL1S("about") )
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

    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

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
        QString title = tabToClose->view()->title();
        QString url = tabToClose->url().prettyUrl();
        HistoryItem item(url, QDateTime(), title);
        m_recentlyClosedTabs.removeAll(item);
        m_recentlyClosedTabs.prepend(item);
    }

    removeTab(index);
    updateTabBar();        // UI operation: do it ASAP!!

    m_widgetBar->removeWidget( tabToClose->urlBar() );
    m_widgetBar->setCurrentIndex(m_currentTabIndex);

    if (del)
    {
        tabToClose->deleteLater();
    }

    emit tabsChanged();
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
        kDebug() << "TAB URL: " << tab->url();
        KIcon icon = Application::iconManager()->iconForUrl(tab->url());
        QLabel *label = animatedLoading(index, false);
        QMovie *movie = label->movie();
        delete movie;
        label->setMovie(0);
        label->setPixmap(icon.pixmap(16, 16));
    }
}


void MainView::webViewTitleChanged(const QString &title)
{
    QString viewTitle = title.isEmpty()? i18n("(Untitled)") : title;
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
    Application::historyManager()->updateHistoryEntry(tab->url(), tabTitle);
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
    emit tabsChanged();
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

    const HistoryItem item = m_recentlyClosedTabs.takeFirst();
    Application::instance()->loadUrl(KUrl(item.url), Rekonq::NewTab);
}


void MainView::openClosedTab()
{
    KAction *action = qobject_cast<KAction *>(sender());
    if (action)
    {
        Application::instance()->loadUrl(action->data().toUrl(), Rekonq::NewTab);

        QString title = action->text();
        title = title.remove('&');
        HistoryItem item(action->data().toString(), QDateTime(), title );
        m_recentlyClosedTabs.removeAll(item);
    }
}


void MainView::switchToTab()
{
    // uses the sender to determine the tab index
    QAction *sender = static_cast<QAction*>(QObject::sender());
    int index = sender->data().toInt();
    index -= 1; // to compensate for off by 1 presented to the user
    if( index < 0 || index >= count() )
        return;
    setCurrentIndex( index );
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
    kDebug() << "detaching tab with url: " << u;
    if (u.scheme() == QL1S("about"))
    {
        closeTab(index);
        Application::instance()->loadUrl(u, Rekonq::NewWindow);
    }
    else
    {
        QString label = tab->view()->title();
        UrlBar *bar = tab->urlBar();
        closeTab(index, false);

        MainWindow *w;
        if( toWindow == NULL )
            w = Application::instance()->newMainWindow(false);
        else
            w = toWindow;
        w->mainView()->addTab(tab, Application::iconManager()->iconForUrl(u), label);
        w->mainView()->widgetBar()->insertWidget(0, bar);
        w->mainView()->updateTabBar();
    }
}
