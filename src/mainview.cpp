/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
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
#include "mainview.h"
#include "mainview.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "tabbar.h"
#include "application.h"
#include "mainwindow.h"
#include "history.h"
#include "stackedurlbar.h"
#include "urlbar.h"
#include "webview.h"

// KDE Includes
#include <KUrl>
#include <KMenu>
#include <KAction>
#include <KShortcut>
#include <KStandardShortcut>
#include <KMessageBox>
#include <KActionCollection>
#include <KDebug>
#include <KStandardDirs>

// Qt Includes
#include <QtCore>
#include <QtGui>
#include <QtWebKit>



MainView::MainView(QWidget *parent)
        : KTabWidget(parent)
        , m_recentlyClosedTabsAction(0)
        , m_recentlyClosedTabsMenu(new KMenu(this))
        , m_urlBars(new StackedUrlBar(this))
        , m_tabBar(new TabBar(this))
{
    setTabBar(m_tabBar);

    m_loadingGitPath = KStandardDirs::locate("appdata" , "pics/loading.gif");

    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(slotCloseTab(int)));
    connect(m_tabBar, SIGNAL(cloneTab(int)), this, SLOT(slotCloneTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(slotCloseOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(slotReloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(slotReloadAllTabs()));
    connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(moveTab(int,int)));

    // Recently Closed Tab Action
    connect(m_recentlyClosedTabsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowRecentTabsMenu()));
    connect(m_recentlyClosedTabsMenu, SIGNAL(triggered(QAction *)), this, SLOT(aboutToShowRecentTriggeredAction(QAction *)));
    m_recentlyClosedTabsAction = new KAction(i18n("Recently Closed Tabs"), this);
    m_recentlyClosedTabsAction->setMenu(m_recentlyClosedTabsMenu);
    m_recentlyClosedTabsAction->setEnabled(false);

    // --
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));

    connect(Application::instance(), SIGNAL(openUrl(const KUrl &, Rekonq::OpenType)),
            this, SLOT(openUrl(const KUrl &, Rekonq::OpenType)));
    // bookmarks loading
    connect(Application::bookmarkProvider(), SIGNAL(openUrl(const KUrl &, Rekonq::OpenType)),
            this, SLOT(openUrl(const KUrl &, Rekonq::OpenType)));

    setTabsClosable(true);
    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(slotCloseTab(int)));
}


MainView::~MainView()
{
}


void MainView::showTabBar()
{
    if ( ReKonfig::alwaysShowTabBar() )
    {
        if (m_tabBar->isHidden())
        {
            m_tabBar->show();
        }
        return;
    }

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


void MainView::slotWebReload()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Reload);
    action->trigger();
}


void MainView::slotWebStop()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Stop);
    action->trigger();
}


void MainView::slotWebBack()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Back);
    action->trigger();
}


void MainView::slotWebForward()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Forward);
    action->trigger();
}


void MainView::slotWebUndo()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Undo);
    action->trigger();
}


void MainView::slotWebRedo()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Redo);
    action->trigger();
}


void MainView::slotWebCut()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Cut);
    action->trigger();
}


void MainView::slotWebCopy()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Copy);
    action->trigger();
}


void MainView::slotWebPaste()
{
    WebView *webView = currentWebView();
    QWebPage *currentParent = webView->webPage();
    QAction *action = currentParent->action(QWebPage::Paste);
    action->trigger();
}


void MainView::clear()
{
    // clear the recently closed tabs
    m_recentlyClosedTabs.clear();
    // clear the line edit history
    for (int i = 0; i < m_urlBars->count(); ++i)
    {
        /// TODO What exacly do we need to clear here?
        urlBar(i)->clearHistory();
        urlBar(i)->clear();
    }
}


void MainView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height()))
    {
        newWebView(true);
        return;
    }
    KTabWidget::mouseDoubleClickEvent(event);
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


