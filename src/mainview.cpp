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
#include "webtab.h"
#include "tabbar.h"
#include "urlbar.h"
#include "sessionmanager.h"

// KDE Includes
#include <KUrl>
#include <KAction>
#include <KShortcut>
#include <KStandardShortcut>
#include <KMessageBox>
#include <KDebug>
#include <KStandardDirs>
#include <KPassivePopup>
#include <KLocalizedString>

// Qt Includes
#include <QTimer>
#include <QString>
#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QMovie>
#include <QWidget>
#include <QVBoxLayout>

// Defines
#define QL1S(x)  QLatin1String(x)


MainView::MainView(MainWindow *parent)
    : KTabWidget(parent)
    , _bars(new QStackedWidget(this))
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
    connect(tabBar, SIGNAL(closeTab(int)),          this,   SLOT(closeTab(int)) );
    connect(tabBar, SIGNAL(mouseMiddleClick(int)),  this,   SLOT(closeTab(int)) );
    connect(tabBar, SIGNAL(newTabRequest()),        this,   SLOT(newTab())      );
    
    connect(tabBar, SIGNAL(cloneTab(int)),          this,   SLOT(cloneTab(int)) );
    connect(tabBar, SIGNAL(closeOtherTabs(int)),    this,   SLOT(closeOtherTabs(int))   );
    connect(tabBar, SIGNAL(reloadTab(int)),         this,   SLOT(reloadTab(int))    );
    connect(tabBar, SIGNAL(reloadAllTabs()),        this,   SLOT(reloadAllTabs())   );
    connect(tabBar, SIGNAL(detachTab(int)),         this,   SLOT(detachTab(int))    );
    
    connect(tabBar, SIGNAL(tabCloseRequested(int)), this,   SLOT(closeTab(int)) );
    connect(tabBar, SIGNAL(tabMoved(int, int)),     this,   SLOT(movedTab(int, int)) );
    
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
    static bool ButtonInCorner = false;

    int tabWidgetWidth = frameSize().width();
    int tabBarWidth = tabBar()->tabSizeHint(0).width() * tabBar()->count();

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
        int tabWidthHint = tabBar()->tabSizeHint(0).width();
        if (tabWidthHint < sizeHint().width()/4)
            newPosX = tabWidgetWidth - m_addTabButton->width();

        m_addTabButton->move(newPosX, 0);
    }
}


QToolButton *MainView::addTabButton() const
{
    return m_addTabButton;
}


TabBar *MainView::tabBar() const
{
    TabBar *tabBar = qobject_cast<TabBar *>( KTabWidget::tabBar() );
    return tabBar; 
}


UrlBar *MainView::urlBar() const
{
    return qobject_cast<UrlBar *>(_bars->widget(m_currentTabIndex));
}


QWidget *MainView::urlBarWidget() const 
{ 
    return _bars; 
}


WebTab *MainView::currentWebTab() const
{
    return webTab(currentIndex());
}


void MainView::updateTabBar()
{
    if( ReKonfig::alwaysShowTabBar() )
    {
        if (!isTabBarHidden())
        {
            if (tabBar()->isHidden())
            {
                tabBar()->show();
                m_addTabButton->show();
            }
            updateTabButtonPosition();
        }
        return;
    }

    if( tabBar()->count() == 1 )
    {
        tabBar()->hide();
        m_addTabButton->hide();
    }
    else if( !isTabBarHidden() )
    {
        if ( tabBar()->isHidden() )
        {
            tabBar()->show();
            m_addTabButton->show();
        }
        updateTabButtonPosition();
    }
}


void MainView::setTabBarHidden(bool hide)
{
    m_addTabButton->setVisible(!hide);
    KTabWidget::setTabBarHidden(hide);
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
    _bars->setCurrentIndex(index);
    
    // clean up "status bar"
    emit showStatusBarMessage( QString() );

    // notify UI to eventually switch stop/reload button
    int progr = tab->progress();
    if(progr == 0)
        emit browserTabLoading(false);
    else
        emit browserTabLoading(true);
    
    // update zoom slider
    if(!Application::instance()->mainWindowList().isEmpty())
        Application::instance()->mainWindow()->setZoomSliderFactor(tab->view()->zoomFactor());

    // set focus to the current webview
    if(tab->url().scheme() == QL1S("about"))
        _bars->currentWidget()->setFocus();
    else
        tab->view()->setFocus();
}


WebTab *MainView::webTab(int index) const
{
    WebTab *tab = qobject_cast<WebTab *>( this->widget(index) );
    if(tab)
    {
        return tab;
    }

    kDebug() << "WebTab with index " << index << "not found. Returning NULL." ;
    return 0;
}


