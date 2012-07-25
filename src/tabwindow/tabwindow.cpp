/***************************************************************************
 *   Copyright (C) 2012 by Andrea Diamantini <adjam7@gmail.com>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#include "tabwindow.h"
#include "tabwindow.moc"

#include "webpage.h"
#include "webwindow.h"
#include "tabbar.h"

#include "tabhistory.h"

#include <KAction>
#include <KDebug>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KUrl>

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


void TabWindow::loadUrlInNewTab(const QUrl &url, TabHistory *history)
{
    WebWindow *tab = prepareNewTab();

    // Now, the dirty jobs...
    addTab(tab, i18n("new tab"));
    tab->load(url);

    setCurrentWidget(tab);

    if (history)
    {
        history->applyHistory(tab->page()->history());
    }

    updateTabBar();
}


void TabWindow::newCleanTab()
{
    QUrl u = QUrl::fromUserInput("/DATI/WEBPAGES/HomePage/index.htm");
    loadUrlInNewTab(u);
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

    setWindowTitle(tab->title() + QLatin1String(" - rekonq"));
    setWindowIcon(tab->icon());
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

    QString tabTitle = title.isEmpty() ? i18n("(Untitled)") : title;
    tabTitle.replace('&', "&&");

    int index = indexOf(tab);
    if (-1 != index)
    {
        setTabText(index, tabTitle);
    }

    if (currentIndex() != index)
    {
        if (tabTitle != i18n("(Untitled)"))
            tabBar()->setTabHighlighted(index, true);
    }
    else
    {
        setWindowTitle(title + QLatin1String(" - rekonq"));
    }

    // TODO: What about window title? Is this enough?
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

    if (currentIndex() == index)
    {
        setWindowIcon(tab->icon());
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

    loadUrlInNewTab(u, &tHistory);
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
            && tabToClose->url().scheme() != QLatin1String("about")
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

    loadUrlInNewTab(u, &history);

    // just to get sure...
    m_recentlyClosedTabs.removeAll(history);
}
