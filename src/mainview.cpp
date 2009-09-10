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
#include "mainwindow.h"
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

// Qt Includes
#include <QtCore/QTimer>
#include <QtCore/QString>

#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QMovie>
#include <QtGui/QWidget>
#include <QtGui/QMouseEvent>


MainView::MainView(QWidget *parent)
        : KTabWidget(parent)
        , m_urlBar(new UrlBar(this))
        , m_tabBar(new TabBar(this))
        , m_currentTabIndex(0)
{
    // setting tabbar
    setTabBar(m_tabBar);

    // loading pixmap path
    m_loadingGitPath = KStandardDirs::locate("appdata" , "pics/loading.mng");

    // connecting tabbar signals
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(slotCloseTab(int)));
    connect(m_tabBar, SIGNAL(mouseMiddleClick(int)), this, SLOT(slotCloseTab(int)));
    
    connect(m_tabBar, SIGNAL(cloneTab(int)), this, SLOT(slotCloneTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(slotCloseOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(slotReloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(slotReloadAllTabs()));
    connect(m_tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(moveTab(int, int)));

    // connecting urlbar signals
    connect(urlBar(), SIGNAL(activated(const KUrl&)), Application::instance(), SLOT(loadUrl(const KUrl&)));
    
    // current page index changing
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
    
    setTabsClosable(true);
    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(slotCloseTab(int)));

    // Session Manager
    connect (this,SIGNAL(tabsChanged()),Application::sessionManager(),SLOT(saveSession()));
}


MainView::~MainView()
{
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


void MainView::showTabBar()
{
    if (ReKonfig::alwaysShowTabBar())
    {
        if (m_tabBar->isHidden())
        {
            m_tabBar->show();
        }
    }
    else
    {
        if (m_tabBar->count() == 1)
        {
            m_tabBar->hide();
        }
        else
        {
            if (m_tabBar->isHidden())
            {
                m_tabBar->show();
            }
        }
    }
}


void MainView::slotWebReload()
{
    WebView *webView = currentWebView();
    QAction *action = webView->page()->action(QWebPage::Reload);
    action->trigger();
}


void MainView::slotWebStop()
{
    WebView *webView = currentWebView();
    QAction *action = webView->page()->action(QWebPage::Stop);
    action->trigger();
}


void MainView::clear()
{
    /// TODO What exactly do we need to clear here?
    m_urlBar->clearHistory();
    m_urlBar->clear();
}


// When index is -1 index chooses the current tab
void MainView::slotReloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    QWidget *widget = this->widget(index);
    if (WebView *tab = qobject_cast<WebView*>(widget))
        tab->reload();
}


// TODO need some extra comments to better understand what happens here..
void MainView::slotCurrentChanged(int index)
{
    WebView *webView = this->webView(index);
    if (!webView)
        return;

    WebView *oldWebView = this->webView(m_currentTabIndex);
    m_currentTabIndex=index;

    if (oldWebView)
    {        
        // disconnecting webview from urlbar
        disconnect(oldWebView, SIGNAL(loadProgress(int)), urlBar(), SLOT(slotUpdateProgress(int)));
        disconnect(oldWebView, SIGNAL(loadFinished(bool)), urlBar(), SLOT(slotLoadFinished(bool)));
        disconnect(oldWebView, SIGNAL(urlChanged(const QUrl &)), urlBar(), SLOT(setUrl(const QUrl &)));
    
        // disconnecting webpage from mainview
        disconnect(oldWebView->page(), SIGNAL(statusBarMessage(const QString&)),
                   this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
                   this, SIGNAL(linkHovered(const QString&)));
    }

    // connecting webview with urlbar
    connect(webView, SIGNAL(loadProgress(int)), urlBar(), SLOT(slotUpdateProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)), urlBar(), SLOT(slotLoadFinished(bool)));
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

    kWarning() << "WebView with index " << index << "not found. Returning NULL." ;
    return 0;
}


WebView *MainView::newWebView(bool focused)
{
    WebView *webView = new WebView;  // should be deleted on tab close?

    // connecting webview with mainview
    connect(webView, SIGNAL(loadStarted()), this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(webViewLoadFinished(bool)));
    connect(webView, SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(titleChanged(const QString &)), this, SLOT(webViewTitleChanged(const QString &)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));

    connect(webView, SIGNAL(ctrlTabPressed()), this, SLOT(nextTab()));
    connect(webView, SIGNAL(shiftCtrlTabPressed()), this, SLOT(previousTab()));

    // connecting webPage signals with mainview
    connect(webView->page(), SIGNAL(windowCloseRequested()), this, SLOT(windowCloseRequested()));
    connect(webView->page(), SIGNAL(printRequested(QWebFrame *)), this, SIGNAL(printRequested(QWebFrame *)));

    addTab(webView, i18n("(Untitled)"));
       
    if (focused)
    {
        setCurrentWidget(webView);
    }

    emit tabsChanged();

    showTabBar();
    
    return webView;
}


void MainView::newTab()
{
    WebView *w = newWebView();

    urlBar()->setUrl(KUrl(""));
    urlBar()->setFocus();
    
    slotHome();
}


void MainView::slotReloadAllTabs()
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
            Application::instance()->mainWindow()->close();
        }
        else
        {
            slotCloseTab(index);
        }
    }
    else
    {
        kWarning() << "Invalid tab index" << "line:" << __LINE__;
    }
}


void MainView::slotCloseOtherTabs(int index)
{
    if (-1 == index)
        return;

    for (int i = count() - 1; i > index; --i)
    {
        slotCloseTab(i);
    }

    for (int i = index - 1; i >= 0; --i)
    {
        slotCloseTab(i);
    }

    showTabBar();
}


// When index is -1 index chooses the current tab
void MainView::slotCloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
    WebView *tab = newWebView();
    tab->setUrl(webView(index)->url());

    showTabBar();
}


// When index is -1 index chooses the current tab
void MainView::slotCloseTab(int index)
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
                        i18n("You have modified this page and when closing it you would lose the modifications.\n"
                             "Do you really want to close this page?\n"),
                        i18n("Do you really want to close this page?"));
            if (risp == KMessageBox::No)
                return;
        }
        hasFocus = tab->hasFocus();
    }

    QWidget *webView = widget(index);
    removeTab(index);
    webView->deleteLater();  // webView is scheduled for deletion.

    emit tabsChanged();

    if (hasFocus && count() > 0)
    {
        currentWebView()->setFocus();
    }

    showTabBar();
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

        urlBar()->slotUpdateUrl();
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


void MainView::mouseDoubleClickEvent(QMouseEvent *event) //WARNING Need to be fix
{
    if (!childAt(event->pos()))
    {
        newTab();
        return;
    }
    KTabWidget::mouseDoubleClickEvent(event);
}


void MainView::resizeEvent(QResizeEvent *event)
{
    KTabWidget::resizeEvent(event);
}


void MainView::slotHome()
{
    WebView *w = currentWebView();
    HomePage p;
    
    switch(ReKonfig::newTabsBehaviour())
    {
    case 0:
        w->setHtml( p.rekonqHomePage(), QUrl());
        break;
    case 2:
        w->load( QUrl(ReKonfig::homePage()) );
        break;
    default:
        break;
    }
}