void MainView::slotCurrentChanged(int index)
{
    WebView *webView = this->webView(index);
    if (!webView)
        return;

    Q_ASSERT(m_urlBars->count() == count());

    WebView *oldWebView = this->webView(m_urlBars->currentIndex());
    if (oldWebView)
    {
        disconnect(oldWebView, SIGNAL(statusBarMessage(const QString&)),
                    this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
                    this, SIGNAL(linkHovered(const QString&)));
        disconnect(oldWebView, SIGNAL(loadProgress(int)),
                    this, SIGNAL(loadProgress(int)));
    }

    connect(webView, SIGNAL(statusBarMessage(const QString &)),
            this, SIGNAL(showStatusBarMessage(const QString &)));
    connect(webView->page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)),
            this, SIGNAL(linkHovered(const QString &)));
    connect(webView, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));

    emit setCurrentTitle(webView->title());
    m_urlBars->setCurrentIndex(index);
    emit loadProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());

    // set focus to the current webview
    webView->setFocus();
}


UrlBar *MainView::urlBar(int index) const
{
    if (index == -1)
    {
        index = m_urlBars->currentIndex();
    }
    UrlBar *urlBar = m_urlBars->urlBar(index);
    if (urlBar)
    {
        return urlBar;
    }
    kWarning() << "URL bar with index" << index << "not found. Returning NULL. (line:" << __LINE__ << ")";
    return NULL;
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


WebView *MainView::newWebView(bool makeCurrent)
{
    // line edit
    UrlBar *urlBar = new UrlBar;  // Ownership of widget is passed on to the QStackedWidget (addWidget method).
    connect(urlBar, SIGNAL(activated(const KUrl &)), this, SLOT(openUrl(const KUrl &)));
    m_urlBars->addUrlBar(urlBar);

    WebView *webView = new WebView;  // should be deleted on tab close

    // connecting webview with urlbar
    connect(webView, SIGNAL(loadProgress(int)), urlBar, SLOT(slotUpdateProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)), urlBar, SLOT(slotLoadFinished(bool)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)), urlBar, SLOT(setUrl(const QUrl &)));
    connect(webView, SIGNAL(iconChanged()), urlBar, SLOT(slotUpdateUrl()));

    // connecting webview with mainview
    connect(webView, SIGNAL(loadStarted()), this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(loadProgress(int)), this, SLOT(webViewLoadProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(webViewLoadFinished(bool)));
    connect(webView, SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(titleChanged(const QString &)), this, SLOT(webViewTitleChanged(const QString &)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));

    connect(webView, SIGNAL(ctrlTabPressed()), this, SLOT(nextTab()));
    connect(webView, SIGNAL(shiftCtrlTabPressed()), this, SLOT(previousTab()));

    // connecting webPage signals with mainview
    connect(webView->page(), SIGNAL(windowCloseRequested()),
            this, SLOT(windowCloseRequested()));
    connect(webView->page(), SIGNAL(geometryChangeRequested(const QRect &)),
            this, SIGNAL(geometryChangeRequested(const QRect &)));
    connect(webView->page(), SIGNAL(printRequested(QWebFrame *)),
            this, SIGNAL(printRequested(QWebFrame *)));
    connect(webView->page(), SIGNAL(menuBarVisibilityChangeRequested(bool)),
            this, SIGNAL(menuBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)),
            this, SIGNAL(statusBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(toolBarVisibilityChangeRequested(bool)),
            this, SIGNAL(toolBarVisibilityChangeRequested(bool)));

    addTab(webView, i18n("(Untitled)"));

    if (makeCurrent)
    {
        setCurrentWidget(webView);  // this method does NOT take owneship of webView
    }

    emit tabsChanged();

    showTabBar();

    return webView;
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
    kWarning() << "Index: " << index;
    // do nothing if just one tab is opened
    if( count() == 1 )
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
            int risp = KMessageBox::questionYesNo(this ,
                       i18n("You have modified this page and when closing it you would lose the modification.\n"
                       "Do you really want to close this page?\n"),
                       i18n("Do you really want to close this page?")
                       );
            if (risp == KMessageBox::No)
                return;
        }
        hasFocus = tab->hasFocus();

        m_recentlyClosedTabsAction->setEnabled(true);
        m_recentlyClosedTabs.prepend(tab->url());

        // don't add empty urls
        if (tab->url().isValid())
        {
            m_recentlyClosedTabs.prepend(tab->url());
        }

        if (m_recentlyClosedTabs.size() >= MainView::m_recentlyClosedTabsSize)
        {
            m_recentlyClosedTabs.removeLast();
        }
    }

    QWidget *urlBar = m_urlBars->urlBar(index);
    m_urlBars->removeWidget(urlBar);
    urlBar->deleteLater();   // urlBar is scheduled for deletion.

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
    kWarning() << "Here.. index = " << index;
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

    emit showStatusBarMessage(i18n("Loading..."));
}


