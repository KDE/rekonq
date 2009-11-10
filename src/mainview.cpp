/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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
#include "tabbar.h"
#include "application.h"
#include "history.h"
#include "urlbar.h"
#include "webview.h"
#include "sessionmanager.h"
#include "homepage.h"

// KDE Includes
#include <KUrl>
#include <KAction>
#include <KShortcut>
#include <KStandardShortcut>
#include <KMessageBox>
#include <KDebug>
#include <KStandardDirs>
#include <KPassivePopup>

// Qt Includes
#include <QtCore/QTimer>
#include <QtCore/QString>

#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QMovie>
#include <QtGui/QWidget>


MainView::MainView(MainWindow *parent)
        : KTabWidget(parent)
        , m_urlBar(new UrlBar(this))
        , m_tabBar(new TabBar(this))
        , m_addTabButton(new QToolButton(this))
        , m_currentTabIndex(0)
        , m_parentWindow(parent)
{
    // setting tabbar
    setTabBar(m_tabBar);

    // set mouse tracking for tab previews
    setMouseTracking(true);
     
    // loading pixmap path
    m_loadingGitPath = KStandardDirs::locate("appdata" , "pics/loading.mng");

    // connecting tabbar signals
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(mouseMiddleClick(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(newTabRequest()), this, SLOT(newTab()));
    
    connect(m_tabBar, SIGNAL(cloneTab(int)), this, SLOT(cloneTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(reloadAllTabs()));

    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    
    // current page index changing
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
    
    QTimer::singleShot(0, this, SLOT(postLaunch()));
}


MainView::~MainView()
{
}


void MainView::postLaunch()
{
    // Session Manager
    connect (this, SIGNAL(tabsChanged()), Application::sessionManager(), SLOT(saveSession()));
    
    m_addTabButton->setDefaultAction(m_parentWindow->actionByName("new_tab"));

    m_addTabButton->setAutoRaise(true);
    m_addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
}


void MainView::updateTabButtonPosition()
{
    kDebug() << "updating new tab button position..";
    
    static bool ButtonInCorner = false;

    int tabWidgetWidth = frameSize().width();
    int tabBarWidth = m_tabBar->tabSizeHint(0).width()*m_tabBar->count();

    if (tabBarWidth + m_addTabButton->width() > tabWidgetWidth)
    {
        if(ButtonInCorner)
            return;
        setCornerWidget(m_addTabButton);
        ButtonInCorner = true;
    }
    else
    {
        if(ButtonInCorner)
        {
            setCornerWidget(0);
            m_addTabButton->show();
            ButtonInCorner = false;
        }

        // detecting X position
        int newPosX = tabBarWidth;      
        int tabWidthHint = m_tabBar->tabSizeHint(0).width();
        if (tabWidthHint < sizeHint().width()/4)
            newPosX = tabWidgetWidth - m_addTabButton->width();

        // detecting Y position
        int newPosY = m_tabBar->height() - m_addTabButton->height();
        if(newPosY < 0)
            newPosY = 5;    // this hardcoded value is used in just ONE situation:
                            // the first time an user changes the "Always Show Tab Bar" settings
                            // try some better fixes, if you can :D

        m_addTabButton->move(newPosX, newPosY);
    }
}


QToolButton *MainView::addTabButton() const
{
    return m_addTabButton;
}


TabBar *MainView::tabBar() const
{
    return m_tabBar; 
}


UrlBar *MainView::urlBar() const 
{ 
    return m_urlBar; 
}


WebView *MainView::currentWebView() const 
{ 
    return webView(currentIndex()); 
}


int MainView::webViewIndex(WebView *webView) const 
{ 
    return indexOf(webView); 
}


void MainView::updateTabBar()
{
    if (ReKonfig::alwaysShowTabBar())
    {
        if (m_tabBar->isHidden())
        {
            m_tabBar->show();
            m_addTabButton->show();
        }
        updateTabButtonPosition();
        return;
    }

    if (m_tabBar->count() == 1)
    {
        m_tabBar->hide();
        m_addTabButton->hide();
    }
    else
    {
        if (m_tabBar->isHidden())
        {
            m_tabBar->show();
            m_addTabButton->show();
        }
        updateTabButtonPosition();
    }
}


void MainView::webReload()
{
    WebView *webView = currentWebView();
    QAction *action = webView->page()->action(QWebPage::Reload);
    action->trigger();
}


void MainView::webStop()
{
    WebView *webView = currentWebView();
    QAction *action = webView->page()->action(QWebPage::Stop);
    action->trigger();
}


void MainView::clear()
{
    // FIXME (the programmer, not the code)
    // What exactly do we need to clear here?
    m_urlBar->clearHistory();
    m_urlBar->clear();

    m_recentlyClosedTabs.clear();
}


// When index is -1 index chooses the current tab
void MainView::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    QWidget *widget = this->widget(index);
    if (WebView *tab = qobject_cast<WebView*>(widget))
        tab->reload();
}


// this slot is called on tab switching
void MainView::currentChanged(int index)
{
    // retrieve the webview related to the index
    WebView *webView = this->webView(index);
    if (!webView)
        return;

    // retrieve the old webview (that where we move from)
    WebView *oldWebView = this->webView(m_currentTabIndex);
    
    // set current index
    m_currentTabIndex = index;

    if (oldWebView)
    {        
        // disconnecting webview from urlbar
        disconnect(oldWebView, SIGNAL(loadProgress(int)), urlBar(), SLOT(updateProgress(int)));
        disconnect(oldWebView, SIGNAL(loadFinished(bool)), urlBar(), SLOT(loadFinished(bool)));
        disconnect(oldWebView, SIGNAL(urlChanged(const QUrl &)), urlBar(), SLOT(setUrl(const QUrl &)));
    
        // disconnecting webpage from mainview
        disconnect(oldWebView->page(), SIGNAL(statusBarMessage(const QString&)),
                   this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
                   this, SIGNAL(linkHovered(const QString&)));
    }

    // connecting webview with urlbar
    connect(webView, SIGNAL(loadProgress(int)), urlBar(), SLOT(updateProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)), urlBar(), SLOT(loadFinished(bool)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)), urlBar(), SLOT(setUrl(const QUrl &)));
    
    connect(webView->page(), SIGNAL(statusBarMessage(const QString&)), 
            this, SIGNAL(showStatusBarMessage(const QString&)));
    connect(webView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)), 
            this, SIGNAL(linkHovered(const QString&)));

    emit setCurrentTitle(webView->title());
    urlBar()->setUrl(webView->url());
    urlBar()->setProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());

    // notify UI to eventually switch stop/reload button
    if(urlBar()->isLoading())
        emit browserTabLoading(true);
    else
        emit browserTabLoading(false);

    // set focus to the current webview
    webView->setFocus();
}


