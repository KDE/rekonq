/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
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


//Self Includes
#include "tabbar.h"
#include "tabbar.moc"

// Local Includes
#include "rekonq.h"

#include "application.h"
#include "iconmanager.h"
#include "mainview.h"
#include "mainwindow.h"
#include "webpage.h"
#include "webtab.h"
#include "websnap.h"

// KDE Includes
#include <KActionMenu>
#include <KMenu>
#include <KPassivePopup>
#include <KToolBar>

// Qt Includes
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolButton>

#define BASE_WIDTH_DIVISOR    4
#define MIN_WIDTH_DIVISOR     8


TabBar::TabBar(QWidget *parent)
        : KTabBar(parent)
        , m_actualIndex(-1)
        , m_currentTabPreviewIndex(-1)
{
    setElideMode(Qt::ElideRight);

    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(contextMenu(int, const QPoint &)), this, SLOT(contextMenu(int, const QPoint &)));
    connect(this, SIGNAL(emptyAreaContextMenu(const QPoint &)), this, SLOT(emptyAreaContextMenu(const QPoint &)));
}


QSize TabBar::tabSizeHint(int index) const
{
    MainView *view = qobject_cast<MainView *>(parent());

    int buttonSize = view->addTabButton()->size().width();
    int tabBarWidth = view->size().width() - buttonSize;
    int baseWidth =  view->sizeHint().width() / BASE_WIDTH_DIVISOR;
    int minWidth =  view->sizeHint().width() / MIN_WIDTH_DIVISOR;

    int w;
    if (baseWidth*count() < tabBarWidth)
    {
        w = baseWidth;
    }
    else
    {
        if (count() > 0 && tabBarWidth / count() > minWidth)
        {
            w = tabBarWidth / count();
        }
        else
        {
            w = minWidth;
        }
    }

    int h = KTabBar::tabSizeHint(index).height();

    QSize ts = QSize(w, h);
    return ts;
}


