/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2007-2008 Trolltech ASA. All rights reserved
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


// Self Includes
#include "mainview.h"
#include "mainview.moc"

// Local Includes
#include "tabbar.h"
#include "browserapplication.h"
#include "mainwindow.h"
#include "history.h"
#include "urlbar.h"
#include "webview.h"

// KDE Includes
#include <KShortcut>
#include <KStandardShortcut>
#include <KMessageBox>

// Qt Includes
#include <QtGui>
#include <QDebug>




WebActionMapper::WebActionMapper(KAction *root, QWebPage::WebAction webAction, QObject *parent)
    : QObject(parent)
    , m_currentParent(0)
    , m_root(root)
    , m_webAction(webAction)
{
    if ( !m_root )
    {
        return;
    }
    connect(m_root, SIGNAL( triggered() ), this, SLOT( rootTriggered() ) );
    connect(root, SIGNAL( destroyed(QObject *) ), this, SLOT( rootDestroyed() ) );
    
    root->setEnabled(false);
}


void WebActionMapper::rootDestroyed()
{
    m_root = 0;
}


void WebActionMapper::currentDestroyed()
{
    updateCurrent(0);
}


void WebActionMapper::addChild(KAction *action)
{
    if ( !action )
    {
        return;
    }
    connect(action, SIGNAL( changed() ), this, SLOT( childChanged() ) );
}

QWebPage::WebAction WebActionMapper::webAction() const
{
    return m_webAction;
}


void WebActionMapper::rootTriggered()
{
    if (m_currentParent) 
    {
        KAction *gotoAction = new KAction( m_currentParent->action(m_webAction) );
        gotoAction->trigger();
    }
}


void WebActionMapper::childChanged()
{
    if (KAction *source = qobject_cast<KAction*>(sender())) 
    {
        if (m_root
            && m_currentParent
            && source->parent() == m_currentParent) 
        {
            m_root->setChecked(source->isChecked());
            m_root->setEnabled(source->isEnabled());
        }
    }
}


void WebActionMapper::updateCurrent(QWebPage *currentParent)
{
    if (m_currentParent)
        disconnect(m_currentParent, SIGNAL(destroyed(QObject *)),
                   this, SLOT(currentDestroyed()));

    m_currentParent = currentParent;
    if (!m_root)
    {
        return;
    }
    if (!m_currentParent) 
    {
        m_root->setEnabled(false);
        m_root->setChecked(false);
        return;
    }
    KAction *source = new KAction( m_currentParent->action(m_webAction) );
    m_root->setChecked(source->isChecked());
    m_root->setEnabled(source->isEnabled());
    connect(m_currentParent, SIGNAL( destroyed(QObject *) ), this, SLOT( currentDestroyed() ) );
}


// ----------------------------------------------------------------------------------------------------------