WebView *MainView::webView(int index) const
{
    QWidget *widget = this->widget(index);
    if (WebView *webView = qobject_cast<WebView*>(widget))
    {
        return webView;
    }

    kDebug() << "WebView with index " << index << "not found. Returning NULL." ;
    return 0;
}


WebView *MainView::newWebView(bool focused, bool nearParent)
{
    WebView *webView = new WebView;  // should be deleted on tab close?

    // connecting webview with mainview
    connect(webView, SIGNAL(loadStarted()), this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(webViewLoadFinished(bool)));
    connect(webView, SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(titleChanged(const QString &)), this, SLOT(webViewTitleChanged(const QString &)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));

    // connecting webPage signals with mainview
    connect(webView->page(), SIGNAL(windowCloseRequested()), this, SLOT(windowCloseRequested()));
    connect(webView->page(), SIGNAL(printRequested(QWebFrame *)), this, SIGNAL(printRequested(QWebFrame *)));

    if (nearParent)
        insertTab(currentIndex() + 1, webView, i18n("(Untitled)"));
    else
        addTab(webView, i18n("(Untitled)"));

    kDebug() << "newWebView";
    updateTabBar();
    
    if (focused)
    {
        setCurrentWidget(webView);
    }

    emit tabsChanged();
    
    return webView;
}


void MainView::newTab()
{
    WebView *w = newWebView();

    switch(ReKonfig::newTabsBehaviour())
    {
    case 0: // new tab page
        kDebug() << "newTab";
        if(m_parentWindow->newTabPage())
            break;
    case 1: // blank page
        urlBar()->setUrl(KUrl(""));
        break;
    case 2: // homepage
        w->load( QUrl(ReKonfig::homePage()) );
        break;
    default:
        break;
    }
    urlBar()->setFocus();
}