WebTab *MainView::newWebTab(bool focused, bool nearParent)
{
    WebTab* tab = new WebTab(this);
    UrlBar *bar = new UrlBar(tab);
    
    // connecting webview with mainview
    connect(tab->view(), SIGNAL(loadStarted()), this, SLOT(webViewLoadStarted()));
    connect(tab->view(), SIGNAL(loadFinished(bool)), this, SLOT(webViewLoadFinished(bool)));
    connect(tab->view(), SIGNAL(iconChanged()), this, SLOT(webViewIconChanged()));
    connect(tab->view(), SIGNAL(titleChanged(const QString &)), this, SLOT(webViewTitleChanged(const QString &)));
    connect(tab->view(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(webViewUrlChanged(const QUrl &)));

    // connecting webPage signals with mainview
    connect(tab->view()->page(), SIGNAL(windowCloseRequested()), this, SLOT(windowCloseRequested()));
    connect(tab->view()->page(), SIGNAL(printRequested(QWebFrame *)), this, SIGNAL(printRequested(QWebFrame *)));
    
    if (nearParent)
    {
        insertTab(currentIndex() + 1, tab, i18n("(Untitled)"));
        _bars->insertWidget(currentIndex() + 1, bar);
    }
    else
    {
        addTab(tab, i18n("(Untitled)"));
        _bars->addWidget(bar);
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

    switch(ReKonfig::newTabsBehaviour())
    {
    case 0: // new tab page
        w->load( KUrl("about:home") );
        break;
    case 1: // blank page
        urlBar()->clear();
        break;
    case 2: // homepage
        w->load( KUrl(ReKonfig::homePage()) );
        break;
    default:
        break;
    }
    _bars->currentWidget()->setFocus();
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
    WebPage *page = qobject_cast<WebPage *>( sender() );
    WebView *view = qobject_cast<WebView *>( page->view() );
    int index = indexOf( view->parentWidget() );

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
    
    WebTab *tab = newWebTab();
    KUrl url = webTab(index)->url();
    
    // workaround against bug in webkit:
    // only set url if it is not empty
    // otherwise the current working directory will be used
    if (!url.isEmpty())
        tab->view()->setUrl(url);

    updateTabBar();
}


// When index is -1 index chooses the current tab
void MainView::closeTab(int index, bool del)
{
    // open default homePage if just one tab is opened
    if (count() == 1)
    {
        WebView *w = currentWebTab()->view();
        switch(ReKonfig::newTabsBehaviour())
        {
        case 0: // new tab page
        case 1: // blank page
            w->load( KUrl("about:home") );
            urlBar()->setFocus();
            break;
        case 2: // homepage
            w->load( KUrl(ReKonfig::homePage()) );
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

    WebTab *tab = webTab(index);
    if (!tab)
        return;

    if (tab->view()->isModified())
    {
        int risp = KMessageBox::warningContinueCancel(this,
                    i18n("This tab contains changes that have not been submitted.\n"
                            "Closing the tab will discard these changes.\n"
                            "Do you really want to close this tab?\n"),
                    i18n("Closing Modified Tab"), KGuiItem(i18n("Close &Tab"),"tab-close"), KStandardGuiItem::cancel());
        if (risp != KMessageBox::Continue)
            return;
    }

    // store close tab except homepage
    if (!tab->url().prettyUrl().startsWith( QLatin1String("about:") ) && !tab->url().isEmpty())
    {
        QString title = tab->view()->title();
        QString url = tab->url().prettyUrl();
        HistoryItem item(url, QDateTime::currentDateTime(), title);
        m_recentlyClosedTabs.removeAll(item);
        m_recentlyClosedTabs.prepend(item);
    }

    removeTab(index);
    updateTabBar();        // UI operation: do it ASAP!!

    QWidget *urlbar = _bars->widget(index);
    _bars->removeWidget(urlbar);
    
    if(del)
    {
        tab->deleteLater();    // tab is scheduled for deletion.
        urlbar->deleteLater();
    }
    
    emit tabsChanged();
}


void MainView::webViewLoadStarted()
{
    WebView *view = qobject_cast<WebView *>( sender() );
    int index = indexOf( view->parentWidget() );
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
    emit showStatusBarMessage(i18n("Loading..."));
}


void MainView::webViewLoadFinished(bool ok)
{
    WebView *view = qobject_cast<WebView *>( sender() );
    int index = -1;
    if(view)
        index = indexOf( view->parentWidget() );

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
    WebView *view = qobject_cast<WebView *>( sender() );
    int index = indexOf( view->parentWidget() );
    if (-1 != index)
    {
        KIcon icon = Application::icon(view->url());
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
    if (title.isEmpty())
    {
        tabTitle = i18n("(Untitled)");
    }
    WebView *view = qobject_cast<WebView *>( sender() );
    int index = indexOf( view->parentWidget() );
    if (-1 != index)
    {
        setTabText(index, tabTitle);
    }
    if (currentIndex() == index)
    {
        emit currentTitle(tabTitle);
    }
    Application::historyManager()->updateHistoryEntry(view->url(), tabTitle);
}


void MainView::webViewUrlChanged(const QUrl &url)
{
    WebView *view = qobject_cast<WebView *>( sender() );
    int index = indexOf( view->parentWidget() );
    if (-1 != index)
    {
        tabBar()->setTabData(index, url);
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


QList<HistoryItem> MainView::recentlyClosedTabs()
{
    return m_recentlyClosedTabs;
}


void MainView::resizeEvent(QResizeEvent *event)
{
    updateTabBar();
    KTabWidget::resizeEvent(event);
}


void MainView::detachTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    WebTab *tab = webTab(index);
    KUrl u = tab->url();
    kDebug() << u;
    if( u.scheme() == QL1S("about") )
    {
        closeTab(index);
        Application::instance()->loadUrl(u, Rekonq::NewWindow);
    }
    else
    {
        QString label = tab->view()->title();
        QWidget *bar = _bars->widget(index);    
        closeTab(index, false);
        
        MainWindow *w = Application::instance()->newMainWindow(false);
        w->mainView()->addTab(tab, Application::icon( u ), label);
        QStackedWidget *stack = qobject_cast<QStackedWidget *>(w->mainView()->urlBarWidget());
        stack->insertWidget(0, bar);
        w->mainView()->updateTabBar();
    }
}


void MainView::movedTab(int from,int to)
{
    QWidget *bar = _bars->widget(from);
    _bars->removeWidget(bar);
    _bars->insertWidget(to, bar);
}