void TabBar::cloneTab()
{
    emit cloneTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::closeTab()
{
    emit closeTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::closeOtherTabs()
{
    emit closeOtherTabs(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::reloadTab()
{
    emit reloadTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::detachTab()
{
    emit detachTab(m_actualIndex);
    m_actualIndex = -1;
}


void TabBar::showTabPreview()
{
    //delete previous tab preview
    delete m_previewPopup.data();
    m_previewPopup.clear();

    MainView *mv = qobject_cast<MainView *>(parent());

    WebTab *indexedTab = mv->webTab(m_currentTabPreviewIndex);
    WebTab *currentTab = mv->webTab(currentIndex());

    // check if view && currentView exist before using them :)
    if (!currentTab || !indexedTab)
        return;

    // no previews during load
    if (indexedTab->isPageLoading())
        return;

    int w = tabSizeHint(m_currentTabPreviewIndex).width();
    int h = w * ((0.0 + currentTab->height()) / currentTab->width());

    m_previewPopup = new KPassivePopup(this);
    m_previewPopup.data()->setFrameShape(QFrame::StyledPanel);
    m_previewPopup.data()->setFrameShadow(QFrame::Plain);
    m_previewPopup.data()->setFixedSize(w, h);

    QLabel *l = new QLabel();
    l->setPixmap( WebSnap::renderTabPreview(*indexedTab->page(), w, h) );

    m_previewPopup.data()->setView(l);
    m_previewPopup.data()->layout()->setAlignment(Qt::AlignTop);
    m_previewPopup.data()->layout()->setMargin(0);

    QPoint pos(tabRect(m_currentTabPreviewIndex).x() , tabRect(m_currentTabPreviewIndex).y() + tabRect(m_currentTabPreviewIndex).height());
    m_previewPopup.data()->show(mapToGlobal(pos));
}


void TabBar::hideEvent(QHideEvent *event)
{
    if (!event->spontaneous())
    {
        qobject_cast<MainView *>(parent())->addTabButton()->hide();
    }
    QTabBar::hideEvent(event);
}


void TabBar::showEvent(QShowEvent *event)
{
    QTabBar::showEvent(event);
    if (!event->spontaneous())
    {
        qobject_cast<MainView *>(parent())->addTabButton()->show();
    }
}


void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (count() == 1)
    {
        return;
    }

    if (event->buttons() & Qt::LeftButton)
    {
        // hide addNewTabButton when moving tabs
        MainView *view = qobject_cast<MainView *>(parent());
        QTimer::singleShot(200, view->addTabButton(), SLOT(hide()));
    }

    KTabBar::mouseMoveEvent(event);

    if (ReKonfig::hoveringTabOption() == 0)
    {
        //Find the tab under the mouse
        const int tabIndex = tabAt(event->pos());

        // if found and not the current tab then show tab preview
        if (tabIndex != -1
                && tabIndex != currentIndex()
                && m_currentTabPreviewIndex != tabIndex
                && event->buttons() == Qt::NoButton
           )
        {
            m_currentTabPreviewIndex = tabIndex;
            QTimer::singleShot(200, this, SLOT( showTabPreview() ) );
        }

        // if current tab or not found then hide previous tab preview
        if (tabIndex == currentIndex() || tabIndex == -1)
        {
            if (!m_previewPopup.isNull())
            {
                m_previewPopup.data()->hide();
            }
            m_currentTabPreviewIndex = -1;
        }
    }
}


void TabBar::leaveEvent(QEvent *event)
{
    if (ReKonfig::hoveringTabOption() == 0)
    {
        //if leave tabwidget then hide previous tab preview
        if (!m_previewPopup.isNull())
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreviewIndex = -1;
    }

    KTabBar::leaveEvent(event);
}


void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (ReKonfig::hoveringTabOption() == 0)
    {
        if (!m_previewPopup.isNull())
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreviewIndex = -1;
    }

    // just close tab on middle mouse click
    if (event->button() == Qt::MidButton)
        return;

    KTabBar::mousePressEvent(event);
}


void TabBar::contextMenu(int tab, const QPoint &pos)
{
    setupHistoryActions();

    m_actualIndex = tab;

    KMenu menu;
    MainWindow *mainWindow = Application::instance()->mainWindow();

    menu.addAction(mainWindow->actionByName( QL1S("new_tab") ));
    menu.addAction(mainWindow->actionByName( QL1S("clone_tab") ));
    if (count() > 1)
        menu.addAction(mainWindow->actionByName( QL1S("detach_tab") ));
    menu.addAction(mainWindow->actionByName( QL1S("open_last_closed_tab") ));
    menu.addAction(mainWindow->actionByName( QL1S("closed_tab_menu") ));
    menu.addSeparator();
    menu.addAction(mainWindow->actionByName( QL1S("close_tab") ));
    menu.addAction(mainWindow->actionByName( QL1S("close_other_tabs") ));
    menu.addSeparator();
    menu.addAction(mainWindow->actionByName( QL1S("reload_tab") ));
    menu.addAction(mainWindow->actionByName( QL1S("reload_all_tabs") ));

    menu.exec(pos);
}


void TabBar::emptyAreaContextMenu(const QPoint &pos)
{
    setupHistoryActions();

    KMenu menu;
    MainWindow *mainWindow = Application::instance()->mainWindow();

    menu.addAction(mainWindow->actionByName( QL1S("new_tab") ));
    menu.addAction(mainWindow->actionByName( QL1S("open_last_closed_tab") ));
    menu.addAction(mainWindow->actionByName( QL1S("closed_tab_menu") ));
    menu.addSeparator();
    menu.addAction(mainWindow->actionByName( QL1S("reload_all_tabs") ));

    KToolBar *mainBar = mainWindow->toolBar("mainToolBar");
    if(!mainBar->isVisible())
    {
        menu.addAction( mainBar->toggleViewAction() );
    }

    menu.exec(pos);
}


void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    MainView *mv = qobject_cast<MainView *>(parent());
    QTimer::singleShot(200, mv->addTabButton(), SLOT(show()));

    KTabBar::mouseReleaseEvent(event);
}


void TabBar::tabRemoved(int /*index*/)
{
    if (ReKonfig::hoveringTabOption() == 0)
    {
        if (!m_previewPopup.isNull())
        {
            m_previewPopup.data()->hide();
        }
        m_currentTabPreviewIndex = -1;
    }
}


void TabBar::setupHistoryActions()
{
    MainWindow *w = Application::instance()->mainWindow();
    MainView *mv = qobject_cast<MainView *>(parent());

    QAction *openLastClosedTabAction = w->actionByName( QL1S("open_last_closed_tab") );
    openLastClosedTabAction->setEnabled( mv->recentlyClosedTabs().size() > 0 );

    // update closed tabs menu
    KActionMenu *am = qobject_cast<KActionMenu *>( w->actionByName( QL1S("closed_tab_menu") ));
    if (!am)
        return;

    bool isEnabled = ( mv->recentlyClosedTabs().size() > 0 );
    am->setEnabled(isEnabled);

    if (am->menu())
        am->menu()->clear();

    if(!isEnabled)
        return;

    Q_FOREACH(const HistoryItem &item, mv->recentlyClosedTabs())
    {
        KAction *a = new KAction(Application::iconManager()->iconForUrl(item.url), item.title, this);
        a->setData(item.url);
        connect(a, SIGNAL(triggered()), mv, SLOT(openClosedTab()));
        am->addAction(a);
    }
}