void MainView::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i)
    {
        QWidget *tabWidget = widget(i);
        if (WebView *tab = qobject_cast<WebView*>(tabWidget))
        {
            tab->reload();
        }
    }
}


void MainView::windowCloseRequested()
{
    WebPage *webPage = qobject_cast<WebPage*>(sender());
    WebView *webView = qobject_cast<WebView*>(webPage->view());
    int index = webViewIndex(webView);

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
    if (-1 == index)
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


// When index is -1 index chooses the current tab
void MainView::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
    
    WebView *tab = newWebView();    
    KUrl url = webView(index)->url();
    
    // workaround against bug in webkit:
    // only set url if it is not empty
    // otherwise the current working directory will be used
    if (!url.isEmpty())
        tab->setUrl(url);

    updateTabBar();
}


// When index is -1 index chooses the current tab
void MainView::closeTab(int index)
{
    // do nothing if just one tab is opened
    if (count() == 1)
        return;

    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    bool hasFocus = false;
    if (WebView *tab = webView(index))
    {
        if (tab->isModified())
        {
            int risp = KMessageBox::questionYesNo(this,
                        i18n("This tab contains changes that have not been submitted.\n"
                             "Closing the tab will discard these changes.\n"
                             "Do you really want to close this tab?\n"),
                        i18n("Closing Modified Tab"));
            if (risp == KMessageBox::No)
                return;
        }
        hasFocus = tab->hasFocus();

        //store close tab except homepage
        if (!tab->url().prettyUrl().startsWith( QLatin1String("rekonq:") ) && !tab->url().isEmpty())
        {
            QString title = tab->title();
            QString url = tab->url().prettyUrl();
            HistoryItem item(url, QDateTime::currentDateTime(), title);
            m_recentlyClosedTabs.removeAll(item);
            m_recentlyClosedTabs.prepend(item);
        }
    }

    QWidget *webView = widget(index);
    removeTab(index);
    updateTabBar();         // UI operation: do it ASAP!!
    webView->deleteLater();  // webView is scheduled for deletion.

    emit tabsChanged();

    if (hasFocus && count() > 0)
    {
        currentWebView()->setFocus();
    }
}


void MainView::webViewLoadStarted()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        QLabel *label = animatedLoading(index, true);
        if (label->movie())
        {
            label->movie()->start();
        }
    }

    emit browserTabLoading(true);

    if (index != currentIndex())
        return;

    emit showStatusBarMessage(i18n("Loading..."));
}


void MainView::webViewLoadFinished(bool ok)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);

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
        emit showStatusBarMessage(i18n("Done"), Rekonq::Success);
    else
        emit showStatusBarMessage(i18n("Failed to load"), Rekonq::Error);
}


void MainView::webViewIconChanged()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        QIcon icon = Application::icon(webView->url());
        QLabel *label = animatedLoading(index, false);
        QMovie *movie = label->movie();
        delete movie;
        label->setMovie(0);
        label->setPixmap(icon.pixmap(16, 16));

        urlBar()->updateUrl();
    }
}


void MainView::webViewTitleChanged(const QString &title)
{
    QString tabTitle = title;
    if (title.isEmpty())
    {
        tabTitle = i18n("(Untitled)");
    }
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        setTabText(index, tabTitle);
    }
    if (currentIndex() == index)
    {
        emit setCurrentTitle(tabTitle);
    }
    Application::historyManager()->updateHistoryEntry(webView->url(), tabTitle);
}


void MainView::webViewUrlChanged(const QUrl &url)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        m_tabBar->setTabData(index, url);
    }
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


QLabel *MainView::animatedLoading(int index, bool addMovie)
{
    if (index == -1)
        return 0;

    QLabel *label = qobject_cast<QLabel*>(m_tabBar->tabButton(index, QTabBar::LeftSide));
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
    m_tabBar->setTabButton(index, QTabBar::LeftSide, 0);
    m_tabBar->setTabButton(index, QTabBar::LeftSide, label);
    return label;
}


QList<HistoryItem> MainView::recentlyClosedTabs()
{
    return m_recentlyClosedTabs;
}


void MainView::resizeEvent(QResizeEvent *event)
{
    kDebug() << "resizeEvent";
    updateTabBar();
    KTabWidget::resizeEvent(event);
}
