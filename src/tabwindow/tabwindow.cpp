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
#include "webpage.h"
#include "webwindow.h"
#include "tabbar.h"

#include "tabhistory.h"

// KDE Includes
#include <KAction>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KUrl>

// Qt Includes
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QMovie>
#include <QTabBar>
#include <QToolButton>

#include <QWebHistory>
#include <QWebSettings>


TabWindow::TabWindow(QWidget *parent)
    : KTabWidget(parent)
    , _addTabButton(new QToolButton(this))
    , _openedTabsCounter(0)
{
    // This has to be a window...
    setWindowFlags(Qt::Window);

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

    // new tab button
    KAction* a = new KAction(KIcon("tab-new"), i18n("New &Tab"), this);
    _addTabButton->setDefaultAction(a);
    _addTabButton->setAutoRaise(true);
    _addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(_addTabButton, SIGNAL(triggered(QAction *)), this, SLOT(newCleanTab()));

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    // NOTE: NEVER create a tabwindow without AT LEAST one tab...
    WebWindow *tab = prepareNewTab();
    addTab(tab, i18n("new tab"));
    setCurrentWidget(tab);

    // FIXME: Manage sizes...
    kDebug() << "SIZE: " << size();
    kDebug() << "SIZE HINT: " << sizeHint();

    resize(sizeHint());

}


QSize TabWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.8;
    return size;
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
    WebWindow *tab;
    if (page)
        tab = new WebWindow(page, this);
    else
        tab = new WebWindow(this);

    connect(tab, SIGNAL(titleChanged(QString)), this, SLOT(tabTitleChanged(QString)));

    connect(tab, SIGNAL(loadStarted()), this, SLOT(tabLoadStarted()));
    connect(tab, SIGNAL(loadFinished(bool)), this, SLOT(tabLoadFinished(bool)));

    connect(tab, SIGNAL(pageCreated(WebPage *)), this, SLOT(pageCreated(WebPage *)));

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
        // TODO
//         emit loadUrlInNewWindow(url);
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

    updateTabBar();
}


void TabWindow::newCleanTab()
{
    QUrl u = QUrl::fromUserInput("/DATI/WEBPAGES/HomePage/index.htm");
    loadUrl(u, Rekonq::NewTab);
}


void TabWindow::pageCreated(WebPage *page)
{
    WebWindow *tab = prepareNewTab(page);

    // Now, the dirty jobs...
    _openedTabsCounter++;
    insertTab(currentIndex() + _openedTabsCounter, tab, i18n("new tab"));

    // Finally, update tab bar...
    updateTabBar();
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


void TabWindow::resizeEvent(QResizeEvent *event)
{
    QTabWidget::resizeEvent(event);
    updateTabBar();
}


void TabWindow::updateTabBar()
{
    // update tab button position
    static bool ButtonInCorner = false;

    int tabWidgetWidth = frameSize().width();
    int tabBarWidth = tabBar()->sizeHint().width();

    if (tabBarWidth + _addTabButton->width() > tabWidgetWidth)
    {
        if (ButtonInCorner)
            return;
        setCornerWidget(_addTabButton);
        ButtonInCorner = true;
    }
    else
    {
        if (ButtonInCorner)
        {
            setCornerWidget(0);
            ButtonInCorner = false;
        }

        _addTabButton->move(tabBarWidth, 0);
        _addTabButton->show();
    }
}


void TabWindow::tabTitleChanged(const QString &title)
{
    WebWindow *tab = qobject_cast<WebWindow *>(sender());
    if (!tab)
        return;

    bool emptyTitle = title.isEmpty();
    
    QString tabTitle = emptyTitle ? tab->url().toString() : title;
    tabTitle.replace('&', "&&");

    int index = indexOf(tab);
    if (-1 != index)
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
    }
}


void TabWindow::tabLoadFinished(bool ok)
{
    Q_UNUSED(ok);

    WebWindow *tab = qobject_cast<WebWindow *>(sender());
    if (!tab)
        return;

    int index = indexOf(tab);

    if (-1 != index)
    {
        QLabel *label = qobject_cast<QLabel* >(tabBar()->tabButton(index, QTabBar::LeftSide));

        QMovie *movie = label->movie();
        movie->stop();
        delete movie;

        label->setMovie(0);
        label->setPixmap(tab->icon().pixmap(16, 16));
    }
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
        history.url = tabToClose->url().toString();

        m_recentlyClosedTabs.removeAll(history);
        if (m_recentlyClosedTabs.count() == recentlyClosedTabsLimit)
            m_recentlyClosedTabs.removeLast();
        m_recentlyClosedTabs.prepend(history);
    }

    removeTab(index);
    updateTabBar();        // UI operation: do it ASAP!!

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