MainView::MainView(QWidget *parent)
    : KTabWidget(parent)
    , m_recentlyClosedTabsAction(0)
    , m_newTabAction(0)
    , m_closeTabAction(0)
    , m_nextTabAction(0)
    , m_previousTabAction(0)
    , m_recentlyClosedTabsMenu(0)
    , m_lineEditCompleter(0)
    , m_lineEdits(0)
    , m_tabBar(new TabBar(this))
{
    setElideMode(Qt::ElideRight);

    connect(m_tabBar, SIGNAL(newTab()), this, SLOT(newTab()));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(cloneTab(int)), this, SLOT(cloneTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(reloadAllTabs()));
    connect(m_tabBar, SIGNAL(tabMoveRequested(int, int)), this, SLOT(moveTab(int, int)));
    setTabBar(m_tabBar);

    // Actions
    m_newTabAction = new KAction( KIcon("tab-new"), i18n("New &Tab"), this);
    m_newTabAction->setShortcut( KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N, Qt::CTRL + Qt::Key_T) );
    m_newTabAction->setIconVisibleInMenu(false);
    connect(m_newTabAction, SIGNAL(triggered()), this, SLOT(newTab()));

    m_closeTabAction = new KAction(KIcon("tab-close"), i18n("&Close Tab"), this);
    m_closeTabAction->setShortcut( KShortcut( Qt::CTRL + Qt::Key_W ) );
    m_closeTabAction->setIconVisibleInMenu(false);
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(closeTab()));

    m_nextTabAction = new KAction(i18n("Show Next Tab"), this);
    m_nextTabAction->setShortcuts( QApplication::isRightToLeft() ? KStandardShortcut::tabPrev() : KStandardShortcut::tabNext() );
    connect(m_nextTabAction, SIGNAL(triggered()), this, SLOT(nextTab()));

    m_previousTabAction = new KAction(i18n("Show Previous Tab"), this);
    m_previousTabAction->setShortcuts( QApplication::isRightToLeft() ? KStandardShortcut::tabNext() : KStandardShortcut::tabPrev() );
    connect(m_previousTabAction, SIGNAL(triggered()), this, SLOT(previousTab()));

    m_recentlyClosedTabsMenu = new KMenu(this);
    connect(m_recentlyClosedTabsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowRecentTabsMenu()));
    connect(m_recentlyClosedTabsMenu, SIGNAL(triggered(QAction *)), this, SLOT(aboutToShowRecentTriggeredAction(QAction *)));
    m_recentlyClosedTabsAction = new KAction(i18n("Recently Closed Tabs"), this);
    m_recentlyClosedTabsAction->setMenu(m_recentlyClosedTabsMenu);
    m_recentlyClosedTabsAction->setEnabled(false);

    // corner buttons
    QToolButton *addTabButton = new QToolButton(this);
    addTabButton->setDefaultAction(m_newTabAction);
    addTabButton->setAutoRaise(true);
    addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    setCornerWidget(addTabButton, Qt::TopLeftCorner);

    QToolButton *closeTabButton = new QToolButton(this);
    closeTabButton->setDefaultAction(m_closeTabAction);
    closeTabButton->setAutoRaise(true);
    closeTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    setCornerWidget(closeTabButton, Qt::TopRightCorner);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    m_lineEdits = new QStackedWidget(this);
}


MainView::~MainView()
{
    delete m_lineEditCompleter;
    delete m_recentlyClosedTabsMenu;
} 


// ========================================================================================================
    KAction *MainView::newTabAction() const {return m_newTabAction; }
    KAction *MainView::closeTabAction() const {return m_closeTabAction; }
    KAction *MainView::recentlyClosedTabsAction() const {return m_recentlyClosedTabsAction;}
    KAction *MainView::nextTabAction() const{}
    KAction *MainView::previousTabAction() const{}

    void MainView::slotWebReload()
    {
        WebView *webView = currentWebView();
        QWebPage *currentParent = webView->webPage();
        QAction *action = currentParent->action(QWebPage::Reload);
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

    void MainView::slotWebSelectAll()
    {
        WebView *webView = currentWebView();
        QWebPage *currentParent = webView->webPage();
        // FIXME
    }

// ========================================================================================================


void MainView::clear()
{
    // clear the recently closed tabs
    m_recentlyClosedTabs.clear();
    // clear the line edit history
    for (int i = 0; i < m_lineEdits->count(); ++i) 
    {
        QLineEdit *qLineEdit = lineEdit(i);
        qLineEdit->setText(qLineEdit->text());
    }
}


void MainView::moveTab(int fromIndex, int toIndex)
{
    disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    QWidget *tabWidget = widget(fromIndex);
    QIcon icon = tabIcon(fromIndex);
    QString text = tabText(fromIndex);
    QVariant data = m_tabBar->tabData(fromIndex);
    removeTab(fromIndex);
    insertTab(toIndex, tabWidget, icon, text);
    m_tabBar->setTabData(toIndex, data);
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
    setCurrentIndex(toIndex);
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


void MainView::addWebAction(KAction *action, QWebPage::WebAction webAction)
{
    if (!action)
        return;
    m_webActionList.append(new WebActionMapper(action, webAction, this));
}


void MainView::currentChanged(int index)
{
    WebView *webView = this->webView(index);
    if (!webView)
        return;

    Q_ASSERT( m_lineEdits->count() == count() );

    WebView *oldWebView = this->webView(m_lineEdits->currentIndex());
    if (oldWebView) 
    {
        disconnect(oldWebView, SIGNAL(statusBarMessage(const QString&)), this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)), this, SIGNAL(linkHovered(const QString&)));
        disconnect(oldWebView, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));
    }

    connect(webView, SIGNAL(statusBarMessage(const QString&)), this, SIGNAL(showStatusBarMessage(const QString&)));
    connect(webView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)), this, SIGNAL(linkHovered(const QString&)));
    connect(webView, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));

    for (int i = 0; i < m_webActionList.count(); ++i) 
    {
        WebActionMapper *mapper = m_webActionList[i];
        mapper->updateCurrent(webView->page());
    }
    emit setCurrentTitle(webView->title());
    m_lineEdits->setCurrentIndex(index);
    emit loadProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());

    // set focus to the current webview
    webView->setFocus();
}