void MainView::webViewLoadProgress(int progress)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (index != currentIndex() || index < 0)
    {
        return;
    }

    double totalBytes = static_cast<double>(webView->webPage()->totalBytes() / 1024);

    QString message = i18n("Loading %1% (%2 %3)...", progress, totalBytes, QLatin1String("kB") );
    emit showStatusBarMessage(message);
}


void MainView::webViewLoadFinished(bool ok)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);

   if (-1 != index)
    {
        QLabel *label = animatedLoading(index, true);
        QMovie *movie = label->movie();
        if(movie)
            movie->stop();
    }

    webViewIconChanged();

    // don't display messages for background tabs
    if (index != currentIndex())
    {
        return;
    }

    if (ok)
        emit showStatusBarMessage(i18n("Done"));
    else
        emit showStatusBarMessage(i18n("Failed to load"));
}


void MainView::webViewIconChanged()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
    {
        QIcon icon = Application::instance()->icon(webView->url());
        QLabel *label = animatedLoading(index, false);
        QMovie *movie = label->movie();
        delete movie;
        label->setMovie(0);
        label->setPixmap(icon.pixmap(16, 16));
    }
}


void MainView::webViewTitleChanged(const QString &title)
{
    QString tabTitle = title;
    if(title.isEmpty())
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


void MainView::aboutToShowRecentTabsMenu()
{
    m_recentlyClosedTabsMenu->clear();
    for (int i = 0; i < m_recentlyClosedTabs.count(); ++i)
    {
        KAction *action = new KAction(m_recentlyClosedTabsMenu);
        action->setData(m_recentlyClosedTabs.at(i));
        QIcon icon = Application::instance()->icon(m_recentlyClosedTabs.at(i));
        action->setIcon(icon);
        action->setText(m_recentlyClosedTabs.at(i).prettyUrl());
        m_recentlyClosedTabsMenu->addAction(action);
    }
}


void MainView::aboutToShowRecentTriggeredAction(QAction *action)
{
    KUrl url = action->data().toUrl();
    openUrl(url, Rekonq::Current);
}


void MainView::openUrl(const KUrl& url, Rekonq::OpenType type)
{
    kDebug() << "sender:" << sender() << "  line:" << __LINE__;

    if (url.isEmpty())
    {
        kWarning() << "Can't load an empty url; sender:" << sender() << "line: " << __LINE__;
        return;
    }

    WebView *webView = NULL;
    if (type == Rekonq::New)
        webView = newWebView(true);
    else if (type == Rekonq::Background)
        webView = newWebView(false);
    else
        webView = currentWebView();

    if (!webView)
    {
        kWarning() << "Can't find the view" << "line:" << __LINE__;
        return;
    }
    webView->loadUrl(url);
    webView->setFocus();

    currentUrlBar()->setUrl(url.prettyUrl());
}


void MainView::loadUrlInCurrentTab(const KUrl &url)
{
    openUrl(url);
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


void MainView::moveTab(int fromIndex, int toIndex)
{
    QWidget *lineEdit = m_urlBars->widget(fromIndex);
    m_urlBars->removeWidget(lineEdit);
    m_urlBars->insertWidget(toIndex, lineEdit);
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