QWidget *MainView::lineEditStack() const
{
    return m_lineEdits;
}


QLineEdit *MainView::currentLineEdit() const
{
    return lineEdit(m_lineEdits->currentIndex());
}


WebView *MainView::currentWebView() const
{
    return webView(currentIndex());
}


QLineEdit *MainView::lineEdit(int index) const
{
    UrlBar *urlLineEdit = qobject_cast<UrlBar*>(m_lineEdits->widget(index));
    if (urlLineEdit)
        return urlLineEdit->lineEdit();
    return 0;
}


WebView *MainView::webView(int index) const
{
    QWidget *widget = this->widget(index);
    if (WebView *webView = qobject_cast<WebView*>(widget)) 
    {
        return webView;
    } 
    else 
    {
        // optimization to delay creating the first webview
        if (count() == 1) 
        {
            MainView *that = const_cast<MainView*>(this);
            that->setUpdatesEnabled(false);
            that->newTab();
            that->closeTab(0);
            that->setUpdatesEnabled(true);
            return currentWebView();
        }
    }
    return 0;
}


int MainView::webViewIndex(WebView *webView) const
{
    int index = indexOf(webView);
    return index;
}


WebView *MainView::newTab(bool makeCurrent)
{
    // line edit
    UrlBar *urlLineEdit = new UrlBar;
    QLineEdit *lineEdit = urlLineEdit->lineEdit();
    if (!m_lineEditCompleter && count() > 0) 
    {
        HistoryCompletionModel *completionModel = new HistoryCompletionModel(this);
        completionModel->setSourceModel(BrowserApplication::historyManager()->historyFilterModel());
        m_lineEditCompleter = new QCompleter(completionModel, this);
        // Should this be in Qt by default?
        QAbstractItemView *popup = m_lineEditCompleter->popup();
        QListView *listView = qobject_cast<QListView*>(popup);
        if (listView)
        {
            listView->setUniformItemSizes(true);
        }
    }
    lineEdit->setCompleter(m_lineEditCompleter);
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(lineEditReturnPressed()));
    m_lineEdits->addWidget(urlLineEdit);
    m_lineEdits->setSizePolicy(lineEdit->sizePolicy());

    // optimization to delay creating the more expensive WebView, history, etc
    if (count() == 0) 
    {
        QWidget *emptyWidget = new QWidget;
        QPalette p = emptyWidget->palette();
        p.setColor(QPalette::Window, palette().color(QPalette::Base));
        emptyWidget->setPalette(p);
        emptyWidget->setAutoFillBackground(true);
        disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
        addTab(emptyWidget, i18n("(Untitled)"));
        connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
        return 0;
    }
    
    // webview
    WebView *webView = new WebView;
    urlLineEdit->setWebView(webView);

    connect(webView, SIGNAL(loadStarted()), this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(titleChanged(const QString &)), this, SLOT(webViewTitleChanged(const QString &)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(webView->page(), SIGNAL(windowCloseRequested()), this, SLOT(windowCloseRequested()));
    connect(webView->page(), SIGNAL(geometryChangeRequested(const QRect &)), this, SIGNAL(geometryChangeRequested(const QRect &)));
    connect(webView->page(), SIGNAL(printRequested(QWebFrame *)), this, SIGNAL(printRequested(QWebFrame *)));
    connect(webView->page(), SIGNAL(menuBarVisibilityChangeRequested(bool)), this, SIGNAL(menuBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)), this, SIGNAL(statusBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(toolBarVisibilityChangeRequested(bool)), this, SIGNAL(toolBarVisibilityChangeRequested(bool)));

    connect(webView, SIGNAL( ctrlTabPressed() ), this, SLOT( nextTab() ) );
    connect(webView, SIGNAL( shiftCtrlTabPressed() ), this, SLOT( previousTab() ) );

    addTab(webView, i18n("(Untitled)") );
    if (makeCurrent)
        setCurrentWidget(webView);

    // webview actions
    for (int i = 0; i < m_webActionList.count(); ++i) 
    {
        WebActionMapper *mapper = m_webActionList[i];
        mapper->addChild( new KAction( webView->page()->action( mapper->webAction() ) )  );
    }

    if (count() == 1)
        currentChanged(currentIndex());
    emit tabsChanged();
    return webView;
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


void MainView::lineEditReturnPressed()
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender())) 
    {
        emit loadUrlPage( KUrl( lineEdit->text() ) );
        if (m_lineEdits->currentWidget() == lineEdit)
        {
            currentWebView()->setFocus();
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
            webView->webPage()->mainWindow()->close();
        else
            closeTab(index);
    }
}


void MainView::closeOtherTabs(int index)
{
    if (-1 == index)
        return;
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}


// When index is -1 index chooses the current tab
void MainView::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
    WebView *tab = newTab(false);
    tab->setUrl( webView(index)->url() );
}


// When index is -1 index chooses the current tab
void MainView::closeTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    bool hasFocus = false;
    if (WebView *tab = webView(index)) 
    {
        if (tab->isModified()) 
        {
            int risp = KMessageBox::questionYesNo( this , 
                                                                            i18n("You have modified this page and when closing it you would lose the modification.\n"
                                                                            "Do you really want to close this page?\n"),
                                                                            i18n("Do you really want to close this page?"),
                                                                            KStandardGuiItem::no() );
            if( risp == KMessageBox::No )
                return;
        }
        hasFocus = tab->hasFocus();

        m_recentlyClosedTabsAction->setEnabled(true);
        m_recentlyClosedTabs.prepend(tab->url());
        if (m_recentlyClosedTabs.size() >= MainView::m_recentlyClosedTabsSize)
            m_recentlyClosedTabs.removeLast();
    }
    QWidget *lineEdit = m_lineEdits->widget(index);
    m_lineEdits->removeWidget(lineEdit);
    lineEdit->deleteLater();
    QWidget *webView = widget(index);
    removeTab(index);
    webView->deleteLater();
    emit tabsChanged();
    if (hasFocus && count() > 0)
        currentWebView()->setFocus();
    if (count() == 0)
        emit lastTabClosed();
}


void MainView::webViewLoadStarted()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) 
    {
        setTabIcon(index, KIcon("rekonq") );
    }
}


void MainView::webViewIconChanged()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) 
    {
        QIcon icon = BrowserApplication::instance()->icon(webView->url());
        setTabIcon(index, icon);
    }
}


void MainView::webViewTitleChanged(const QString &title)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        setTabText(index, title);
    }
    if (currentIndex() == index)
        emit setCurrentTitle(title);
    BrowserApplication::historyManager()->updateHistoryItem(webView->url(), title);
}


void MainView::webViewUrlChanged(const QUrl &url)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
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
        QIcon icon = BrowserApplication::instance()->icon(m_recentlyClosedTabs.at(i));
        action->setIcon(icon);
        action->setText( m_recentlyClosedTabs.at(i).prettyUrl() );
        m_recentlyClosedTabsMenu->addAction(action);
    }
}


void MainView::aboutToShowRecentTriggeredAction(QAction *action)
{
    KUrl url = action->data().toUrl();
    loadUrlInCurrentTab(url);
}


void MainView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if ( !childAt(event->pos() )
                // Remove the line below when QTabWidget does not have a one pixel frame
                && event->pos().y() < (tabBar()->y() + tabBar()->height())) 
    {
        newTab();
        return;
    }
    QTabWidget::mouseDoubleClickEvent(event);
}


void MainView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!childAt(event->pos())) {
        m_tabBar->contextMenuRequested(event->pos());
        return;
    }
    QTabWidget::contextMenuEvent(event);
}


void MainView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton && !childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height())) 
    {
        KUrl url( QApplication::clipboard()->text(QClipboard::Selection) );
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) 
        {
            WebView *webView = newTab();
            webView->setUrl(url);
        }
    }
}


void MainView::loadUrlInCurrentTab(const KUrl &url)
{
    WebView *webView = currentWebView();
    if (webView)
    {
        webView->loadUrl(url);
        webView->setFocus();
    }
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


// ----------------------------------------------------------------------------------------------------------------------------
